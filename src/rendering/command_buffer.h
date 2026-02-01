/**
 * @file command_buffer.h
 * @brief High-performance, Zero-copy Command Buffer System (v3.0)
 *
 * This module provides a high-performance command buffer system designed for
 * real-time 3D rendering applications. It features a paged memory architecture
 * that eliminates expensive memory reallocations and supports zero-copy
 * command recording for optimal performance.
 *
 * Key Features:
 * - Paged Memory Architecture (No resize copies)
 * - Zero-copy Command Recording (In-place construction)
 * - Priority-based Command Execution (High/Normal/Low)
 * - Multi-window Support (Thread-safe buffer management)
 * - API-agnostic Design (Supports multiple graphics backends)
 * - Lock-free Resource Management (Atomic operations)
 *
 * Architecture:
 * - CommandBuffer: Individual buffer for command recording and execution
 * - CommandBufferManager: Manages buffer pools for multiple windows/threads
 * - CommandStorage: Template-based command data storage with execution logic
 *
 * @author Yameng.He (Optimized by DeepSeek)
 * @date 2025-12-21
 * @version 3.0
 */

#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <array>
#include <cstdint>
#include <type_traits>
#include <mutex>
#include <cassert>
#include <atomic>
#include <limits>
#include "rendering_define.h"
#include "platform/graphics_context.h"
#include "utils/memory/memory_pool.h"

namespace hud_3d
{
    namespace rendering
    {
        // =========================================================================
        // Memory Pool Type Definition
        // =========================================================================

        // MemoryPool is a static utility class - no instances needed
        // CommandBuffer will use MemoryPool static methods directly

        // =========================================================================
        // Paged Command Buffer (Linear & Resizable without Copy)
        // =========================================================================

        /**
         * @brief High-performance command buffer with paged memory architecture
         *
         * The CommandBuffer class provides a zero-copy, paged memory system for
         * recording and executing graphics commands. It uses a linked list of
         * fixed-size pages to avoid expensive memory reallocations.
         */
        class CommandBuffer
        {
        public:
            /**
             * @brief Fixed size of each memory page (1KB)
             *
             * This constant defines the allocation granularity for command buffer pages.
             * Each page can store multiple commands, and when a page is full, a new
             * page is allocated from the memory pool.
             */
            static constexpr uint32_t PAGE_SIZE = 64 * 1024;

            /**
             * @brief Construct a command buffer
             */
            CommandBuffer() noexcept;

            /**
             * @brief Destructor - automatically frees all allocated pages
             */
            ~CommandBuffer() noexcept;

            /**
             * @brief Move constructor for efficient buffer transfer
             *
             * Transfers ownership of pages and state from another buffer.
             * The source buffer becomes empty after the move operation.
             *
             * @param other Source buffer to move from
             */
            CommandBuffer(CommandBuffer &&other) noexcept;

            /**
             * @brief Move assignment operator
             *
             * Transfers ownership from another buffer, freeing any existing
             * resources in the current buffer first.
             *
             * @param other Source buffer to move from
             * @return Reference to this buffer
             */
            CommandBuffer &operator=(CommandBuffer &&other) noexcept;

        public:
            /**
             * @brief Record a command directly into buffer memory (zero-copy argument passing)
             *
             * This template method constructs the command in-place within the buffer,
             * avoiding unnecessary copies of command data.
             *
             * @tparam CommandT Type of command to record
             * @tparam Args Argument types for command construction
             * @param args Arguments to forward to command constructor
             */
            template <typename CommandT, typename... Args>
            void RecordCommand(Args &&...args);

            /**
             * @brief Execute all recorded commands in the buffer
             *
             * Commands are executed in priority order (High -> Normal -> Low)
             * and the execution is noexcept to ensure exception safety.
             */
            void Execute() const noexcept;

            /**
             * @brief Reset the buffer, freeing all commands and returning memory to pool
             */
            void Reset() noexcept;

            /**
             * @brief Check if the buffer contains any commands
             * @return true if buffer is empty, false otherwise
             */
            bool IsEmpty() const noexcept;

            /**
             * @brief Get total number of commands in the buffer
             * @return Total command count across all priorities
             */
            uint32_t GetCommandCount() const noexcept;

            /**
             * @brief Get number of commands for a specific priority level
             * @param priority Priority level to query
             * @return Number of commands with the specified priority
             */
            uint32_t GetCommandCount(CommandPriority priority) const noexcept;

            /**
             * @brief Get total memory usage of the buffer
             * @return Memory usage in bytes including all pages and commands
             */
            uint32_t GetMemoryUsed() const noexcept;

            /**
             * @brief Get number of allocated pages
             * @return Total number of pages allocated for this buffer
             */
            uint32_t GetPageCount() const noexcept;

            /**
             * @brief Get detailed performance statistics
             * @return CommandBufferStats structure containing various metrics
             */
            CommandBufferStats GetStats() const noexcept;

