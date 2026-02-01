/**
 * @file perf_utils.h
 * @brief Performance Utility Functions for 3D HUD Project
 *
 * This header provides utility functions and macros for performance analysis
 * that complement the main CPU profiling functionality.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#ifdef __3D_HUD_PERF_ANALYSIS_CPU__

#include "utils/utils_define.h"
#include "tracy/Tracy.hpp"
#include <fmt/format.h>
#include <string>

// Stringification macros for source location
#ifndef HUD_3D_STRINGIFY_IMPL
#define HUD_3D_STRINGIFY_IMPL(x) #x
#define HUD_3D_STRINGIFY(x) HUD_3D_STRINGIFY_IMPL(x)
#endif

namespace hud_3d
{
    namespace utils
    {
        namespace perf
        {
            /**
             * @brief Sets a descriptive name for the current thread in the profiler
             *
             * @param thread_name Name to assign to the current thread
             */
            void SetThreadName(const std::string_view thread_name);

            /**
             * @brief Gets the current thread's name from the profiler
             *
             * @return Thread name or "Unknown" if not set
             */
            std::string_view GetThreadName();

            /**
             * @brief Marks a memory allocation in the profiler
             *
             * @param ptr Pointer to the allocated memory
             * @param size Size of the allocation in bytes
             * @param name Descriptive name for the allocation
             */
            void MarkMemoryAllocation(void *ptr, size_t size, const std::string_view name = "");

            /**
             * @brief Marks a memory deallocation in the profiler
             *
             * @param ptr Pointer to the memory being deallocated
             */
            void MarkMemoryDeallocation(void *ptr);

            /**
             * @brief Marks a memory reallocation in the profiler
             *
             * @param old_ptr Original pointer being reallocated
             * @param new_ptr New pointer after reallocation
             * @param new_size New size of the allocation in bytes
             * @param name Descriptive name for the reallocation
             */
            void MarkMemoryReallocation(void *old_ptr, void *new_ptr, const size_t new_size,
                                        const std::string_view name = "");

            /**
             * @brief Logs a message to the Tracy profiler
             *
             * @param message Text message to log
             * @param color Optional color for the message in the profiler UI
             */
            /**
            void LogProfilerMessage(const char *message, uint32_t color = 0xFFFFFFFF);
            */

            /**
             * @brief Logs a formatted message to the Tracy profiler using fmt::format
             *
             * @tparam Args Variadic template arguments for formatting
             * @param format Format string (fmt-style)
             * @param args Arguments for formatting
             */
            template <typename... Args>
            void LogProfilerMessageFormatted(const std::string_view format, Args &&...args)
            {
                if (!format.empty())
                {
                    try
                    {
                        std::string message = fmt::format(format, std::forward<Args>(args)...);
                        TracyMessage(message.c_str(), message.length());
                    }
                    catch (const fmt::format_error &e)
                    {
                        // Fallback to basic error message
                        TracyMessage("Format error in profiler message", 33);
                    }
                }
            }

            /**
             * @brief Gets the current high-resolution timestamp
             * @return Current timestamp in nanoseconds
             */
            uint64_t GetHighResolutionTimestamp();

            /**
             * @brief Calculates the duration between two timestamps
             *
             * @param start Start timestamp from getHighResolutionTimestamp()
             * @param end End timestamp from getHighResolutionTimestamp()
             * @return Duration in nanoseconds
             */
            uint64_t CalculateDuration(uint64_t start, uint64_t end);

        } // namespace perf
    } // namespace utils
} // namespace hud_3d

/**
 * @brief Sets the current thread name in the profiler
 * @param name Thread name to set
 */
#define HUD_3D_PROFILER_SET_THREAD_NAME(name) hud_3d::utils::perf::SetThreadName(name)

/**
 * @brief Marks a memory allocation with automatic source location
 * @param ptr Pointer to allocated memory
 * @param size Allocation size in bytes
 */
#define HUD_3D_PROFILER_MARK_ALLOC(ptr, size) \
    hud_3d::utils::perf::MarkMemoryAllocation(ptr, size, "Allocation at " __FILE__ ":" HUD_3D_STRINGIFY(__LINE__))

/**
 * @brief Marks a memory deallocation
 * @param ptr Pointer to deallocated memory
 */
#define HUD_3D_PROFILER_MARK_FREE(ptr) hud_3d::utils::perf::MarkMemoryDeallocation(ptr)

/**
 * @brief Logs a simple message to the profiler
 * @param message Text message to log
 */
#define HUD_3D_PROFILER_LOG(message) hud_3d::utils::perf::LogProfilerMessage(message)

/**
 * @brief Logs a formatted message to the profiler
 * @param format printf-style format string
 * @param ... Variable arguments for formatting
 */
#define HUD_3D_PROFILER_LOG_FORMATTED(format, ...) \
    hud_3d::utils::perf::LogProfilerMessageFormatted(format, ##__VA_ARGS__)

#else

#define HUD_3D_PROFILER_SET_THREAD_NAME(name)
#define HUD_3D_PROFILER_MARK_ALLOC(ptr, size)
#define HUD_3D_PROFILER_MARK_FREE(ptr)
#define HUD_3D_PROFILER_LOG(message)
#define HUD_3D_PROFILER_LOG_FORMATTED(format, ...)

#endif // __3D_HUD_PERF_ANALYSIS_CPU__