/**
 * @file memory_monitor.h
 * @brief Memory Monitor for Memory Pool - Public Domain - 2025 3D HUD Project
 *
 * @author Yameng.He
 * @date 2025-12-22
 * @version 1.0
 *
 * @section overview Overview
 * This class provides integrated memory monitoring for MemoryPool. It is designed
 * to be used as a member of MemoryPool, enabling optional memory tracking with
 * zero overhead when monitoring is disabled.
 *
 * @section license License
 * This library is put in the public domain; you can redistribute it and/or
 * modify it without any restrictions.
 *
 * @section features Features
 * - Zero overhead when monitoring is disabled
 * - Memory allocation tracking with source location
 * - Memory stomp detection using guard bytes
 * - Thread-safe monitoring operations
 * - Detailed statistics and leak reporting
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>

namespace hud_3d
{
    namespace utils
    {
        // =============================================================================
        // Memory Monitor Configuration
        // =============================================================================

        /**
         * @brief Memory monitor configuration
         */
        struct MemoryMonitorConfig
        {
            bool enable_guard_bytes;     //!< Enable guard bytes for stomp detection
            bool enable_fill_patterns;   //!< Enable memory fill patterns
            bool track_source_location;  //!< Track source file, line, and function
            bool enable_leak_reporting;  //!< Enable automatic leak reporting
            uint32_t guard_bytes_size;   //!< Size of guard bytes
            uint8_t alloc_pattern;       //!< Pattern for allocated memory
            uint8_t free_pattern;        //!< Pattern for freed memory
            uint8_t guard_pattern_front; //!< Pattern for front guard bytes
            uint8_t guard_pattern_back;  //!< Pattern for back guard bytes

            MemoryMonitorConfig();
        };

        // =============================================================================
        // Memory Allocation Information
        // =============================================================================

        /**
         * @brief Detailed information about a memory allocation
         */
        struct MemoryAllocationInfo
        {
            void *ptr;              //!< Pointer to allocated memory
            uint64_t size;          //!< Requested allocation size
            uint64_t actual_size;   //!< Actual allocated size (including guard bytes)
            std::string file;       //!< Source file where allocation occurred
            int32_t line;           //!< Source line number
            std::string function;   //!< Function name
            uint64_t timestamp;     //!< Allocation timestamp (microseconds)
            uint64_t allocation_id; //!< Unique allocation identifier
            bool is_freed;          //!< Whether the block has been freed
            bool has_guard_bytes;   //!< Whether guard bytes are present

            MemoryAllocationInfo();
            MemoryAllocationInfo(void *p, uint64_t sz, const std::string &f, int32_t ln,
                                 const std::string &func, uint64_t alloc_id);
        };

        // =============================================================================
        // Statistics Structure
        // =============================================================================

        /**
         * @brief Memory monitoring statistics
         */
        struct MemoryMonitorStatistics
        {
            uint64_t total_allocations;     //!< Total allocations
            uint64_t total_deallocations;   //!< Total deallocations
            uint64_t current_allocations;   //!< Current active allocations
            uint64_t current_memory_usage;  //!< Current memory usage
            uint64_t peak_memory_usage;     //!< Peak memory usage
            uint64_t total_allocated_bytes; //!< Total bytes allocated
            uint64_t allocation_failures;   //!< Allocation failures
            uint64_t deallocation_failures; //!< Deallocation failures
            uint64_t memory_stomps;         //!< Memory stomps detected
            uint64_t memory_leaks;          //!< Memory leaks detected

            MemoryMonitorStatistics();
            void Reset();
        };

        // =============================================================================
        // Memory Monitor Class
        // =============================================================================

        /**
         * @brief Memory monitoring tool for MemoryPool
         *
         * @class MemoryMonitor
         *
         * This class provides integrated memory monitoring that can be used
         * as a member of MemoryPool. When monitoring is disabled, it has
         * zero performance overhead.
         */
        class MemoryMonitor
        {
        public:
            // =============================================================================
            // Constructor and Destructor
            // =============================================================================

            /**
             * @brief Construct a new MemoryMonitor object
             *
             * @param enabled Whether monitoring is enabled
             * @param config Configuration options
             */
            explicit MemoryMonitor(bool enabled = false,
                                   const MemoryMonitorConfig &config = MemoryMonitorConfig());

            /**
             * @brief Destroy the MemoryMonitor object
             *
             * Automatically reports memory leaks if leak reporting is enabled.
             */
            ~MemoryMonitor();

        public:
            // =============================================================================
            // Monitoring Control Functions
            // =============================================================================

            /**
             * @brief Enable memory monitoring
             *
             * @param config Configuration options
             */
            void Enable(const MemoryMonitorConfig &config = MemoryMonitorConfig());

            /**
             * @brief Disable memory monitoring
             */
            void Disable();

            /**
             * @brief Check if monitoring is enabled
             *
             * @return true if monitoring is enabled, false otherwise
             */
            bool IsEnabled() const;

            // =============================================================================
            // Core Monitoring Functions
            // =============================================================================

            /**
             * @brief Track a memory allocation
             *
             * @param ptr Pointer to allocated memory
             * @param size Allocation size
             * @param file Source file
             * @param line Source line
             * @param function Function name
             * @return true if tracking succeeded, false otherwise
             */
            bool TrackAllocation(void *ptr, uint64_t size, const std::string &file = "",
                                 int32_t line = 0, const std::string &function = "");

            /**
             * @brief Track a memory deallocation
             *
             * @param ptr Pointer to deallocated memory
             * @param file Source file
             * @param line Source line
             * @param function Function name
             * @return true if deallocation was valid, false otherwise
             */
            bool TrackDeallocation(void *ptr, const std::string &file = "",
                                   int32_t line = 0, const std::string &function = "");

            /**
             * @brief Validate memory block integrity
             *
             * @param ptr Pointer to memory block
             * @return true if block is valid, false if corruption detected
             */
            bool ValidateMemoryBlock(void *ptr);

            /**
             * @brief Validate all active memory blocks
             *
             * @return Number of corrupted blocks found
             */
            uint64_t ValidateAllMemoryBlocks();

            // =============================================================================
            // Statistics and Reporting Functions
            // =============================================================================

            /**
             * @brief Get current monitoring statistics
             *
             * @param stats Reference to statistics structure to fill
             */
            void GetStatistics(MemoryMonitorStatistics &stats) const;

            /**
             * @brief Reset all statistics
             */
            void ResetStatistics();

            /**
             * @brief Report memory leaks
             *
             * @param detailed Whether to include detailed information
             * @return Number of memory leaks found
             */
            uint64_t ReportLeaks(bool detailed = false) const;

            /**
             * @brief Get current memory usage
             *
             * @return Current memory usage in bytes
             */
            uint64_t GetCurrentMemoryUsage() const;

            /**
             * @brief Get peak memory usage
             *
             * @return Peak memory usage in bytes
             */
            uint64_t GetPeakMemoryUsage() const;

            /**
             * @brief Get number of active allocations
             *
             * @return Number of currently active allocations
             */
            uint64_t GetActiveAllocationCount() const;

            // =============================================================================
            // Configuration Functions
            // =============================================================================

            /**
             * @brief Get current configuration
             *
             * @return Current configuration
             */
            MemoryMonitorConfig GetConfiguration() const;

            /**
             * @brief Update configuration
             *
             * @param config New configuration
             */
            void SetConfiguration(const MemoryMonitorConfig &config);

        public:
            // =============================================================================
            // Memory Allocation Coordination Functions
            // =============================================================================

            /**
             * @brief Calculate the actual allocation size needed including guard bytes
             * 
             * This function calculates the total memory size required for an allocation,
             * taking into account additional overhead such as guard bytes for memory
             * corruption detection. The actual size allocated by MemoryPool will be
             * larger than the requested size to accommodate monitoring features.
             * 
             * @param requested_size The size requested by the user in bytes
             * @return The actual size that should be allocated by MemoryPool, including
             *         any monitoring overhead such as guard bytes
             */
            uint64_t CalculateAllocationSize(uint64_t requested_size) const;

            /**
             * @brief Get the user-accessible pointer from the allocated memory block
             * @param allocated_ptr The pointer returned by MemoryPool allocation
             * @param requested_size The size requested by the user
             * @return Pointer to the user-accessible memory area
             */
            void *GetUserPointer(void *allocated_ptr, uint64_t requested_size) const;

            /**
             * @brief Get the original allocated pointer from the user pointer
             * @param user_ptr The user-accessible pointer
             * @param requested_size The size requested by the user
             * @return The original pointer allocated by MemoryPool
             */
            void *GetAllocatedPointer(void *user_ptr, uint64_t requested_size) const;

        private:
            // Disable copying
            MemoryMonitor(const MemoryMonitor &) = delete;
            MemoryMonitor &operator=(const MemoryMonitor &) = delete;

        private:
            // =============================================================================
            // Private Helper Functions
            // =============================================================================

            /**
             * @brief Set up guard bytes around user memory for corruption detection
             *
             * This function places guard bytes before and after the user-accessible
             * memory area to detect buffer overruns and underruns.
             *
             * @param allocated_ptr The original pointer returned by MemoryPool
             * @param user_ptr The user-accessible pointer (after guard bytes)
             * @param requested_size The size requested by the user
             */
            void SetupGuardBytes(void *allocated_ptr, void *user_ptr, uint64_t requested_size);

            /**
             * @brief Validate guard bytes integrity for a memory allocation
             *
             * Checks if the guard bytes surrounding a memory block have been
             * corrupted, indicating a buffer overrun or underrun.
             *
             * @param info Memory allocation information structure
             * @return true if guard bytes are intact, false if corruption detected
             */
            bool ValidateGuardBytes(const MemoryAllocationInfo &info);

            /**
             * @brief Fill memory region with a specific pattern
             *
             * Used for debugging memory issues by filling allocated or freed
             * memory with recognizable patterns.
             *
             * @param ptr Pointer to the memory region
             * @param size Size of the memory region in bytes
             * @param pattern Pattern byte to fill the memory with
             */
            void FillMemoryWithPattern(void *ptr, uint64_t size, uint8_t pattern);

            /**
             * @brief Check if memory region matches expected pattern
             *
             * Verifies that a memory region contains the expected pattern,
             * used for detecting memory corruption or use-after-free errors.
             *
             * @param ptr Pointer to the memory region
             * @param size Size of the memory region in bytes
             * @param pattern Expected pattern byte
             * @return true if memory matches pattern, false otherwise
             */
            bool CheckMemoryPattern(const void *ptr, uint64_t size, uint8_t pattern);

            /**
             * @brief Get current timestamp in microseconds
             *
             * Provides high-resolution timing for allocation tracking and
             * performance monitoring.
             *
             * @return Current timestamp in microseconds
             */
            uint64_t GetCurrentTimestamp() const;

            /**
             * @brief Generate unique allocation identifier
             *
             * Creates a unique ID for each memory allocation to enable
             * precise tracking and debugging.
             *
             * @return Unique allocation identifier
             */
            uint64_t GenerateAllocationId();

        private:
            bool enabled_;                                                 //!< Whether monitoring is enabled
            MemoryMonitorConfig config_;                                   //!< Monitor configuration
            mutable std::mutex mutex_;                                     //!< Thread safety mutex
            std::unordered_map<void *, MemoryAllocationInfo> allocations_; //!< Active allocations
            MemoryMonitorStatistics statistics_;                           //!< Monitoring statistics
            std::atomic<uint64_t> allocation_id_counter_;                  //!< Allocation ID counter
        };

// =============================================================================
// Monitoring Macros
// =============================================================================

/**
 * @brief Macro for tracking memory allocations with source location
 */
#define HUD_3D_MEMORY_MONITOR_TRACK_ALLOC(monitor, ptr, size) \
    if ((monitor) && (monitor)->IsEnabled())                  \
    (monitor)->TrackAllocation(ptr, size, __FILE__, __LINE__, __FUNCTION__)

/**
 * @brief Macro for tracking memory deallocations with source location
 */
#define HUD_3D_MEMORY_MONITOR_TRACK_FREE(monitor, ptr) \
    if ((monitor) && (monitor)->IsEnabled())           \
    (monitor)->TrackDeallocation(ptr, __FILE__, __LINE__, __FUNCTION__)

    } // namespace utils
} // namespace hud_3d