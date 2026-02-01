/**
 * @file memory_monitor.cpp
 * @brief Memory Monitor Implementation - Public Domain - 2025 3D HUD Project
 *
 * @author Yameng.He
 * @date 2025-12-22
 * @version 1.0
 *
 * @section overview Overview
 * This file contains the implementation of the MemoryMonitor class for
 * integrated memory monitoring of MemoryPool operations.
 *
 * @section license License
 * This library is put in the public domain; you can redistribute it and/or
 * modify it without any restrictions.
 */

#include <chrono>
#include <cstring>
#include <sstream>
#include "utils/log/log_manager.h"
#include "utils/memory/memory_monitor.h"

namespace hud_3d
{
    namespace utils
    {
        // =============================================================================
        // MemoryMonitorConfig Implementation
        // =============================================================================

        MemoryMonitorConfig::MemoryMonitorConfig()
            : enable_guard_bytes(true),
              enable_fill_patterns(true),
              track_source_location(true),
              enable_leak_reporting(true),
              guard_bytes_size(16),
              alloc_pattern(0xCD),
              free_pattern(0xDD),
              guard_pattern_front(0xFD),
              guard_pattern_back(0xBD)
        {
        }

        // =============================================================================
        // MemoryAllocationInfo Implementation
        // =============================================================================

        MemoryAllocationInfo::MemoryAllocationInfo()
            : ptr(nullptr),
              size(0),
              actual_size(0),
              file(),
              line(0),
              function(),
              timestamp(0),
              allocation_id(0),
              is_freed(false),
              has_guard_bytes(false)
        {
        }

        MemoryAllocationInfo::MemoryAllocationInfo(
            void *p, uint64_t sz, const std::string &f,
            int32_t ln, const std::string &func, uint64_t alloc_id)
            : ptr(p),
              size(sz),
              actual_size(sz),
              file(f),
              line(ln),
              function(func),
              timestamp(0),
              allocation_id(alloc_id),
              is_freed(false),
              has_guard_bytes(false)
        {
            // Get current timestamp
            auto now = std::chrono::steady_clock::now();
            auto duration = now.time_since_epoch();
            timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        }

        // =============================================================================
        // MemoryMonitorStatistics Implementation
        // =============================================================================

        MemoryMonitorStatistics::MemoryMonitorStatistics()
            : total_allocations(0),
              total_deallocations(0),
              current_allocations(0),
              current_memory_usage(0),
              peak_memory_usage(0),
              total_allocated_bytes(0),
              allocation_failures(0),
              deallocation_failures(0),
              memory_stomps(0),
              memory_leaks(0)
        {
        }

        void MemoryMonitorStatistics::Reset()
        {
            total_allocations = 0;
            total_deallocations = 0;
            current_allocations = 0;
            current_memory_usage = 0;
            peak_memory_usage = 0;
            total_allocated_bytes = 0;
            allocation_failures = 0;
            deallocation_failures = 0;
            memory_stomps = 0;
            memory_leaks = 0;
        }

        // =============================================================================
        // MemoryMonitor Implementation
        // =============================================================================

        MemoryMonitor::MemoryMonitor(bool enabled, const MemoryMonitorConfig &config)
            : enabled_(enabled),
              config_(config),
              mutex_(),
              allocations_(),
              statistics_(),
              allocation_id_counter_(0)
        {
            statistics_.Reset();
        }

        MemoryMonitor::~MemoryMonitor()
        {
            if (enabled_ && config_.enable_leak_reporting)
            {
                ReportLeaks(true);
            }
        }

