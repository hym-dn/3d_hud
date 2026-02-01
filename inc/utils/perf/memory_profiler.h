/**
 * @file memory_profiler.h
 * @brief Memory Performance Profiler for 3D HUD Project
 *
 * This header defines the memory performance profiling interface for detecting
 * memory leaks, buffer overruns, use-after-free, and other memory-related issues.
 *
 * @author Yameng.He
 * @version 2.0
 * @date 2025-12-04
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#ifdef __3D_HUD_PERF_ANALYSIS_MEMORY__

#include "utils/utils_define.h"
#include "tracy/Tracy.hpp"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace hud_3d
{
    namespace utils
    {
        namespace perf
        {
            /**
             * @struct MemoryBlockInfo
             * @brief Information about a tracked memory block
             */
            struct MemoryBlockInfo
            {
                void *ptr;            ///< Pointer to allocated memory
                size_t size;          ///< Size of allocated memory
                size_t actual_size;   ///< Actual allocated size (with guard bytes)
                std::string file;     ///< Source file where allocation occurred
                int32_t line;         ///< Source line where allocation occurred
                std::string function; ///< Function where allocation occurred
                int64_t timestamp;    ///< Allocation timestamp
                bool is_freed;        ///< Whether the block has been freed
                bool is_protected;    ///< Whether the block has protection features
            };

            /**
             * @class MemoryProfiler
             * @brief Memory performance profiling class
             *
             * Provides comprehensive memory tracking for detecting leaks,
             * buffer overruns, use-after-free, and other memory issues.
             */
            class MemoryProfiler
            {
            public:
                ~MemoryProfiler();

            public:
                /**
                 * @brief Gets the singleton instance of the memory profiler
                 */
                static MemoryProfiler &GetInstance();

                /**
                 * @brief Tracks a memory allocation (for hooking mode)
                 *
                 * @param ptr Pointer to allocated memory
                 * @param size Requested allocation size
                 * @param file Source file name
                 * @param line Source line number
                 * @param function Function name
                 */
                void TrackAllocation(void *ptr, size_t size, const std::string_view file,
                                     const int32_t line, const std::string_view function);

                /**
                 * @brief Tracks a memory deallocation (for hooking mode)
                 *
                 * @param ptr Pointer to deallocated memory
                 * @param file Source file name
                 * @param line Source line number
                 * @param function Function name
                 * @return true if deallocation was valid, false if double-free or invalid pointer
                 */
                bool TrackDeallocation(void *ptr, const std::string_view file,
                                       const int32_t line, const std::string_view function);

                /**
                 * @brief Allocates memory with protection features
                 *
                 * @param size Requested allocation size
                 * @param file Source file name
                 * @param line Source line number
                 * @param function Function name
                 * @return Pointer to allocated memory with protection
                 */
                void *ProtectedAllocate(size_t size, const std::string_view file,
                                        const int32_t line, const std::string_view function);

                /**
                 * @brief Deallocates protected memory
                 *
                 * @param ptr Pointer to protected memory
                 * @param file Source file name
                 * @param line Source line number
                 * @param function Function name
                 * @return true if deallocation was successful, false otherwise
                 */
                bool ProtectedDeallocate(void *ptr, const std::string_view file,
                                         const int32_t line, const std::string_view function);

                /**
                 * @brief Validates memory block integrity
                 *
                 * @param ptr Pointer to memory block to validate
                 * @return true if block is valid, false if corruption detected
                 */
                bool ValidateMemoryBlock(void *ptr);

                /**
                 * @brief Reports memory leaks at application shutdown
                 */
                void ReportLeaks();

                /**
                 * @brief Gets current memory usage statistics
                 *
                 * @return Current memory usage in bytes
                 */
                size_t GetCurrentMemoryUsage() const;

                /**
                 * @brief Gets peak memory usage
                 *
                 * @return Peak memory usage in bytes
                 */
                size_t GetPeakMemoryUsage() const;

                /**
                 * @brief Gets total allocations count
                 *
                 * @return Total number of allocations
                 */
                uint64_t GetAllocationCount() const;

                /**
                 * @brief Enables/disables guard bytes for buffer overrun detection
                 *
                 * @param enable true to enable guard bytes, false to disable
                 */
                void SetGuardBytesEnabled(bool enable);

                /**
                 * @brief Checks if guard bytes are enabled
                 *
                 * @return true if guard bytes are enabled, false otherwise
                 */
                bool IsGuardBytesEnabled() const;

                /**
                 * @brief Sets the size of guard bytes for buffer overrun detection
                 *
                 * @param size Size of guard bytes in bytes
                 */
                void SetGuardBytesSize(size_t size);

                /**
                 * @brief Enables/disables memory fill patterns for corruption detection
                 *
                 * @param enable true to enable fill patterns, false to disable
                 */
                void SetFillPatternsEnabled(bool enable);

            private:
                MemoryProfiler(MemoryProfiler &&) = delete;
                MemoryProfiler(const MemoryProfiler &) = delete;
                MemoryProfiler &operator=(MemoryProfiler &&) = delete;
                MemoryProfiler &operator=(const MemoryProfiler &) = delete;

            private:
                MemoryProfiler();

                /**
                 * @brief Adds guard bytes around allocated memory
                 */
                void *AddGuardBytes(void *ptr, size_t size, size_t &actual_size);

                /**
                 * @brief Removes guard bytes from allocated memory
                 */
                void *RemoveGuardBytes(void *ptr);

                /**
                 * @brief Validates guard bytes integrity
                 */
                bool ValidateGuardBytes(const MemoryBlockInfo &info);

                /**
                 * @brief Fills memory with patterns for corruption detection
                 */
                void FillMemoryWithPattern(void *ptr, size_t size, uint8_t pattern);

                /**
                 * @brief Checks if memory pattern is intact
                 */
                bool CheckMemoryPattern(const void *ptr, size_t size, uint8_t pattern);

            private:
                std::unordered_map<void *, MemoryBlockInfo> allocations_; ///< Active allocations
                mutable std::mutex mutex_;                                ///< Thread safety mutex
                std::atomic<size_t> current_memory_usage_;                ///< Current memory usage
                std::atomic<size_t> peak_memory_usage_;                   ///< Peak memory usage
                std::atomic<uint64_t> allocation_count_;                  ///< Total allocation count
                std::atomic<uint64_t> deallocation_count_;                ///< Total deallocation count
                bool guard_bytes_enabled_;                                ///< Guard bytes enabled flag
                size_t guard_bytes_size_;                                 ///< Size of guard bytes
                bool fill_patterns_enabled_;                              ///< Fill patterns enabled flag
                uint8_t alloc_pattern_;                                   ///< Pattern for allocated memory
                uint8_t free_pattern_;                                    ///< Pattern for freed memory
            };

            /**
             * @brief Initializes the memory profiling system
             */
            void InitializeMemoryProfiling();

            /**
             * @brief Shuts down the memory profiling system
             */
            void ShutdownMemoryProfiling();

            /**
             * @brief Enable automatic memory protection in hooking mode
             *
             * When enabled, all memory allocations through hooking (new/delete, malloc/free)
             * will automatically include protection features (guard bytes, pattern filling).
             */
            void EnableAutoProtection();

            /**
             * @brief Disable automatic memory protection in hooking mode
             */
            void DisableAutoProtection();

            /**
             * @brief Check if automatic memory protection is enabled
             * @return true if auto-protection is enabled, false otherwise
             */
            bool IsAutoProtectionEnabled();

        } // namespace perf
    } // namespace utils
} // namespace hud_3d

