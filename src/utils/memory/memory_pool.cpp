/**
 * @file memory_pool.cpp
 * @brief Memory Pool C++ Implementation - Public Domain - 2025 3D HUD Project
 *
 * @author Yameng.He
 * @date 2025-12-20
 * @version 1.0
 *
 * @section overview Overview
 * This file contains the implementation of the memory pool C++ interface,
 * providing a clean wrapper around the underlying rpmalloc implementation.
 *
 * @section license License
 * This library is put in the public domain; you can redistribute it and/or
 * modify it without any restrictions.
 *
 * @section implementation Implementation Details
 * - All functions are implemented as static methods of the MemoryPool class
 * - Direct mapping to rpmalloc functions for memory operations
 * - RAII wrappers for automatic initialization and cleanup
 * - Thread-safe operations through rpmalloc's thread-local storage
 */

#include "utils/memory/memory_pool.h"
#include "rpmalloc.h"
#include "utils/log/log_manager.h"

namespace hud_3d
{
    namespace utils
    {

#ifdef __3D_HUD_MEMORY_MONITOR__
        // =============================================================================
        // Memory Monitor Static Member
        // =============================================================================

        MemoryMonitor MemoryPool::monitor_(true); // Enabled by default when monitoring is compiled in
#endif

        // =============================================================================
        // MemoryPoolStatistics Implementation
        // =============================================================================

        MemoryPoolStatistics::MemoryPoolStatistics()
            : mapped_memory(0),
              mapped_memory_peak(0),
              committed_memory(0),
              active_memory(0),
              active_memory_peak(0),
              heap_count(0),
              thread_cache_size(0),
              span_cache_size(0),
              thread_to_global(0),
              global_to_thread(0)
        {
        }

        // =============================================================================
        // MemoryPool Implementation
        // =============================================================================

        bool MemoryPool::Initialize()
        {
            return rpmalloc_initialize(nullptr) == 0;
        }

        bool MemoryPool::Initialize(bool enable_huge_pages, bool disable_decommit,
                                    bool unmap_on_finalize, bool disable_thp)
        {
            rpmalloc_config_t config = {};
            config.enable_huge_pages = enable_huge_pages ? 1 : 0;
            config.disable_decommit = disable_decommit ? 1 : 0;
            config.unmap_on_finalize = unmap_on_finalize ? 1 : 0;
#if defined(__linux__) || defined(__ANDROID__)
            config.disable_thp = disable_thp ? 1 : 0;
#else
            (void)disable_thp; // Suppress unused parameter warning on non-Linux platforms
#endif
            return rpmalloc_initialize_config(nullptr, &config) == 0;
        }

        void MemoryPool::Finalize()
        {
            rpmalloc_finalize();
        }

        void MemoryPool::ThreadInitialize()
        {
            rpmalloc_thread_initialize();
        }

        void MemoryPool::ThreadFinalize()
        {
            rpmalloc_thread_finalize();
        }

        bool MemoryPool::IsThreadInitialized()
        {
            return rpmalloc_is_thread_initialized() != 0;
        }

        void MemoryPool::ThreadCollect()
        {
            rpmalloc_thread_collect();
        }

        void *MemoryPool::Allocate(uint64_t size)
        {
#ifdef __3D_HUD_MEMORY_MONITOR__
            if (monitor_.IsEnabled())
            {
                // Calculate actual allocation size including monitoring overhead
                uint64_t actual_size = monitor_.CalculateAllocationSize(size);
                void *allocated_ptr = rpmalloc(static_cast<size_t>(actual_size));

                if (allocated_ptr)
                {
                    // Track allocation with source location
                    if (!monitor_.TrackAllocation(allocated_ptr, size, __FILE__, __LINE__, __FUNCTION__))
                    {
                        rpfree(allocated_ptr);
                        return nullptr;
                    }

                    // Return user-accessible pointer (after guard bytes)
                    return monitor_.GetUserPointer(allocated_ptr, size);
                }
                return nullptr;
            }
            else
#endif
            {
                return rpmalloc(static_cast<size_t>(size));
            }
        }

        void *MemoryPool::AllocateZero(uint64_t size)
        {
#ifdef __3D_HUD_MEMORY_MONITOR__
            if (monitor_.IsEnabled())
            {
                void *ptr = Allocate(size);
                if (ptr)
                {
                    std::memset(ptr, 0, static_cast<size_t>(size));
                }
                return ptr;
            }
            else
#endif
            {
                return rpzalloc(static_cast<size_t>(size));
            }
        }

        void *MemoryPool::AllocateArray(uint64_t num, uint64_t size)
        {
#ifdef __3D_HUD_MEMORY_MONITOR__
            if (monitor_.IsEnabled())
            {
                void *ptr = Allocate(num * size);
                if (ptr)
                {
                    std::memset(ptr, 0, static_cast<size_t>(num * size));
                }
                return ptr;
            }
            else
#endif
            {
                return rpcalloc(static_cast<size_t>(num), static_cast<size_t>(size));
            }
        }

