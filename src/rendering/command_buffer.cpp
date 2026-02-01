/**
 * @file command_buffer.cpp
 * @brief Command Buffer Implementation - Core Functionality
 *
 * This file contains the implementation of the high-performance command buffer
 * system. It provides the actual memory management, command execution, and
 * multi-window buffer management logic.
 *
 * Key Implementation Details:
 * - Paged memory allocation and deallocation
 * - Priority-based command execution
 * - Thread-safe buffer pool management
 * - Zero-copy command recording and execution
 *
 * @author Yameng.He
 * @date 2025-12-20
 * @version 3.0
 */

#include <cstring>
#include "utils/math/foundation/bit_ops.h"
#include "command_buffer.h"

namespace hud_3d
{
    namespace rendering
    {
        // =========================================================================
        // CommandBuffer Implementation
        // =========================================================================

        CommandBuffer::CommandBuffer() noexcept
            : head_pages_{nullptr},
              current_pages_{nullptr},
              command_counts_{0},
              page_counts_{0},
              memory_used_(0),
              stats_{},
              is_ready_(false)
        {
            // Constructor initializes all member variables to empty state
            // The command buffer starts with no allocated pages and zero commands
        }

        CommandBuffer::~CommandBuffer() noexcept
        {
            // Destructor ensures all allocated pages are freed back to the pool
            FreePages();
        }

        CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
            : head_pages_(other.head_pages_),
              current_pages_(other.current_pages_),
              command_counts_(other.command_counts_),
              page_counts_(other.page_counts_),
              memory_used_(other.memory_used_),
              stats_(other.stats_),
              is_ready_(other.is_ready_.load(std::memory_order_relaxed))
        {
            // Move constructor: transfer ownership of pages and state from other buffer
            // After the move, the source buffer becomes empty and ready for reuse

            for (size_t i = 0; i < static_cast<size_t>(CommandPriority::Count); ++i)
            {
                other.head_pages_[i] = nullptr;
                other.current_pages_[i] = nullptr;
                other.command_counts_[i] = 0;
                other.page_counts_[i] = 0;
            }
            other.memory_used_ = 0;
            other.stats_ = {};
            other.is_ready_.store(false, std::memory_order_relaxed);
        }

        CommandBuffer &CommandBuffer::operator=(CommandBuffer &&other) noexcept
        {
            // Move assignment operator: handle self-assignment and transfer ownership
            if (this != &other)
            {
                // Free any existing resources in this buffer before taking ownership
                FreePages();

                // Transfer ownership of pages and state from other buffer
                head_pages_ = other.head_pages_;
                current_pages_ = other.current_pages_;
                command_counts_ = other.command_counts_;
                page_counts_ = other.page_counts_;
                memory_used_ = other.memory_used_;
                stats_ = other.stats_;
                is_ready_.store(other.is_ready_.load(std::memory_order_relaxed), std::memory_order_relaxed);

                // Reset the source buffer to empty state
                for (size_t i = 0; i < static_cast<size_t>(CommandPriority::Count); ++i)
                {
                    other.head_pages_[i] = nullptr;
                    other.current_pages_[i] = nullptr;
                    other.command_counts_[i] = 0;
                    other.page_counts_[i] = 0;
                }
                other.memory_used_ = 0;
                other.stats_ = {};
                other.is_ready_.store(false, std::memory_order_relaxed);
            }
            return *this;
        }

        void CommandBuffer::Execute() const noexcept
        {
            // Execute commands in priority order: High -> Normal -> Low
            // This ensures critical commands are processed before less important ones
            for (size_t prio = 0; prio < static_cast<size_t>(CommandPriority::Count); ++prio)
            {
                ExecutePriority(static_cast<CommandPriority>(prio));
            }
        }

        void CommandBuffer::Reset() noexcept
        {
            // Free all allocated pages and return memory to the pool
            FreePages();

            // Reset all internal state for each priority level
            for (size_t i = 0; i < static_cast<size_t>(CommandPriority::Count); ++i)
            {
                command_counts_[i] = 0;
                page_counts_[i] = 0;
                head_pages_[i] = nullptr;
                current_pages_[i] = nullptr;
            }
            memory_used_ = 0;

            // Reset performance statistics to zero
            stats_.commands_recorded = 0;
            stats_.commands_executed = 0;
            stats_.total_bytes_used = 0;
            stats_.page_count = 0;
            stats_.memory_allocations = 0;

            // Reset the ready flag - buffer is no longer ready for execution
            // Use release semantics to ensure all previous writes are visible
            is_ready_.store(false, std::memory_order_release);
        }

        // =========================================================================
        // CommandBuffer Query Methods Implementation
        // =========================================================================

        bool CommandBuffer::IsEmpty() const noexcept
        {
            // Check if any priority level has commands
            // Returns true only if all command counts are zero
            for (auto count : command_counts_)
            {
                if (count > 0)
                    return false;
            }
            return true;
        }