            // Thread safety methods
            /**
             * @brief Mark the buffer as ready for execution
             *
             * This method should be called by the business thread after all commands
             * have been recorded to indicate that the buffer is complete and ready
             * for execution by the render thread.
             */
            void MarkReady() noexcept;

            /**
             * @brief Check if the buffer is ready for execution
             *
             * This method is used by the render thread to determine if the buffer
             * contains a complete set of commands that can be safely executed.
             *
             * @return true if the buffer is ready for execution, false otherwise
             */
            bool IsReady() const noexcept;

        private:
            /**
             * @brief Page structure using flexible array member
             *
             * Each page contains a linked list pointer, usage counter,
             * and a flexible array for actual command storage.
             */
            struct Page
            {
                Page *next;      /**< Pointer to next page in linked list */
                uint32_t used;   /**< Number of bytes used in this page */
                uint8_t data[1]; /**< Flexible array member for command storage */

                /**
                 * @brief Calculate the total allocation size for a page
                 * @return Size in bytes needed for page allocation
                 */
                static uint32_t GetAllocationSize()
                {
                    return static_cast<uint32_t>(sizeof(Page) - sizeof(uint8_t[1]) + PAGE_SIZE);
                }

                uint8_t *GetData() { return data; }             /**< Get pointer to data storage */
                const uint8_t *GetData() const { return data; } /**< Get const pointer to data storage */
            };

        private:
            /**
             * @brief Copy constructor is deleted to prevent double-free issues
             *
             * Command buffers cannot be copied due to their ownership of
             * dynamically allocated pages. Use move semantics instead.
             */
            CommandBuffer(const CommandBuffer &) = delete;

            /**
             * @brief Copy assignment operator is deleted
             *
             * Copy assignment is disabled for the same reasons as copy construction.
             */
            CommandBuffer &operator=(const CommandBuffer &) = delete;

        private:
            /**
             * @brief Allocate aligned space for a command with specific priority
             *
             * This internal method handles the actual memory allocation within
             * the buffer's pages. It will allocate a new page if needed.
             *
             * @param size Required allocation size in bytes
             * @param priority Priority level for the command
             * @return Pointer to allocated memory, or nullptr on failure
             */
            void *AllocateSpace(uint32_t size, CommandPriority priority) noexcept;

            /**
             * @brief Allocate a new page for specific priority
             *
             * Creates a new page from the memory pool and links it into
             * the appropriate priority chain.
             *
             * @param priority Priority level for the new page
             * @return Pointer to the newly allocated page
             */
            Page *NewPage(CommandPriority priority) noexcept;

            /**
             * @brief Free all pages back to the memory pool
             *
             * Releases all allocated pages and resets the buffer state.
             * This is called during reset and destruction.
             */
            void FreePages() noexcept;

            /**
             * @brief Execute commands of specific priority
             *
             * Iterates through all commands of the specified priority level
             * and executes them in order.
             *
             * @param priority Priority level to execute
             */
            void ExecutePriority(CommandPriority priority) const noexcept;

        private:
            /** @brief Head pages for each priority level (linked list heads) */
            std::array<Page *, static_cast<size_t>(CommandPriority::Count)> head_pages_;
            /** @brief Current pages for each priority level (for appending) */
            std::array<Page *, static_cast<size_t>(CommandPriority::Count)> current_pages_;
            /** @brief Command counts for each priority level */
            std::array<uint32_t, static_cast<size_t>(CommandPriority::Count)> command_counts_;
            /** @brief Page counts for each priority level */
            std::array<uint32_t, static_cast<size_t>(CommandPriority::Count)> page_counts_;
            uint32_t memory_used_;             /**< Total memory used by commands */
            mutable CommandBufferStats stats_; /**< Performance statistics */
            std::atomic<bool> is_ready_;       /**< Atomic flag indicating buffer is ready for execution */
        };

        // =========================================================================
        // Template Implementation (Must be in header file)
        // =========================================================================

        /**
         * @brief Record a command with zero-copy in-place construction
         *
         * This template method provides the core functionality for recording
         * commands into the buffer. It performs the following steps:
         * 1. Determines command type and priority from the template parameters
         * 2. Allocates space within the appropriate priority chain
         * 3. Constructs the command in-place using perfect forwarding
         * 4. Updates internal statistics
         *
         * @tparam CommandT Type of command to record (must define TYPE_ID)
         * @tparam Args Argument types for command construction
         * @param args Arguments to forward to command constructor
         */
        template <typename CommandT, typename... Args>
        void CommandBuffer::RecordCommand(Args &&...args)
        {
            // Use TYPE_ID defined in the command type
            constexpr CommandType TypeID = CommandT::TYPE_ID;

            // Priority is now determined by the command type itself
            // Backend implementations should define appropriate priorities
            constexpr CommandPriority Prio = CommandPriority::Normal; // Default priority

            // Use the corresponding storage type
            using StorageT = CommandStorage<CommandT, TypeID, Prio>;

            // 1. Allocate space for the specific priority
            void *ptr = AllocateSpace(sizeof(StorageT), Prio);
            if (!ptr)
            {
                // Handle allocation failure
                return;
            }

            // 2. In-place construction (zero-copy)
            new (ptr) StorageT(std::forward<Args>(args)...);

            // 3. Update statistics
            command_counts_[static_cast<size_t>(Prio)]++;
            stats_.commands_recorded++;
        }