        void *MemoryPool::AllocateAligned(uint64_t alignment, uint64_t size)
        {
#ifdef __3D_HUD_MEMORY_MONITOR__
            if (monitor_.IsEnabled())
            {
                // Calculate actual allocation size including monitoring overhead
                uint64_t actual_size = monitor_.CalculateAllocationSize(size);
                void *allocated_ptr = rpaligned_alloc(
                    static_cast<size_t>(alignment), static_cast<size_t>(actual_size));
                if (allocated_ptr)
                {
                    // Track allocation with source location
                    if (!monitor_.TrackAllocation(allocated_ptr, size, __FILE__, __LINE__, __FUNCTION__))
                    {
                        rpfree(allocated_ptr);
                        return nullptr;
                    }
                    // Return user-accessible pointer (after guard bytes)
                    return monitor_.GetUserPointer(allocated_ptr, size);
                }
                return nullptr;
            }
            else
#endif
            {
                return rpaligned_alloc(static_cast<size_t>(alignment), static_cast<size_t>(size));
            }
        }

        void *MemoryPool::AllocateAlignedZero(uint64_t alignment, uint64_t size)
        {
#ifdef __3D_HUD_MEMORY_MONITOR__
            if (monitor_.IsEnabled())
            {
                void *ptr = AllocateAligned(alignment, size);
                if (ptr)
                {
                    std::memset(ptr, 0, static_cast<size_t>(size));
                }
                return ptr;
            }
            else
#endif
            {
                return rpaligned_zalloc(static_cast<size_t>(alignment), static_cast<size_t>(size));
            }
        }

        void *MemoryPool::Reallocate(void *ptr, uint64_t size)
        {
#ifdef __3D_HUD_MEMORY_MONITOR__
            if (monitor_.IsEnabled())
            {
                if (!ptr)
                {
                    return Allocate(size);
                }
                // Get original allocated pointer
                void *allocated_ptr = monitor_.GetAllocatedPointer(ptr, UsableSize(ptr));
                // Calculate new actual size
                uint64_t actual_size = monitor_.CalculateAllocationSize(size);
                // Reallocate
                void *new_allocated_ptr = rprealloc(allocated_ptr, static_cast<size_t>(actual_size));
                if (new_allocated_ptr)
                {
                    // Track the new allocation (realloc is treated as free + allocate)
                    if (monitor_.TrackDeallocation(ptr, __FILE__, __LINE__, __FUNCTION__) &&
                        monitor_.TrackAllocation(new_allocated_ptr, size, __FILE__, __LINE__, __FUNCTION__))
                    {
                        return monitor_.GetUserPointer(new_allocated_ptr, size);
                    }
                    rpfree(new_allocated_ptr);
                    return nullptr;
                }
                return nullptr;
            }
            else
#endif
            {
                return rprealloc(ptr, static_cast<size_t>(size));
            }
        }

        void MemoryPool::Free(void *ptr)
        {
#ifdef __3D_HUD_MEMORY_MONITOR__
            if (monitor_.IsEnabled() && ptr)
            {
                // Get original allocated pointer
                void *allocated_ptr = monitor_.GetAllocatedPointer(ptr, UsableSize(ptr));
                // Track deallocation
                monitor_.TrackDeallocation(ptr, __FILE__, __LINE__, __FUNCTION__);
                // Free the original allocated memory
                rpfree(allocated_ptr);
            }
            else
#endif
            {
                rpfree(ptr);
            }
        }

        uint64_t MemoryPool::UsableSize(void *ptr)
        {
            return static_cast<uint64_t>(rpmalloc_usable_size(ptr));
        }

        void MemoryPool::GetGlobalStatistics(MemoryPoolStatistics &stats)
        {
            rpmalloc_global_statistics_t rpmalloc_stats;
            rpmalloc_global_statistics(&rpmalloc_stats);
            stats.mapped_memory = rpmalloc_stats.mapped;
            stats.mapped_memory_peak = rpmalloc_stats.mapped_peak;
            stats.committed_memory = rpmalloc_stats.committed;
            stats.active_memory = rpmalloc_stats.active;
            stats.active_memory_peak = rpmalloc_stats.active_peak;
            stats.heap_count = rpmalloc_stats.heap_count;
        }

        void MemoryPool::GetThreadStatistics(MemoryPoolStatistics &stats)
        {
            rpmalloc_thread_statistics_t rpmalloc_stats;
            rpmalloc_thread_statistics(&rpmalloc_stats);
            stats.thread_cache_size = rpmalloc_stats.sizecache;
            stats.span_cache_size = rpmalloc_stats.spancache;
            stats.thread_to_global = rpmalloc_stats.thread_to_global;
            stats.global_to_thread = rpmalloc_stats.global_to_thread;
        }

        void MemoryPool::DumpStatistics(void *file)
        {
            rpmalloc_dump_statistics(file);
        }

        // =============================================================================
        // RAII Wrapper Classes Implementation
        // =============================================================================

        MemoryPoolInitializer::MemoryPoolInitializer(bool enable_huge_pages,
                                                     bool disable_decommit,
                                                     bool unmap_on_finalize,
                                                     bool disable_thp)
        {
            MemoryPool::Initialize(enable_huge_pages, disable_decommit,
                                   unmap_on_finalize, disable_thp);
        }

        MemoryPoolInitializer::~MemoryPoolInitializer()
        {
            MemoryPool::Finalize();
        }

        ThreadMemoryPoolInitializer::ThreadMemoryPoolInitializer()
        {
            MemoryPool::ThreadInitialize();
        }

        ThreadMemoryPoolInitializer::~ThreadMemoryPoolInitializer()
        {
            MemoryPool::ThreadFinalize();
        }

    } // namespace utils
} // namespace hud3d