/**
 * @file memory_profiler.cpp
 * @brief Memory Performance Profiler Implementation for 3D HUD Project
 *
 * This file implements the memory performance profiling functionality for
 * detecting memory leaks, buffer overruns, use-after-free, and other memory issues.
 *
 * @author Yameng.He
 * @version 2.0
 * @date 2025-12-04
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#ifdef __3D_HUD_PERF_ANALYSIS_MEMORY__

#include <cstring>
#include <chrono>
#include <iostream>
#include <sstream>
#include "utils/log/log_manager.h"
#include "tracy/TracyC.h"
#include "utils/perf/memory_profiler.h"

#undef malloc
#undef calloc
#undef realloc
#undef free

namespace hud_3d
{
    namespace utils
    {
        namespace perf
        {
            MemoryProfiler &MemoryProfiler::GetInstance()
            {
                static MemoryProfiler instance;
                return instance;
            }

            MemoryProfiler::MemoryProfiler()
                : current_memory_usage_(0),
                  peak_memory_usage_(0),
                  allocation_count_(0),
                  deallocation_count_(0),
                  guard_bytes_enabled_(true),
                  guard_bytes_size_(16),         // 16 bytes guard by default
                  fill_patterns_enabled_(false), // Disabled by default - less useful than guard bytes
                  alloc_pattern_(0xAA),          // Pattern for allocated memory
                  free_pattern_(0xDD)            // Pattern for freed memory
            {
                // Initialize Tracy memory profiling
                TracyCAllocS(nullptr, 0, 0); // Initialize memory tracking
            }

            MemoryProfiler::~MemoryProfiler()
            {
                ReportLeaks();
            }

            void MemoryProfiler::TrackAllocation(void *ptr, size_t size,
                                                 const std::string_view file,
                                                 const int32_t line,
                                                 const std::string_view function)
            {
                if (ptr == nullptr)
                    return;

                std::lock_guard<std::mutex> lock(mutex_);

                // For tracking-only mode (hooking), just record the allocation
                MemoryBlockInfo info{
                    ptr,
                    size,
                    size, // No protection, actual size equals requested size
                    (!file.empty() ? file : "[unknown]").data(),
                    line,
                    (!function.empty() ? function : "[unknown]").data(),
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count(),
                    false,
                    false};

                allocations_[ptr] = info;
                current_memory_usage_ += size;
                allocation_count_++;

                // Update peak memory usage
                if (current_memory_usage_ > peak_memory_usage_)
                {
                    peak_memory_usage_.store(current_memory_usage_);
                }

                // Report to Tracy
                TracyCAllocS(ptr, size, 0);
            }

            bool MemoryProfiler::TrackDeallocation(void *ptr,
                                                   const std::string_view file,
                                                   const int32_t line,
                                                   const std::string_view function)
            {
                if (ptr == nullptr)
                    return true;

                std::lock_guard<std::mutex> lock(mutex_);

                auto it = allocations_.find(ptr);
                if (it == allocations_.end())
                {
                    // Double-free or invalid pointer
                    LOG_3D_HUD_ERROR("[MEMORY ERROR] Invalid free at {} : {} in {} - pointer: {}",
                                     file, line, function, ptr);
                    return false;
                }

                if (it->second.is_freed)
                {
                    // Double-free detected
                    LOG_3D_HUD_ERROR("[MEMORY ERROR] Double free at {} : {} in {} - pointer: {}",
                                     file, line, function, ptr);
                    return false;
                }

                // For hooking mode, just mark as freed
                it->second.is_freed = true;
                current_memory_usage_ -= it->second.size; // TrackDeallocation always uses requested size (size)
                deallocation_count_++;

                // Remove from allocations map
                allocations_.erase(it);

                // Report to Tracy
                TracyCFreeS(ptr, 0);

                return true;
            }

            void *MemoryProfiler::ProtectedAllocate(size_t size, const std::string_view file,
                                                    const int32_t line, const std::string_view function)
            {
                // Allocate with protection features
                size_t actual_size;

                void *ptr = std::malloc(size);
                if (ptr == nullptr)
                    return nullptr;

                // Apply protection features
                void *protected_ptr = AddGuardBytes(ptr, size, actual_size);

                // Fill with allocation pattern
                FillMemoryWithPattern(protected_ptr, size, alloc_pattern_);

                // Track the allocation with protection info
                std::lock_guard<std::mutex> lock(mutex_);

                MemoryBlockInfo info{
                    protected_ptr,
                    size,
                    actual_size,
                    (!file.empty() ? file : "[unknown]").data(),
                    line,
                    (!function.empty() ? function : "[unknown]").data(),
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count(),
                    false,
                    true};

                allocations_[protected_ptr] = info;
                // BUG FIX: Use requested size (size) for logical memory usage tracking,
                // regardless of actual_size (which includes guard bytes).
                current_memory_usage_ += size;
                allocation_count_++;

                // Update peak memory usage
                if (current_memory_usage_ > peak_memory_usage_)
                {
                    peak_memory_usage_.store(current_memory_usage_);
                }

                // Report to Tracy
                TracyCAllocS(protected_ptr, size, 0);

                return protected_ptr;
            }

            bool MemoryProfiler::ProtectedDeallocate(void *ptr, const std::string_view file,
                                                     const int32_t line, const std::string_view function)
            {
                if (ptr == nullptr)
                    return true;

                std::lock_guard<std::mutex> lock(mutex_);

                auto it = allocations_.find(ptr);
                if (it == allocations_.end())
                {
                    // Double-free or invalid pointer
                    LOG_3D_HUD_ERROR("[MEMORY ERROR] Invalid free at {} : {} in {} - pointer: {}",
                                     file, line, function, ptr);
                    return false;
                }

                if (it->second.is_freed)
                {
                    // Double-free detected
                    LOG_3D_HUD_ERROR("[MEMORY ERROR] Double free at {} : {} in {} - pointer: {}",
                                     file, line, function, ptr);
                    return false;
                }

                // Validate memory integrity before deallocation (only for protected blocks)
                if (it->second.is_protected)
                {
                    if (!ValidateGuardBytes(it->second))
                    {
                        LOG_3D_HUD_ERROR("[MEMORY ERROR] Buffer overrun detected at {} : {} in {} - pointer: {}",
                                         file, line, function, ptr);
                    }

                    // Fill with free pattern
                    FillMemoryWithPattern(ptr, it->second.size, free_pattern_);
                }

                // Update tracking
                it->second.is_freed = true;
                // BUG FIX: Use requested size (size) for logical memory usage tracking,
                // regardless of actual_size (which includes guard bytes).
                current_memory_usage_ -= it->second.size;
                deallocation_count_++;

                // Free the actual memory
                void *actual_ptr = it->second.is_protected ? RemoveGuardBytes(ptr) : ptr;
                std::free(actual_ptr);

                // Remove from allocations map
                allocations_.erase(it);

                // Report to Tracy
                TracyCFreeS(ptr, 0);

                return true;
            }

            bool MemoryProfiler::ValidateMemoryBlock(void *ptr)
            {
                if (ptr == nullptr)
                    return false;

                std::lock_guard<std::mutex> lock(mutex_);

                auto it = allocations_.find(ptr);
                if (it == allocations_.end())
                {
                    return false; // Not tracked
                }

                if (it->second.is_freed)
                {
                    return false; // Already freed
                }

                // Check guard bytes integrity (only for protected blocks)
                if (it->second.is_protected && !ValidateGuardBytes(it->second))
                {
                    return false;
                }

                // Check memory pattern integrity (only for protected blocks)
                if (it->second.is_protected && !CheckMemoryPattern(ptr, it->second.size, alloc_pattern_))
                {
                    return false;
                }

                return true;
            }

            void MemoryProfiler::ReportLeaks()
            {
                std::lock_guard<std::mutex> lock(mutex_);

                size_t leak_count = 0;
                size_t leak_size = 0;

                for (const auto &[ptr, info] : allocations_)
                {
                    if (!info.is_freed)
                    {
                        leak_count++;
                        leak_size += info.size; // info.size is the requested logical size

                        LOG_3D_HUD_ERROR("[MEMORY LEAK] {} bytes at {} : {} in {} - pointer: {} (protected: {}).",
                                         info.size, info.file, info.line, info.function, ptr,
                                         info.is_protected ? "yes" : "no");
                    }
                }

                if (leak_count > 0)
                {
                    LOG_3D_HUD_ERROR("[MEMORY SUMMARY] {} memory leaks detected, total {} bytes leaked.",
                                     leak_count, leak_size);
                }
                else
                {
                    LOG_3D_HUD_INFO("[MEMORY SUMMARY] No memory leaks detected");
                }

                // Report statistics
                LOG_3D_HUD_INFO("[MEMORY STATS] Peak usage: {} bytes, Allocations: {}, Deallocations: {}.",
                                peak_memory_usage_.load(), allocation_count_.load(), deallocation_count_.load());
            }

            size_t MemoryProfiler::GetCurrentMemoryUsage() const
            {
                return current_memory_usage_;
            }

            size_t MemoryProfiler::GetPeakMemoryUsage() const
            {
                return peak_memory_usage_;
            }

            uint64_t MemoryProfiler::GetAllocationCount() const
            {
                return allocation_count_;
            }

            void MemoryProfiler::SetGuardBytesEnabled(bool enable)
            {
                guard_bytes_enabled_ = enable;
            }

            bool MemoryProfiler::IsGuardBytesEnabled() const
            {
                return guard_bytes_enabled_;
            }

            void MemoryProfiler::SetGuardBytesSize(size_t size)
            {
                guard_bytes_size_ = size;
            }

            void MemoryProfiler::SetFillPatternsEnabled(bool enable)
            {
                fill_patterns_enabled_ = enable;
            }

            void *MemoryProfiler::AddGuardBytes(void *ptr, size_t size, size_t &actual_size)
            {
                if (!guard_bytes_enabled_)
                {
                    actual_size = size;
                    return ptr;
                }

                // Allocate extra space for guard bytes
                size_t total_size = size + 2 * guard_bytes_size_;
                void *new_ptr = std::malloc(total_size);

                if (new_ptr == nullptr)
                    return ptr;

                // Set up guard bytes
                std::memset(new_ptr, 0xFE, guard_bytes_size_); // Front guard
                std::memset(static_cast<uint8_t *>(new_ptr) + guard_bytes_size_ + size,
                            0xFD, guard_bytes_size_); // Rear guard

                // Copy original data
                if (ptr != nullptr)
                {
                    std::memcpy(static_cast<uint8_t *>(new_ptr) + guard_bytes_size_, ptr, size);
                    std::free(ptr);
                }

                actual_size = total_size;
                return static_cast<uint8_t *>(new_ptr) + guard_bytes_size_;
            }

            void *MemoryProfiler::RemoveGuardBytes(void *ptr)
            {
                if (!guard_bytes_enabled_)
                    return ptr;

                return static_cast<uint8_t *>(ptr) - guard_bytes_size_;
            }

            bool MemoryProfiler::ValidateGuardBytes(const MemoryBlockInfo &info)
            {
                if (!guard_bytes_enabled_ || !info.is_protected)
                    return true;

                void *actual_ptr = static_cast<uint8_t *>(info.ptr) - guard_bytes_size_;

                // Check front guard bytes
                for (size_t i = 0; i < guard_bytes_size_; ++i)
                {
                    if (static_cast<uint8_t *>(actual_ptr)[i] != 0xFE)
                    {
                        return false; // Front guard corrupted
                    }
                }

                // Check rear guard bytes
                void *rear_guard = static_cast<uint8_t *>(actual_ptr) + guard_bytes_size_ + info.size;
                for (size_t i = 0; i < guard_bytes_size_; ++i)
                {
                    if (static_cast<uint8_t *>(rear_guard)[i] != 0xFD)
                    {
                        return false; // Rear guard corrupted
                    }
                }

                return true;
            }

            void MemoryProfiler::FillMemoryWithPattern(void *ptr, size_t size, uint8_t pattern)
            {
                if (!fill_patterns_enabled_ || size == 0)
                    return;
                // Simple memset - pattern filling is less important than guard bytes
                std::memset(ptr, pattern, size);
            }

            bool MemoryProfiler::CheckMemoryPattern(const void *ptr, size_t size, uint8_t pattern)
            {
                if (!fill_patterns_enabled_ || size == 0)
                    return true;

                // For performance, only check a few key locations
                const uint8_t *byte_ptr = static_cast<const uint8_t *>(ptr);

                // Check first, middle, and last bytes only
                if (byte_ptr[0] != pattern || byte_ptr[size - 1] != pattern)
                {
                    return false;
                }

                // Only check middle if size is large enough
                if (size > 10 && byte_ptr[size / 2] != pattern)
                {
                    return false;
                }

                return true;
            }

            void InitializeMemoryProfiling()
            {
                // Memory profiler initializes automatically on first use
                MemoryProfiler::GetInstance();

                // Configure default settings for protection features
                MemoryProfiler::GetInstance().SetGuardBytesEnabled(true);
                MemoryProfiler::GetInstance().SetGuardBytesSize(16);

                LOG_3D_HUD_INFO("[MEMORY] Memory profiling system initialized.");
            }

            void ShutdownMemoryProfiling()
            {
                // Memory profiler reports leaks automatically on destruction
                LOG_3D_HUD_INFO("[MEMORY] Memory profiling system shutting down.");
            }

            // Helper function to check if we should use protected allocation
            inline bool ShouldUseProtectedAllocation()
            {
                return hud_3d::utils::perf::MemoryProfiler::GetInstance().IsGuardBytesEnabled();
            }

            void EnableAutoProtection()
            {
                hud_3d::utils::perf::MemoryProfiler::GetInstance().SetGuardBytesEnabled(true);
                LOG_3D_HUD_INFO("[MEMORY] Auto-protection mode enabled (hooking + protection).");
            }

            void DisableAutoProtection()
            {
                hud_3d::utils::perf::MemoryProfiler::GetInstance().SetGuardBytesEnabled(false);
                LOG_3D_HUD_INFO("[MEMORY] Auto-protection mode disabled.");
            }

            bool IsAutoProtectionEnabled()
            {
                return hud_3d::utils::perf::MemoryProfiler::GetInstance().IsGuardBytesEnabled();
            }

        } // namespace perf
    } // namespace utils
} // namespace hud_3d

// Override new/delete operators
void *operator new(size_t size, const std::string_view file,
                   const int32_t line, const std::string_view function)
{
    if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
    {
        // Use protected allocation with automatic protection
        return hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(
            size, file, line, function);
    }

    // Use normal tracking only
    void *ptr = std::malloc(size);
    if (ptr != nullptr)
    {
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(
            ptr, size, file, line, function);
    }
    return ptr;
}

void *operator new[](size_t size, const std::string_view file,
                     const int32_t line, const std::string_view function)
{
    if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
    {
        // Use protected allocation with automatic protection
        return hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(
            size, file, line, function);
    }

    // Use normal tracking only
    void *ptr = std::malloc(size);
    if (ptr != nullptr)
    {
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(
            ptr, size, file, line, function);
    }
    return ptr;
}

void *operator new(size_t size)
{
    if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
    {
        // Use protected allocation with automatic protection
        return hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(
            size, "Unknown File", 0, "operator new");
    }

    // Use normal tracking only
    void *ptr = std::malloc(size);
    if (ptr != nullptr)
    {
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(
            ptr, size, "Unknown File", 0, "Unknown Function");
    }
    return ptr;
}

void *operator new[](size_t size)
{
    if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
    {
        // Use protected allocation with automatic protection
        return hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(
            size, "Unknown File", 0, "Unknown Function");
    }

    // Use normal tracking only
    void *ptr = std::malloc(size);
    if (ptr != nullptr)
    {
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(
            ptr, size, "Unknown File", 0, "Unknown Function");
    }
    return ptr;
}

void operator delete(void *ptr) noexcept
{
    if (ptr != nullptr)
    {
        // operator delete cannot easily get caller file/line info
        constexpr const char *caller_info_unavailable = "[caller info unavailable]";
        if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
        {
            // Use protected deallocation with validation
            hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(
                ptr, caller_info_unavailable, 0, "operator delete");
        }
        else
        {
            // Use normal tracking only
            hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(
                ptr, caller_info_unavailable, 0, "operator delete");
            std::free(ptr);
        }
    }
}

void operator delete[](void *ptr, const std::string_view file, const int32_t line,
                       const std::string_view function) noexcept
{
    if (ptr != nullptr)
    {
        // This overload allows capturing caller info
        if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
        {
            // Use protected deallocation with validation
            hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(
                ptr, file, line, function);
        }
        else
        {
            // Use normal tracking only
            hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(
                ptr, file, line, function);
            std::free(ptr);
        }
    }
}

void operator delete(void *ptr, size_t) noexcept
{
    if (ptr != nullptr)
    {
        // operator delete cannot easily get caller file/line info
        constexpr const char *caller_info_unavailable = "[caller info unavailable]";
        if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
        {
            // Use protected deallocation with validation
            hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(
                ptr, caller_info_unavailable, 0, "operator delete sized");
        }
        else
        {
            // Use normal tracking only
            hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(
                ptr, caller_info_unavailable, 0, "operator delete sized");
            std::free(ptr);
        }
    }
}

void operator delete[](void *ptr) noexcept
{
    if (ptr != nullptr)
    {
        // operator delete cannot easily get caller file/line info
        constexpr const char *caller_info_unavailable = "[caller info unavailable]";
        if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
        {
            // Use protected deallocation with validation
            hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(
                ptr, caller_info_unavailable, 0, "operator delete[]");
        }
        else
        {
            // Use normal tracking only
            hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(
                ptr, caller_info_unavailable, 0, "operator delete[]");
            std::free(ptr);
        }
    }
}

void operator delete[](void *ptr, size_t) noexcept
{
    if (ptr != nullptr)
    {
        // operator delete cannot easily get caller file/line info
        constexpr const char *caller_info_unavailable = "[caller info unavailable]";
        if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
        {
            // Use protected deallocation with validation
            hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(
                ptr, caller_info_unavailable, 0, "operator delete[] sized");
        }
        else
        {
            // Use normal tracking only
            hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(
                ptr, caller_info_unavailable, 0, "operator delete[] sized");
            std::free(ptr);
        }
    }
}

// Override malloc/free functions
void *tracked_malloc(std::size_t size, const std::string_view file,
                     const int32_t line, const std::string_view function)
{
    if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
    {
        // Use protected allocation with automatic protection
        return hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(
            size, file, line, function);
    }

    // Use normal tracking only
    void *ptr = std::malloc(size);
    if (ptr != nullptr)
    {
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(
            ptr, size, file, line, function);
    }
    return ptr;
}

void tracked_free(void *ptr, const std::string_view file,
                  const int32_t line, const std::string_view function)
{
    if (ptr != nullptr)
    {
        if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
        {
            // Use protected deallocation with validation
            hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(
                ptr, file, line, function);
        }
        else
        {
            // Use normal tracking only
            hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(
                ptr, file, line, function);
            std::free(ptr);
        }
    }
}

void *tracked_calloc(std::size_t num, std::size_t size,
                     const std::string_view file,
                     const int32_t line, const std::string_view function)
{
    const size_t total_size = num * size;
    if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
    {
        // Use protected allocation with automatic protection
        void *ptr = hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(
            total_size, file, line, function);
        // Zero the memory since calloc should do this
        if (ptr != nullptr)
        {
            std::memset(ptr, 0, total_size);
        }
        return ptr;
    }

    // Use normal tracking only
    void *ptr = std::calloc(num, size);
    if (ptr != nullptr)
    {
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(
            ptr, total_size, file, line, function);
    }
    return ptr;
}

void *tracked_realloc(void *ptr, std::size_t size,
                      const std::string_view file, const int32_t line,
                      const std::string_view function)
{
    if (hud_3d::utils::perf::ShouldUseProtectedAllocation())
    {
        // For protected mode, we need to handle realloc specially.
        // NOTE: This implementation performs free then alloc, and DOES NOT preserve data.
        if (ptr != nullptr)
        {
            // Track deallocation of old pointer
            hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(
                ptr, file, line, function);
        }

        // Allocate new protected memory
        void *new_ptr = hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(
            size, file, line, function);

        // Copy old data if needed (Skipped due to limitation of not knowing old size easily)
        // if (ptr != nullptr && new_ptr != nullptr) {}

        return new_ptr;
    }

    // Track deallocation of old pointer if hooking is enabled
    if (ptr != nullptr)
    {
        // Get old size is difficult here, so we track deallocation with the assumption
        // the original tracking included the necessary size information.
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(
            ptr, file, line, function);
    }

    void *new_ptr = std::realloc(ptr, size);

    // Track allocation of new pointer if hooking is enabled
    if (new_ptr != nullptr)
    {
        hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(
            new_ptr, size, file, line, function);
    }

    return new_ptr;
}

#endif // __3D_HUD_PERF_ANALYSIS_MEMORY__