        uint32_t CommandBuffer::GetCommandCount() const noexcept
        {
            // Sum command counts across all priority levels
            uint32_t total = 0;
            for (auto count : command_counts_)
            {
                total += count;
            }
            return total;
        }

        uint32_t CommandBuffer::GetCommandCount(CommandPriority priority) const noexcept
        {
            // Return the command count for a specific priority level
            return command_counts_[static_cast<size_t>(priority)];
        }

        uint32_t CommandBuffer::GetMemoryUsed() const noexcept
        {
            // Return the total memory usage including all commands and page overhead
            return memory_used_;
        }

        uint32_t CommandBuffer::GetPageCount() const noexcept
        {
            // Sum page counts across all priority levels
            uint32_t total = 0;
            for (auto count : page_counts_)
            {
                total += count;
            }
            return total;
        }

        CommandBufferStats CommandBuffer::GetStats() const noexcept
        {
            return stats_;
        }

        // =========================================================================
        // CommandBuffer Thread Safety Methods Implementation
        // =========================================================================

        void CommandBuffer::MarkReady() noexcept
        {
            // Mark the buffer as ready for execution by the render thread
            // Use release semantics to ensure all recorded commands are visible
            is_ready_.store(true, std::memory_order_release);
        }

        bool CommandBuffer::IsReady() const noexcept
        {
            // Check if the buffer is ready for execution
            // Use acquire semantics to ensure we see all commands recorded before MarkReady()
            return is_ready_.load(std::memory_order_acquire);
        }

        void *CommandBuffer::AllocateSpace(uint32_t size, CommandPriority priority) noexcept
        {
            // Align the requested size to 16-byte boundary for optimal performance
            size = static_cast<uint32_t>(utils::math::AlignUp(size, 16U));

            // Check if we need a new page: either no current page or not enough space
            if (!current_pages_[static_cast<size_t>(priority)] ||
                (current_pages_[static_cast<size_t>(priority)]->used + size > PAGE_SIZE))
            {
                Page *new_page = NewPage(priority);
                if (!new_page)
                {
                    // Allocation failed - return nullptr to indicate failure
                    return nullptr;
                }

                // Link the new page into the priority chain
                if (current_pages_[static_cast<size_t>(priority)])
                {
                    // Link new page after current page
                    current_pages_[static_cast<size_t>(priority)]->next = new_page;
                }
                else
                {
                    // This is the first page for this priority
                    head_pages_[static_cast<size_t>(priority)] = new_page;
                }
                current_pages_[static_cast<size_t>(priority)] = new_page;
            }

            // Allocate space from the current page
            void *ptr = current_pages_[static_cast<size_t>(priority)]->GetData() + current_pages_[static_cast<size_t>(priority)]->used;
            current_pages_[static_cast<size_t>(priority)]->used += size;
            memory_used_ += size;

            return ptr;
        }

        CommandBuffer::Page *CommandBuffer::NewPage(CommandPriority priority) noexcept
        {
            // Allocate a new page from the memory pool using flexible array member
            Page *page = utils::MemoryPool::CreateFlexibleStruct<Page, uint8_t>(PAGE_SIZE);
            if (page)
            {
                // Initialize the new page
                page->next = nullptr;
                page->used = 0;

                // Update statistics
                page_counts_[static_cast<size_t>(priority)]++;
                stats_.memory_allocations++;
                stats_.total_bytes_used += Page::GetAllocationSize();
            }
            return page;
        }

        void CommandBuffer::FreePages() noexcept
        {
            // Iterate through all priority levels and free their pages
            for (size_t i = 0; i < static_cast<size_t>(CommandPriority::Count); ++i)
            {
                // Traverse the linked list of pages for this priority
                Page *current = head_pages_[i];
                while (current)
                {
                    Page *next = current->next;
                    // Return the page to the memory pool
                    utils::MemoryPool::DestroyFlexibleStruct<Page, uint8_t>(current, PAGE_SIZE);
                    current = next;
                }
                // Reset the page pointers for this priority
                head_pages_[i] = nullptr;
                current_pages_[i] = nullptr;
                page_counts_[i] = 0;
            }
            // Reset memory usage counter
            memory_used_ = 0;
        }

        void CommandBuffer::ExecutePriority(CommandPriority priority) const noexcept
        {
            const size_t priority_index = static_cast<size_t>(priority);
            
            // Get the head page for this priority level
            Page *current_page = head_pages_[priority_index];
            
            // Iterate through all pages for this priority
            while (current_page)
            {
                uint32_t offset = 0;
                const uint8_t *page_data = current_page->GetData();
                
                // Process all commands in the current page
                while (offset < current_page->used)
                {
                    // Get the command header (16-byte aligned)
                    const CommandHeader *header = reinterpret_cast<const CommandHeader *>(page_data + offset);
                    
                    // Safety check: ensure we don't read beyond page bounds
                    if (offset + header->size > current_page->used)
                    {
                        // Corrupted command data - skip remaining commands in this page
                        break;
                    }
                    
                    // Execute the command using the function pointer in the header
                    if (header->Execute)
                    {
                        header->Execute(header);
                        
                        // Update execution statistics
                        stats_.commands_executed++;
                    }
                    
                    // Move to next command (aligned to 16 bytes)
                    offset += header->size;
                    
                    // Ensure alignment for next command
                    offset = static_cast<uint32_t>(utils::math::AlignUp(offset, 16U));
                }
                
                // Move to next page in the linked list
                current_page = current_page->next;
            }
        }