/**
 * @brief Macro for tracking memory allocations with automatic source location
 * @param ptr Pointer to allocated memory
 * @param size Allocation size in bytes
 */
#define HUD_3D_MEMORY_TRACK_ALLOC(ptr, size)                                      \
    hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackAllocation(ptr, size, \
                                                                       __FILE__, __LINE__, __FUNCTION__)

/**
 * @brief Macro for tracking memory deallocations with automatic source location
 * @param ptr Pointer to deallocated memory
 */
#define HUD_3D_MEMORY_TRACK_FREE(ptr)                                         \
    hud_3d::utils::perf::MemoryProfiler::GetInstance().TrackDeallocation(ptr, \
                                                                         __FILE__, __LINE__, __FUNCTION__)

/**
 * @brief Macro for allocating memory with protection features
 * @param size Requested allocation size
 */
#define HUD_3D_MEMORY_PROTECTED_ALLOC(size) \
    hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedAllocate(size, __FILE__, __LINE__, __FUNCTION__)

/**
 * @brief Macro for deallocating protected memory
 * @param ptr Pointer to protected memory
 */
#define HUD_3D_MEMORY_PROTECTED_FREE(ptr) \
    hud_3d::utils::perf::MemoryProfiler::GetInstance().ProtectedDeallocate(ptr, __FILE__, __LINE__, __FUNCTION__)