        // =========================================================================
        // Command Structure Definitions (API-agnostic)
        // =========================================================================

        // Note: Concrete command implementations are now provided by backend modules
        // (e.g., opengl_backend.h, vulkan_backend.h, etc.)
        // This header only contains the generic command buffer framework.

        // =========================================================================
        // Type Aliases (Defined in backend-specific headers)
        // =========================================================================

        // Note: Type aliases for specific command storage types are now defined
        // in backend-specific headers (e.g., opengl_backend.h)

        // =========================================================================
        // Command Buffer Manager (Lock-free Stack)
        // =========================================================================

        /**
         * @brief Manages a pool of command buffers using lock-free algorithms
         *
         * The CommandBufferManager provides efficient allocation and recycling
         * of command buffers using atomic operations to avoid locking overhead.
         */
        class CommandBufferManager
        {
        public:
            /** @brief Maximum number of windows supported by the manager */
            static constexpr uint32_t MAX_WINDOWS = 8;

            /** @brief Number of command buffers allocated per window */
            static constexpr uint32_t BUFFERS_PER_WINDOW = 16;

        public:
            /**
             * @brief Construct the buffer manager with specified window count
             * @param window_count Number of windows to support (1 to MAX_WINDOWS)
             */
            explicit CommandBufferManager(uint32_t window_count = 1) noexcept;

            /**
             * @brief Destructor - ensures all buffers are properly cleaned up
             */
            ~CommandBufferManager() noexcept;

        public:
            /**
             * @brief Acquire a command buffer for a specific window
             * @param window_id Window ID (0 to window_count-1)
             * @return Pointer to acquired command buffer, or nullptr if failed
             */
            CommandBuffer *AcquireBuffer(uint32_t window_id) noexcept;

            /**
             * @brief Release a command buffer back to its window's pool
             * @param buffer Pointer to the buffer to release
             * @param window_id Window ID that owns the buffer
             */
            void ReleaseBuffer(CommandBuffer *buffer, uint32_t window_id) noexcept;

            /**
             * @brief Execute all command buffers for a specific window
             * @param window_id Window ID to execute buffers for
             */
            void ExecuteWindowBuffers(uint32_t window_id) noexcept;

            /**
             * @brief Execute all command buffers for all windows
             */
            void ExecuteAllWindows() noexcept;

            /**
             * @brief Get number of available buffers for a window
             * @param window_id Window ID to query
             * @return Number of buffers currently available for acquisition
             */
            uint32_t GetAvailableBuffers(uint32_t window_id) const noexcept;

            /**
             * @brief Get total number of buffers allocated for a window
             * @param window_id Window ID to query
             * @return Total number of buffers allocated for the window
             */
            uint32_t GetTotalBuffers(uint32_t window_id) const noexcept;

            /**
             * @brief Get number of active (in-use) buffers for a window
             * @param window_id Window ID to query
             * @return Number of buffers currently in use
             */
            uint32_t GetActiveBuffers(uint32_t window_id) const noexcept;

            /**
             * @brief Get the number of windows supported by this manager
             * @return Number of windows configured during construction
             */
            uint32_t GetWindowCount() const noexcept { return window_count_; }

        private:
            /**
             * @brief Per-window buffer management structure
             *
             * This structure manages the lifecycle of command buffers for
             * a specific window, including allocation, recycling, and
             * thread-safe access.
             */
            struct WindowBufferPool
            {
                /** @brief Array of managed buffers (unique ownership) */
                std::array<std::unique_ptr<CommandBuffer>, BUFFERS_PER_WINDOW> buffers;

                /** @brief Mutex for thread-safe buffer management */
                std::mutex buffer_mutex;

                /**
                 * @brief Free stack for managing available buffer indices
                 */
                struct FreeStack
                {
                    std::array<uint32_t, BUFFERS_PER_WINDOW> indices; /**< Stack of free buffer indices */
                    int32_t top{-1};                                  /**< Top index (initially empty) */
                } free_stack;

                /** @brief Fast buffer pointer lookup array */
                std::array<CommandBuffer *, BUFFERS_PER_WINDOW> buffer_pointers;
            };

        private:
            uint32_t window_count_;                            /**< Number of windows supported */
            std::unique_ptr<WindowBufferPool[]> window_pools_; /**< Array of window buffer pools */
        };

    } // namespace rendering
} // namespace hud_3d