        // =========================================================================
        // CommandBufferManager Implementation (Multi-Window Support)
        // =========================================================================

        CommandBufferManager::CommandBufferManager(uint32_t window_count) noexcept
            : window_count_(std::min(window_count, MAX_WINDOWS)),
              window_pools_(std::make_unique<WindowBufferPool[]>(window_count_))
        {
            for (uint32_t window_id = 0; window_id < window_count_; ++window_id)
            {
                auto &pool = window_pools_[window_id];

                for (uint32_t i = 0; i < BUFFERS_PER_WINDOW; ++i)
                {
                    pool.buffers[i] = std::make_unique<CommandBuffer>();
                    pool.buffer_pointers[i] = pool.buffers[i].get();
                    pool.free_stack.indices[i] = static_cast<uint32_t>(i);
                }

                pool.free_stack.top = static_cast<int32_t>(BUFFERS_PER_WINDOW - 1);
            }
        }

        CommandBufferManager::~CommandBufferManager() noexcept
        {
        }

        CommandBuffer *CommandBufferManager::AcquireBuffer(uint32_t window_id) noexcept
        {
            if (window_id >= window_count_)
            {
                return nullptr;
            }

            auto &pool = window_pools_[window_id];
            std::lock_guard<std::mutex> lock(pool.buffer_mutex);

            if (pool.free_stack.top >= 0)
            {
                uint32_t buffer_index = pool.free_stack.indices[pool.free_stack.top];
                pool.free_stack.top--;
                return pool.buffer_pointers[buffer_index];
            }

            return nullptr;
        }

        void CommandBufferManager::ReleaseBuffer(CommandBuffer *buffer, uint32_t window_id) noexcept
        {
            if (window_id >= window_count_ || !buffer)
            {
                return;
            }

            buffer->Reset();

            auto &pool = window_pools_[window_id];
            std::lock_guard<std::mutex> lock(pool.buffer_mutex);

            for (uint32_t i = 0; i < BUFFERS_PER_WINDOW; ++i)
            {
                if (pool.buffer_pointers[i] == buffer)
                {
                    if (pool.free_stack.top + 1 < static_cast<int32_t>(BUFFERS_PER_WINDOW))
                    {
                        pool.free_stack.top++;
                        pool.free_stack.indices[pool.free_stack.top] = static_cast<uint32_t>(i);
                    }
                    break;
                }
            }
        }

        void CommandBufferManager::ExecuteWindowBuffers(uint32_t window_id) noexcept
        {
            if (window_id >= window_count_)
            {
                return;
            }

            auto &pool = window_pools_[window_id];

            for (uint32_t i = 0; i < BUFFERS_PER_WINDOW; ++i)
            {
                bool in_use = true;
                int32_t top = pool.free_stack.top;

                for (int32_t j = 0; j <= top; ++j)
                {
                    if (pool.free_stack.indices[j] == i)
                    {
                        in_use = false;
                        break;
                    }
                }

                if (in_use && pool.buffers[i]->IsReady() && !pool.buffers[i]->IsEmpty())
                {
                    pool.buffers[i]->Execute();
                }
            }
        }

        void CommandBufferManager::ExecuteAllWindows() noexcept
        {
            for (uint32_t window_id = 0; window_id < window_count_; ++window_id)
            {
                ExecuteWindowBuffers(window_id);
            }
        }

        uint32_t CommandBufferManager::GetAvailableBuffers(uint32_t window_id) const noexcept
        {
            if (window_id >= window_count_)
            {
                return 0;
            }
            auto &pool = window_pools_[window_id];
            std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(pool.buffer_mutex));
            return static_cast<uint32_t>(pool.free_stack.top + 1);
        }

        uint32_t CommandBufferManager::GetTotalBuffers(uint32_t window_id) const noexcept
        {
            if (window_id >= window_count_)
            {
                return 0;
            }
            return BUFFERS_PER_WINDOW;
        }

        uint32_t CommandBufferManager::GetActiveBuffers(uint32_t window_id) const noexcept
        {
            if (window_id >= window_count_)
            {
                return 0;
            }
            auto &pool = window_pools_[window_id];
            std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(pool.buffer_mutex));
            return BUFFERS_PER_WINDOW - static_cast<uint32_t>(pool.free_stack.top + 1);
        }

    } // namespace rendering
} // namespace hud_3d