/**
 * @brief Macro for validating memory block integrity
 * @param ptr Pointer to memory block to validate
 */
#define HUD_3D_MEMORY_VALIDATE(ptr) \
    hud_3d::utils::perf::MemoryProfiler::GetInstance().ValidateMemoryBlock(ptr)

/**
 * @brief Macro for initializing memory profiling system
 */
#define HUD_3D_MEMORY_INITIALIZE() hud_3d::utils::perf::InitializeMemoryProfiling()

/**
 * @brief Macro for shutting down memory profiling system
 */
#define HUD_3D_MEMORY_SHUTDOWN() hud_3d::utils::perf::ShutdownMemoryProfiling()

/**
 * @brief Macro for enabling automatic memory protection in hooking mode
 */
#define HUD_3D_MEMORY_ENABLE_AUTO_PROTECTION() hud_3d::utils::perf::EnableAutoProtection()

/**
 * @brief Macro for disabling automatic memory protection in hooking mode
 */
#define HUD_3D_MEMORY_DISABLE_AUTO_PROTECTION() hud_3d::utils::perf::DisableAutoProtection()

/**
 * @brief Macro for checking if automatic memory protection is enabled
 */
#define HUD_3D_MEMORY_IS_AUTO_PROTECTION_ENABLED() hud_3d::utils::perf::IsAutoProtectionEnabled()

// Override new/delete operators
void *operator new(std::size_t size, const std::string_view file, const int32_t line, const std::string_view function);
void *operator new[](std::size_t size, const std::string_view file, const int32_t line, const std::string_view function);
void *operator new(std::size_t size);
void *operator new[](std::size_t size);
void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;
void operator delete(void *ptr, std::size_t size) noexcept;
void operator delete[](void *ptr, std::size_t size) noexcept;

// Override malloc/free functions
void *tracked_malloc(std::size_t size, const std::string_view file,
                     const int32_t line, const std::string_view function);
void tracked_free(void *ptr, const std::string_view file,
                  const int32_t line, const std::string_view function);
void *tracked_calloc(std::size_t num, std::size_t size,
                     const std::string_view file, const int32_t line,
                     const std::string_view function);
void *tracked_realloc(void *ptr, std::size_t size,
                      const std::string_view file, const int32_t line,
                      const std::string_view function);

#define malloc(size) ::tracked_malloc((size), "Unknown File", 0, "Unknown Function")
#define calloc(num, size) ::tracked_calloc((num), (size), "Unknown File", 0, "Unknown Function")
#define realloc(ptr, size) ::tracked_realloc((ptr), (size), "Unknown File", 0, "Unknown Function")
#define free(ptr) ::tracked_free((ptr), "Unknown File", 0, "Unknown Function")

#define HUD_3D_NEW(type, ...) \
    []() -> type * { \
        type* ptr = static_cast<type *>(::operator new(sizeof(type), __FILE__, __LINE__, __FUNCTION__)); \
        if (ptr) new(ptr) type(__VA_ARGS__); \
        return ptr; }()
#define HUD_3D_NEW_ARRAY(type, count) \
    []() -> type * { \
        type* ptr = static_cast<type *>(::operator new[](sizeof(type) * (count), __FILE__, __LINE__, __FUNCTION__)); \
        if (ptr) for(size_t i = 0; i < count; ++i) new(&ptr[i]) type(); \
        return ptr; }()
#define HUD_3D_DELETE(ptr)                                         \
    do                                                             \
    {                                                              \
        if (ptr)                                                   \
        {                                                          \
            using Type = std::remove_pointer_t<decltype(ptr)>;     \
            if constexpr (!std::is_trivially_destructible_v<Type>) \
            {                                                      \
                ptr->~Type();                                      \
            }                                                      \
            ::operator delete(ptr);                                \
            ptr = nullptr;                                         \
        }                                                          \
    } while (0)
