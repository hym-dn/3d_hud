/**
 * @file perf_utils.cpp
 * @brief Performance Utility Functions Implementation for 3D HUD Project
 *
 * This file implements utility functions for performance analysis that
 * complement the main CPU profiling functionality.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#ifdef __3D_HUD_PERF_ANALYSIS_CPU__

#include "utils/perf/perf_utils.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <chrono>

namespace hud_3d
{
    namespace utils
    {
        namespace perf
        {
            void SetThreadName(const std::string_view thread_name)
            {
                tracy::SetThreadName(thread_name.data());
            }

            std::string_view GetThreadName()
            {
                return "Unknown";
            }

            void MarkMemoryAllocation(void *ptr, size_t size, const std::string_view name)
            {
                if (ptr != nullptr)
                {
                    if (!name.empty())
                    {
                        TracyAllocN(ptr, size, name.data());
                    }
                    else
                    {
                        TracyAlloc(ptr, size);
                    }
                }
            }

            void MarkMemoryDeallocation(void *ptr)
            {
                if (ptr != nullptr)
                {
                    TracyFree(ptr);
                }
            }

            void MarkMemoryReallocation(void *old_ptr, void *new_ptr,
                                        const size_t new_size, const std::string_view name)
            {
                if (old_ptr != nullptr)
                {
                    TracyFree(old_ptr);
                }

                if (new_ptr != nullptr)
                {
                    if (!name.empty())
                    {
                        TracyAllocN(new_ptr, new_size, name.data());
                    }
                    else
                    {
                        TracyAlloc(new_ptr, new_size);
                    }
                }
            }

            /**
            void LogProfilerMessage(const char *message, uint32_t color)
            {
                if (message != nullptr)
                {
                    if (color != 0xFFFFFFFF)
                    {
                        TracyMessageC(message, strlen(message), color);
                    }
                    else
                    {
                        TracyMessage(message, strlen(message));
                    }
                }
            }
            */

            uint64_t GetHighResolutionTimestamp()
            {
                // Use steady_clock instead of high_resolution_clock
                // steady_clock guarantees monotonic behavior and is available on all platforms
                auto now = std::chrono::steady_clock::now();
                auto duration = now.time_since_epoch();
                return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
            }

            uint64_t CalculateDuration(uint64_t start, uint64_t end)
            {
                if (end >= start)
                {
                    return end - start;
                }
                else
                {
                    return (UINT64_MAX - start) + end + 1;
                }
            }

        } // namespace perf
    } // namespace utils
} // namespace hud_3d

#endif // __3D_HUD_PERF_ANALYSIS_CPU__