        void MemoryMonitor::Enable(const MemoryMonitorConfig &config)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!enabled_)
            {
                enabled_ = true;
                config_ = config;
                statistics_.Reset();
                allocations_.clear();
            }
        }

        void MemoryMonitor::Disable()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (enabled_)
            {
                enabled_ = false;
                allocations_.clear();
                statistics_.Reset();
            }
        }

        bool MemoryMonitor::IsEnabled() const
        {
            return enabled_;
        }

        bool MemoryMonitor::TrackAllocation(
            void *allocated_ptr, uint64_t requested_size,
            const std::string &file, int32_t line, const std::string &function)
        {
            if (!enabled_ || !allocated_ptr)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            // Get the user-accessible pointer
            void *user_ptr = GetUserPointer(allocated_ptr, requested_size);

            // Check for duplicate allocation
            if (allocations_.find(user_ptr) != allocations_.end())
            {
                ++statistics_.allocation_failures;
                return false;
            }

            // Create allocation info
            uint64_t alloc_id = GenerateAllocationId();
            MemoryAllocationInfo info(user_ptr, requested_size, file, line, function, alloc_id);
            info.actual_size = CalculateAllocationSize(requested_size);
            info.has_guard_bytes = config_.enable_guard_bytes;

            // Setup guard bytes if enabled
            if (config_.enable_guard_bytes)
            {
                SetupGuardBytes(allocated_ptr, user_ptr, requested_size);
            }

            // Fill user memory with pattern if enabled
            if (config_.enable_fill_patterns)
            {
                FillMemoryWithPattern(user_ptr, requested_size, config_.alloc_pattern);
            }

            // Track the allocation
            allocations_[user_ptr] = info;

            // Update statistics
            ++statistics_.total_allocations;
            ++statistics_.current_allocations;
            statistics_.current_memory_usage += info.actual_size;
            statistics_.total_allocated_bytes += info.actual_size;

            if (statistics_.current_memory_usage > statistics_.peak_memory_usage)
            {
                statistics_.peak_memory_usage = statistics_.current_memory_usage;
            }

            return true;
        }

        bool MemoryMonitor::TrackDeallocation(
            void *user_ptr, const std::string &file, int32_t line, const std::string &function)
        {
            (void)file;     // Unused parameter
            (void)line;     // Unused parameter
            (void)function; // Unused parameter

            if (!enabled_ || !user_ptr)
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            auto it = allocations_.find(user_ptr);
            if (it == allocations_.end())
            {
                ++statistics_.deallocation_failures;
                return false; // Not tracked or already freed
            }

            MemoryAllocationInfo &info = it->second;

            // Check for double-free
            if (info.is_freed)
            {
                ++statistics_.deallocation_failures;
                return false;
            }

            // Validate memory integrity
            if (info.has_guard_bytes && !ValidateGuardBytes(info))
            {
                ++statistics_.memory_stomps;
                ++statistics_.deallocation_failures;
                return false;
            }

            // Fill user memory with free pattern if enabled
            if (config_.enable_fill_patterns)
            {
                FillMemoryWithPattern(user_ptr, info.size, config_.free_pattern);
            }

            // Mark as freed
            info.is_freed = true;

            // Update statistics
            ++statistics_.total_deallocations;
            --statistics_.current_allocations;
            statistics_.current_memory_usage -= info.actual_size;

            // Remove from tracking
            allocations_.erase(it);

            return true;
        }

        bool MemoryMonitor::ValidateMemoryBlock(void *ptr)
        {
            if (!enabled_ || !ptr)
            {
                return true;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            auto it = allocations_.find(ptr);
            if (it == allocations_.end())
            {
                return false;
            }

            const MemoryAllocationInfo &info = it->second;

            // Check guard bytes if present
            if (info.has_guard_bytes && !ValidateGuardBytes(info))
            {
                ++statistics_.memory_stomps;
                return false;
            }

            return true;
        }

        uint64_t MemoryMonitor::ValidateAllMemoryBlocks()
        {
            if (!enabled_)
            {
                return 0;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            uint64_t corrupted_blocks = 0;

            for (const auto &pair : allocations_)
            {
                const MemoryAllocationInfo &info = pair.second;

                if (info.has_guard_bytes && !ValidateGuardBytes(info))
                {
                    ++corrupted_blocks;
                    ++statistics_.memory_stomps;
                }
            }

            return corrupted_blocks;
        }

        void MemoryMonitor::GetStatistics(MemoryMonitorStatistics &stats) const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stats = statistics_;
        }

        void MemoryMonitor::ResetStatistics()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            statistics_.Reset();
        }

        uint64_t MemoryMonitor::ReportLeaks(bool detailed) const
        {
            if (!enabled_)
            {
                return 0;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            uint64_t leak_count = 0;

            for (const auto &pair : allocations_)
            {
                const MemoryAllocationInfo &info = pair.second;

                if (!info.is_freed)
                {
                    ++leak_count;

                    if (detailed)
                    {
                        LOG_3D_HUD_ERROR("Memory leak detected:");
                        LOG_3D_HUD_ERROR("  Address: 0x{:x}", reinterpret_cast<uintptr_t>(info.ptr));
                        LOG_3D_HUD_ERROR("  Size: {} bytes", info.size);
                        LOG_3D_HUD_ERROR("  Allocation ID: {}", info.allocation_id);

                        if (!info.file.empty())
                        {
                            LOG_3D_HUD_ERROR("  Location: {}:{} in {}", info.file, info.line, info.function);
                        }

                        LOG_3D_HUD_ERROR("");
                    }
                }
            }

            if (leak_count > 0)
            {
                LOG_3D_HUD_ERROR("Total memory leaks detected: {}", leak_count);
                LOG_3D_HUD_ERROR("Total leaked memory: {} bytes", statistics_.current_memory_usage);
            }

            return leak_count;
        }

        uint64_t MemoryMonitor::GetCurrentMemoryUsage() const
        {
            return statistics_.current_memory_usage;
        }

        uint64_t MemoryMonitor::GetPeakMemoryUsage() const
        {
            return statistics_.peak_memory_usage;
        }

        uint64_t MemoryMonitor::GetActiveAllocationCount() const
        {
            return statistics_.current_allocations;
        }

        MemoryMonitorConfig MemoryMonitor::GetConfiguration() const
        {
            return config_;
        }

        void MemoryMonitor::SetConfiguration(const MemoryMonitorConfig &config)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            config_ = config;
        }

        // =============================================================================
        // Memory Allocation Coordination Functions
        // =============================================================================

        uint64_t MemoryMonitor::CalculateAllocationSize(uint64_t requested_size) const
        {
            if (!enabled_ || !config_.enable_guard_bytes)
            {
                return requested_size;
            }

            // Add space for front and back guard bytes
            return requested_size + static_cast<uint64_t>(config_.guard_bytes_size) * 2;
        }

        void *MemoryMonitor::GetUserPointer(void *allocated_ptr, uint64_t requested_size) const
        {
            (void)requested_size; // Unused parameter

            if (!enabled_ || !config_.enable_guard_bytes || !allocated_ptr)
            {
                return allocated_ptr;
            }

            // User memory starts after the front guard bytes
            return static_cast<uint8_t *>(allocated_ptr) + config_.guard_bytes_size;
        }

        void *MemoryMonitor::GetAllocatedPointer(void *user_ptr, uint64_t requested_size) const
        {
            (void)requested_size; // Unused parameter

            if (!enabled_ || !config_.enable_guard_bytes || !user_ptr)
            {
                return user_ptr;
            }

            // Original allocated memory is before the user memory
            return static_cast<uint8_t *>(user_ptr) - config_.guard_bytes_size;
        }

        // =============================================================================
        // Private Helper Functions
        // =============================================================================

        void MemoryMonitor::SetupGuardBytes(void *allocated_ptr, void *user_ptr, uint64_t requested_size)
        {
            if (!allocated_ptr || !user_ptr || config_.guard_bytes_size == 0)
            {
                return;
            }

            uint8_t *allocated_bytes = static_cast<uint8_t *>(allocated_ptr);
            uint8_t *user_bytes = static_cast<uint8_t *>(user_ptr);

            // Setup front guard bytes (before user memory)
            FillMemoryWithPattern(allocated_bytes, config_.guard_bytes_size, config_.guard_pattern_front);

            // Setup back guard bytes (after user memory)
            uint8_t *back_guard = user_bytes + requested_size;
            FillMemoryWithPattern(back_guard, config_.guard_bytes_size, config_.guard_pattern_back);
        }

        bool MemoryMonitor::ValidateGuardBytes(const MemoryAllocationInfo &info)
        {
            if (!info.has_guard_bytes || config_.guard_bytes_size == 0)
            {
                return true;
            }

            uint8_t *byte_ptr = static_cast<uint8_t *>(info.ptr);

            // Check front guard bytes
            if (!CheckMemoryPattern(byte_ptr, config_.guard_bytes_size, config_.guard_pattern_front))
            {
                return false;
            }

            // Check back guard bytes
            uint8_t *back_guard = byte_ptr + config_.guard_bytes_size + info.size;
            if (!CheckMemoryPattern(back_guard, config_.guard_bytes_size, config_.guard_pattern_back))
            {
                return false;
            }

            return true;
        }

        void MemoryMonitor::FillMemoryWithPattern(void *ptr, uint64_t size, uint8_t pattern)
        {
            if (!ptr || size == 0)
            {
                return;
            }
            std::memset(ptr, pattern, static_cast<size_t>(size));
        }

        bool MemoryMonitor::CheckMemoryPattern(const void *ptr, uint64_t size, uint8_t pattern)
        {
            if (!ptr || size == 0)
            {
                return true;
            }

            const uint8_t *byte_ptr = static_cast<const uint8_t *>(ptr);

            for (uint64_t i = 0; i < size; ++i)
            {
                if (byte_ptr[i] != pattern)
                {
                    return false;
                }
            }

            return true;
        }

        uint64_t MemoryMonitor::GetCurrentTimestamp() const
        {
            auto now = std::chrono::steady_clock::now();
            auto duration = now.time_since_epoch();
            return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        }

        uint64_t MemoryMonitor::GenerateAllocationId()
        {
            return ++allocation_id_counter_;
        }

    } // namespace utils
} // namespace hud_3d