#define HUD_3D_DELETE_ARRAY(ptr, count)                            \
    do                                                             \
    {                                                              \
        if (ptr)                                                   \
        {                                                          \
            using Type = std::remove_pointer_t<decltype(ptr)>;     \
            if constexpr (!std::is_trivially_destructible_v<Type>) \
            {                                                      \
                for (size_t i = 0; i < count; ++i)                 \
                    ptr[i].~Type();                                \
            }                                                      \
            ::operator delete[](ptr);                              \
            ptr = nullptr;                                         \
        }                                                          \
    } while (0)
#define HUD_3D_MALLOC(size) ::tracked_malloc((size), __FILE__, __LINE__, __FUNCTION__)
#define HUD_3D_CALLOC(num, size) ::tracked_calloc((num), (size), __FILE__, __LINE__, __FUNCTION__)
#define HUD_3D_REALLOC(ptr, size) ::tracked_realloc((ptr), (size), __FILE__, __LINE__, __FUNCTION__)
#define HUD_3D_FREE(ptr)                                             \
    do                                                               \
    {                                                                \
        if (ptr)                                                     \
        {                                                            \
            ::tracked_free((ptr), __FILE__, __LINE__, __FUNCTION__); \
            ptr = nullptr;                                           \
        }                                                            \
    } while (0)
#else
#define HUD_3D_MEMORY_TRACK_ALLOC(ptr, size)
#define HUD_3D_MEMORY_TRACK_FREE(ptr)
#define HUD_3D_MEMORY_PROTECTED_ALLOC(size)
#define HUD_3D_MEMORY_PROTECTED_FREE(ptr)
#define HUD_3D_MEMORY_VALIDATE(ptr) true
#define HUD_3D_MEMORY_INITIALIZE()
#define HUD_3D_MEMORY_SHUTDOWN()
#define HUD_3D_MEMORY_ENABLE_AUTO_PROTECTION()
#define HUD_3D_MEMORY_DISABLE_AUTO_PROTECTION()
#define HUD_3D_MEMORY_IS_AUTO_PROTECTION_ENABLED()
#define HUD_3D_NEW(type, ...) \
    []() -> type * { \
        type* ptr = static_cast<type *>(::operator new(sizeof(type))); \
        if (ptr) new(ptr) type(__VA_ARGS__); \
        return ptr; }()
#define HUD_3D_NEW_ARRAY(type, count) \
    []() -> type * { \
        type* ptr = static_cast<type *>(::operator new[](sizeof(type) * (count))); \
        if (ptr) for(size_t i = 0; i < count; ++i) new(&ptr[i]) type(); \
        return ptr; }()
#define HUD_3D_DELETE(ptr)                                         \
    do                                                             \
    {                                                              \
        if (ptr)                                                   \
        {                                                          \
            using Type = std::remove_pointer_t<decltype(ptr)>;     \
            if constexpr (!std::is_trivially_destructible_v<Type>) \
            {                                                      \
                ptr->~Type();                                      \
            }                                                      \
            ::operator delete(ptr);                                \
            ptr = nullptr;                                         \
        }                                                          \
    } while (0)
#define HUD_3D_DELETE_ARRAY(ptr, count)                            \
    do                                                             \
    {                                                              \
        if (ptr)                                                   \
        {                                                          \
            using Type = std::remove_pointer_t<decltype(ptr)>;     \
            if constexpr (!std::is_trivially_destructible_v<Type>) \
            {                                                      \
                for (size_t i = 0; i < count; ++i)                 \
                    ptr[i].~Type();                                \
            }                                                      \
            ::operator delete[](ptr);                              \
            ptr = nullptr;                                         \
        }                                                          \
    } while (0)
#define HUD_3D_MALLOC(size) ::malloc(size)
#define HUD_3D_CALLOC(num, size) ::calloc(num, size)
#define HUD_3D_REALLOC(ptr, size) ::realloc(ptr, size)
#define HUD_3D_FREE(ptr)   \
    do                     \
    {                      \
        if (ptr)           \
        {                  \
            ::free(ptr);   \
            ptr = nullptr; \
        }                  \
    } while (0)
#endif // __3D_HUD_PERF_ANALYSIS_MEMORY__