/**
 * @file memory_pool.h
 * @brief Memory Pool C++ Interface - Public Domain - 2025 3D HUD Project
 *
 * @author Yameng.He
 * @date 2025-12-20
 * @version 1.0
 *
 * @section overview Overview
 * This library provides a clean C++ interface for memory allocation,
 * hiding the underlying rpmalloc implementation details. It offers
 * RAII semantics, type safety, and simplified memory management.
 *
 * @section license License
 * This library is put in the public domain; you can redistribute it and/or
 * modify it without any restrictions.
 *
 * @section features Features
 * - RAII-based initialization and cleanup
 * - Type-safe allocation and deallocation
 * - Smart pointer integration
 * - Thread-local memory management
 * - Memory statistics and monitoring
 * - Aligned memory allocation support
 * - Array allocation with proper construction/destruction
 */

#pragma once

#include <memory>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <new>

// =============================================================================
// Memory Monitor Integration
// =============================================================================
// Memory monitoring is integrated directly into MemoryPool for transparent operation
// Use __3D_HUD_MEMORY_MONITOR__ macro to enable/disable monitoring at compile time

#ifdef __3D_HUD_MEMORY_MONITOR__
#include "utils/memory/memory_monitor.h"
#endif

namespace hud_3d
{
    namespace utils
    {

        // =============================================================================
        // Forward Declarations
        // =============================================================================

        struct MemoryPoolStatistics;

        // =============================================================================
        // Memory Pool Class
        // =============================================================================

        /**
         * @brief Clean C++ interface for memory allocation with integrated monitoring
         *
         * @class MemoryPool
         *
         * This class provides a comprehensive C++ wrapper around a memory allocation
         * system, offering RAII semantics, type safety, and simplified memory
         * management without exposing the underlying implementation details.
         *
         * @note All methods are static as this class manages a global memory pool.
         * @note Memory monitoring is integrated transparently when __3D_HUD_MEMORY_MONITOR__ is defined
         * @warning Ensure proper initialization before using allocation functions.
         * @see MemoryPoolInitializer for RAII-based initialization
         * @see ThreadMemoryPoolInitializer for thread-local initialization
         */
        class MemoryPool final
        {
        public:
            // =============================================================================
            // Initialization Functions
            // =============================================================================

            /**
             * @brief Initialize the global memory pool
             *
             * This function initializes the global memory pool with default settings.
             * It must be called before any memory allocation operations.
             *
             * @return true if initialization succeeded, false otherwise
             *
             * @note This function is thread-safe
             * @warning Do not call this function multiple times without calling finalize()
             * @see finalize()
             * @see initialize(bool, bool) for custom configuration
             */
            static bool Initialize();

            /**
             * @brief Initialize the memory pool with custom configuration
             *
             * This function initializes the global memory pool with custom settings.
             * It provides more control over memory pool behavior than the default
             * initialization.
             *
             * @param enable_huge_pages Enable huge pages support for better performance
             * @param disable_decommit Disable memory decommitting to reduce system calls
             * @param unmap_on_finalize Unmap all memory on finalize (useful for dynamic libraries)
             * @param disable_thp Disable Transparent Huge Pages on Linux/Android
             * @return true if initialization succeeded, false otherwise
             *
             * @note Huge pages can improve performance but require system configuration
             * @note Disabling decommit may increase memory usage but reduce overhead
             * @note Unmap on finalize is useful for dynamic libraries to avoid memory leaks
             * @note Disable THP is Linux/Android specific and can affect performance
             * @warning This function must be called before any allocation operations
             * @see Initialize() for default initialization
             */
            static bool Initialize(bool enable_huge_pages = false, bool disable_decommit = false,
                                   bool unmap_on_finalize = false, bool disable_thp = false);

            /**
             * @brief Finalize the memory pool
             */
            static void Finalize();

            /**
             * @brief Initialize thread-local memory pool
             */
            static void ThreadInitialize();

            /**
             * @brief Finalize thread-local memory pool
             */
            static void ThreadFinalize();

            /**
             * @brief Check if thread-local memory pool is initialized
             * @return true if initialized, false otherwise
             */
            static bool IsThreadInitialized();

            /**
             * @brief Perform deferred deallocations
             */
            static void ThreadCollect();

            // =============================================================================
            // Allocation Functions
            // =============================================================================

            /**
             * @brief Allocate raw memory
             * @param size Number of bytes to allocate
             * @return Pointer to allocated memory, or nullptr if allocation failed
             */
            static void *Allocate(uint64_t size);

            /**
             * @brief Allocate and zero-initialize memory
             * @param size Number of bytes to allocate
             * @return Pointer to allocated memory, or nullptr if allocation failed
             */
            static void *AllocateZero(uint64_t size);

            /**
             * @brief Allocate memory for an array
             * @param num Number of elements
             * @param size Size of each element
             * @return Pointer to allocated memory, or nullptr if allocation failed
             */
            static void *AllocateArray(uint64_t num, uint64_t size);

            /**
             * @brief Allocate aligned memory
             * @param alignment Alignment requirement (must be power of two)
             * @param size Number of bytes to allocate
             * @return Pointer to allocated memory, or nullptr if allocation failed
             */
            static void *AllocateAligned(uint64_t alignment, uint64_t size);

            /**
             * @brief Allocate and zero-initialize aligned memory
             * @param alignment Alignment requirement (must be power of two)
             * @param size Number of bytes to allocate
             * @return Pointer to allocated memory, or nullptr if allocation failed
             */
            static void *AllocateAlignedZero(uint64_t alignment, uint64_t size);

            /**
             * @brief Reallocate memory
             * @param ptr Pointer to previously allocated memory
             * @param size New size in bytes
             * @return Pointer to reallocated memory, or nullptr if reallocation failed
             */
            static void *Reallocate(void *ptr, uint64_t size);

            /**
             * @brief Free memory
             * @param ptr Pointer to memory to free
             */
            static void Free(void *ptr);

            /**
             * @brief Get usable size of allocated memory
             * @param ptr Pointer to allocated memory
             * @return Usable size in bytes
             */
            static uint64_t UsableSize(void *ptr);

            // =============================================================================
            // Type-Safe Allocation Functions
            // =============================================================================

            /**
             * @brief Allocate memory for a single object
             *
             * This template function allocates memory for a single object of type T.
             * The memory is allocated but the object is not constructed.
             *
             * @tparam T Type of object to allocate (must be default constructible)
             * @return Pointer to allocated memory, or nullptr if allocation failed
             *
             * @note The returned pointer points to uninitialized memory
             * @see create() for allocation with construction
             * @see destroy() for proper destruction and deallocation
             * @see allocate_array() for array allocation
             */
            template <typename T>
            static T *Allocate()
            {
                return static_cast<T *>(Allocate(sizeof(T)));
            }

            /**
             * @brief Allocate and construct a single object
             *
             * This template function allocates memory and constructs an object of type T
             * using perfect forwarding of constructor arguments.
             *
             * @tparam T Type of object to create
             * @tparam Args Argument types for constructor
             * @param args Arguments to forward to the constructor
             * @return Pointer to constructed object, or nullptr if allocation failed
             *
             * @note Uses perfect forwarding to preserve argument value categories
             * @note The object is properly constructed using placement new
             * @see destroy() for proper destruction and deallocation
             * @see allocate() for allocation without construction
             * @see create_array() for array construction
             */
            template <typename T, typename... Args>
            static T *Create(Args &&...args)
            {
                T *ptr = Allocate<T>();
                if (ptr)
                {
                    new (ptr) T(std::forward<Args>(args)...);
                }
                return ptr;
            }

            /**
             * @brief Destroy and free a single object
             * @tparam T Type of object
             * @param ptr Pointer to object
             */
            template <typename T>
            static void Destroy(T *ptr)
            {
                if (ptr)
                {
                    ptr->~T();
                    Free(ptr);
                }
            }

            /**
             * @brief Allocate memory for an array of objects
             * @tparam T Type of objects
             * @param count Number of objects
             * @return Pointer to allocated array, or nullptr if allocation failed
             */
            template <typename T>
            static T *AllocateArray(uint64_t count)
            {
                return static_cast<T *>(AllocateArray(count, sizeof(T)));
            }

            /**
             * @brief Allocate and construct an array of objects
             * @tparam T Type of objects
             * @param count Number of objects
             * @return Pointer to constructed array, or nullptr if allocation failed
             */
            template <typename T>
            static T *CreateArray(uint64_t count)
            {
                T *ptr = AllocateArray<T>(count);
                if (ptr)
                {
                    for (uint64_t i = 0; i < count; ++i)
                    {
                        new (&ptr[i]) T();
                    }
                }
                return ptr;
            }

            /**
             * @brief Destroy and free an array of objects
             * @tparam T Type of objects
             * @param ptr Pointer to array
             * @param count Number of objects
             */
            template <typename T>
            static void DestroyArray(T *ptr, uint64_t count)
            {
                if (ptr)
                {
                    for (uint64_t i = 0; i < count; ++i)
                    {
                        ptr[i].~T();
                    }
                    Free(ptr);
                }
            }

            // =============================================================================
            // Flexible Array Allocation Functions
            // =============================================================================

            /**
             * @brief Allocate memory for a flexible structure
             *
             * This template function allocates memory for a structure with a flexible array member.
             * It calculates the total size needed for the structure plus the flexible array data.
             *
             * @tparam T Type of the base structure (must have a flexible array member)
             * @tparam U Type of the flexible array elements
             * @param array_size Number of elements in the flexible array
             * @return Pointer to allocated memory, or nullptr if allocation failed
             *
             * @note The structure must have a flexible array member as the last field
             * @note The allocated memory includes space for the structure and the flexible array
             * @see CreateFlexibleStruct() for allocation with construction
             * @see DestroyFlexibleStruct() for proper destruction and deallocation
             */
            template <typename T, typename U>
            static T *AllocateFlexibleStruct(uint64_t array_size)
            {
                // Calculate total size: sizeof(T) + (array_size * sizeof(U))
                // Subtract sizeof(U[1]) because flexible array member is already counted in sizeof(T)
                uint64_t total_size = sizeof(T) - sizeof(U[1]) + (array_size * sizeof(U));
                return static_cast<T *>(Allocate(total_size));
            }

            /**
             * @brief Allocate and construct a flexible structure
             *
             * This template function allocates memory and constructs a structure with a flexible array member.
             * It uses perfect forwarding of constructor arguments for the base structure.
             *
             * @tparam T Type of the base structure
             * @tparam U Type of the flexible array elements
             * @tparam Args Argument types for base structure constructor
             * @param array_size Number of elements in the flexible array
             * @param args Arguments to forward to the base structure constructor
             * @return Pointer to constructed structure, or nullptr if allocation failed
             *
             * @note The flexible array elements are not initialized by this function
             * @note The structure must have a flexible array member as the last field
             */
            template <typename T, typename U, typename... Args>
            static T *CreateFlexibleStruct(uint64_t array_size, Args &&...args)
            {
                T *ptr = AllocateFlexibleStruct<T, U>(array_size);
                if (ptr)
                {
                    new (ptr) T(std::forward<Args>(args)...);
                }
                return ptr;
            }

            /**
             * @brief Destroy and free a flexible structure
             *
             * This template function properly destroys and deallocates a structure with a flexible array member.
             *
             * @tparam T Type of the base structure
             * @tparam U Type of the flexible array elements
             * @param ptr Pointer to the structure to destroy
             * @param array_size Number of elements in the flexible array (for size calculation)
             *
             * @note The flexible array elements are not destroyed by this function
             * @note The structure must have been allocated using AllocateFlexibleStruct or CreateFlexibleStruct
             */
            template <typename T, typename U>
            static void DestroyFlexibleStruct(T *ptr, uint64_t array_size)
            {
                if (ptr)
                {
                    ptr->~T();
                    // Calculate the same total size used for allocation
                    uint64_t total_size = sizeof(T) - sizeof(U[1]) + (array_size * sizeof(U));
                    Free(ptr);
                }
            }

            // =============================================================================
            // Statistics Functions
            // =============================================================================

            /**
             * @brief Get global statistics
             * @param stats Reference to statistics structure to fill
             */
            static void GetGlobalStatistics(MemoryPoolStatistics &stats);

            /**
             * @brief Get thread statistics
             * @param stats Reference to statistics structure to fill
             */
            static void GetThreadStatistics(MemoryPoolStatistics &stats);

            /**
             * @brief Dump statistics to file
             * @param file FILE* to dump statistics to
             */
            static void DumpStatistics(void *file);

#ifdef __3D_HUD_MEMORY_MONITOR__
        private:
            static MemoryMonitor monitor_;
#endif

        private:
            MemoryPool() = delete;
            ~MemoryPool() = delete;

        private:
            MemoryPool(MemoryPool &&) = delete;
            MemoryPool(const MemoryPool &) = delete;

        private:
            MemoryPool &operator=(MemoryPool &&) = delete;
            MemoryPool &operator=(const MemoryPool &) = delete;
        };

        // =============================================================================
        // Statistics Structure
        // =============================================================================

        /**
         * @brief Memory pool statistics structure
         *
         * @struct MemoryPoolStatistics
         *
         * This structure contains detailed statistics about the memory pool's
         * current state and performance metrics.
         */
        struct MemoryPoolStatistics
        {
            // Global statistics
            uint64_t mapped_memory;      //!< Current amount of mapped virtual memory (in bytes)
            uint64_t mapped_memory_peak; //!< Peak amount of mapped virtual memory (in bytes)
            uint64_t committed_memory;   //!< Current committed memory (in bytes)
            uint64_t active_memory;      //!< Current active memory (in bytes)
            uint64_t active_memory_peak; //!< Peak active memory (in bytes)
            uint64_t heap_count;         //!< Current heap count

            // Thread-specific statistics
            uint64_t thread_cache_size; //!< Current thread cache size (in bytes)
            uint64_t span_cache_size;   //!< Current span cache size (in bytes)
            uint64_t thread_to_global;  //!< Total bytes transitioned from thread to global cache
            uint64_t global_to_thread;  //!< Total bytes transitioned from global to thread cache

            /**
             * @brief Construct a new MemoryPoolStatistics object
             *
             * Initializes all statistics fields to zero.
             */
            MemoryPoolStatistics();
        };

        // =============================================================================
        // Smart Pointer Support
        // =============================================================================

        /**
         * @brief Custom deleter for std::unique_ptr using memory pool
         *
         * @tparam T Type of object to delete
         *
         * This deleter properly destroys objects and deallocates memory
         * using the memory pool, ensuring proper resource management.
         */
        template <typename T>
        struct MemoryPoolDeleter
        {
            /**
             * @brief Delete operator for memory pool objects
             *
             * @param ptr Pointer to object to delete
             *
             * @note Properly calls the destructor before deallocating memory
             * @note Handles nullptr safely
             */
            void operator()(T *ptr) const
            {
                if (ptr)
                {
                    ptr->~T();
                    MemoryPool::Free(ptr);
                }
            }
        };

        /**
         * @brief Custom deleter for arrays using memory pool
         */
        template <typename T>
        struct MemoryPoolArrayDeleter
        {
            uint64_t count;

            explicit MemoryPoolArrayDeleter(uint64_t n) : count(n) {}

            void operator()(T *ptr) const
            {
                if (ptr)
                {
                    MemoryPool::DestroyArray(ptr, count);
                }
            }
        };

        /**
         * @brief Create a unique_ptr using memory pool
         * @tparam T Type of object
         * @tparam Args Argument types
         * @param args Constructor arguments
         * @return Unique pointer to created object
         */
        /**
         * @brief Create a unique_ptr using memory pool
         * @tparam T Type of object
         * @tparam Args Argument types
         * @param args Constructor arguments
         * @return Unique pointer to created object
         */
        template <typename T, typename... Args>
        std::unique_ptr<T, MemoryPoolDeleter<T>> MakeUniqueMempool(Args &&...args)
        {
            T *ptr = MemoryPool::Create<T>(std::forward<Args>(args)...);
            return std::unique_ptr<T, MemoryPoolDeleter<T>>(ptr, MemoryPoolDeleter<T>{});
        }

        /**
         * @brief Create a unique_ptr for array using memory pool
         * @tparam T Type of objects
         * @param count Number of objects
         * @return Unique pointer to created array
         */
        template <typename T>
        std::unique_ptr<T[], MemoryPoolArrayDeleter<T>> MakeUniqueMempoolArray(uint64_t count)
        {
            T *ptr = MemoryPool::CreateArray<T>(count);
            return std::unique_ptr<T[], MemoryPoolArrayDeleter<T>>(ptr, MemoryPoolArrayDeleter<T>{count});
        }

        /**
         * @brief Create a shared_ptr using memory pool
         * @tparam T Type of object
         * @tparam Args Argument types
         * @param args Constructor arguments
         * @return Shared pointer to created object
         */
        template <typename T, typename... Args>
        std::shared_ptr<T> MakeSharedMempool(Args &&...args)
        {
            T *ptr = MemoryPool::Create<T>(std::forward<Args>(args)...);
            return std::shared_ptr<T>(ptr, MemoryPoolDeleter<T>{});
        }

        /**
         * @brief Create a shared_ptr for array using memory pool
         * @tparam T Type of objects
         * @param count Number of objects
         * @return Shared pointer to created array
         */
        template <typename T>
        std::shared_ptr<T[]> MakeSharedMempoolArray(uint64_t count)
        {
            T *ptr = MemoryPool::CreateArray<T>(count);
            return std::shared_ptr<T[]>(ptr, MemoryPoolArrayDeleter<T>{count});
        }

        // =============================================================================
        // STL Allocator Support
        // =============================================================================

        /**
         * @brief STL-compatible allocator using memory pool
         *
         * @tparam T Type of object to allocate
         *
         * This class provides a standard C++ allocator interface that uses the
         * memory pool for allocation and deallocation. It can be used with any
         * STL container that accepts a custom allocator.
         *
         * @note This allocator is stateless and thread-safe
         * @warning Ensure memory pool is initialized before using this allocator
         * @see std::vector, std::list, std::map, std::set
         */
        template <typename T>
        class MemoryPoolAllocator
        {
        public:
            // Standard allocator type definitions
            using value_type = T;                                          //!< Type of allocated elements
            using pointer = T *;                                           //!< Pointer to element type
            using const_pointer = const T *;                               //!< Const pointer to element type
            using reference = T &;                                         //!< Reference to element type
            using const_reference = const T &;                             //!< Const reference to element type
            using size_type = uint64_t;                                    //!< Size type
            using difference_type = std::ptrdiff_t;                        //!< Difference type
            using propagate_on_container_move_assignment = std::true_type; //!< Propagate on move assignment
            using is_always_equal = std::true_type;                        //!< Always equal comparison

            /**
             * @brief Default constructor
             */
            MemoryPoolAllocator() noexcept = default;

            /**
             * @brief Copy constructor from allocator of different type
             *
             * @tparam U Type of other allocator
             * @param other Other allocator instance
             *
             * @note Allows conversion between different allocator types
             */
            template <typename U>
            MemoryPoolAllocator(const MemoryPoolAllocator<U> &other) noexcept {}

            /**
             * @brief Allocate memory for n objects
             *
             * @param n Number of objects to allocate
             * @return Pointer to allocated memory
             *
             * @throws std::bad_alloc if allocation fails
             */
            T *allocate(size_t n)
            {
                if (n > max_size())
                {
                    throw std::bad_alloc();
                }

                void *ptr = MemoryPool::AllocateArray(n, sizeof(T));
                if (!ptr)
                {
                    throw std::bad_alloc();
                }

                return static_cast<T *>(ptr);
            }

            /**
             * @brief Deallocate memory
             *
             * @param p Pointer to memory to deallocate
             * @param n Number of objects (unused)
             */
            void deallocate(T *p, size_t n) noexcept
            {
                if (p)
                {
                    MemoryPool::Free(p);
                }
            }

            /**
             * @brief Construct an object in allocated memory
             *
             * @tparam U Type of object to construct
             * @tparam Args Argument types
             * @param p Pointer to memory
             * @param args Constructor arguments
             */
            template <typename U, typename... Args>
            void construct(U *p, Args &&...args)
            {
                ::new (static_cast<void *>(p)) U(std::forward<Args>(args)...);
            }

            /**
             * @brief Destroy an object
             *
             * @tparam U Type of object to destroy
             * @param p Pointer to object
             */
            template <typename U>
            void destroy(U *p)
            {
                p->~U();
            }

            /**
             * @brief Get maximum allocation size
             *
             * @return Maximum number of objects that can be allocated
             */
            size_t max_size() const noexcept
            {
                return std::numeric_limits<size_t>::max() / sizeof(T);
            }

            /**
             * @brief Equality comparison
             *
             * @tparam U Type of other allocator
             * @param other Other allocator
             * @return true (allocators are always equal)
             */
            template <typename U>
            bool operator==(const MemoryPoolAllocator<U> &other) const noexcept
            {
                return true;
            }

            /**
             * @brief Inequality comparison
             *
             * @tparam U Type of other allocator
             * @param other Other allocator
             * @return false (allocators are always equal)
             */
            template <typename U>
            bool operator!=(const MemoryPoolAllocator<U> &other) const noexcept
            {
                return false;
            }
        };

        // =============================================================================
        // RAII Wrapper Classes
        // =============================================================================

        /**
         * @brief RAII wrapper for memory pool initialization
         *
         * @class MemoryPoolInitializer
         *
         * This class provides RAII (Resource Acquisition Is Initialization)
         * semantics for memory pool initialization. The memory pool is
         * automatically initialized when the object is created and finalized
         * when the object is destroyed.
         *
         * @note Use this class to ensure proper cleanup even in case of exceptions
         * @warning Only one instance should exist at a time
         * @see ThreadMemoryPoolInitializer for thread-local initialization
         */
        class MemoryPoolInitializer
        {
        public:
            /**
             * @brief Construct a new MemoryPoolInitializer object
             *
             * Initializes the global memory pool with the specified configuration.
             *
             * @param enable_huge_pages Enable huge pages support
             * @param disable_decommit Disable memory decommitting
             * @param unmap_on_finalize Unmap all memory on finalize (useful for dynamic libraries)
             * @param disable_thp Disable Transparent Huge Pages on Linux/Android
             *
             * @throws std::runtime_error if memory pool initialization fails
             */
            explicit MemoryPoolInitializer(bool enable_huge_pages = false, bool disable_decommit = false,
                                           bool unmap_on_finalize = false, bool disable_thp = false);

            /**
             * @brief Destroy the MemoryPoolInitializer object
             *
             * Automatically finalizes the memory pool when the object is destroyed.
             */
            ~MemoryPoolInitializer();

            // Disable copying
            MemoryPoolInitializer(const MemoryPoolInitializer &) = delete;            //!< Copy constructor deleted
            MemoryPoolInitializer &operator=(const MemoryPoolInitializer &) = delete; //!< Copy assignment deleted
        };

        /**
         * @brief RAII wrapper for thread-local memory pool initialization
         */
        class ThreadMemoryPoolInitializer
        {
        public:
            ThreadMemoryPoolInitializer();
            ~ThreadMemoryPoolInitializer();

            // Disable copying
            ThreadMemoryPoolInitializer(const ThreadMemoryPoolInitializer &) = delete;
            ThreadMemoryPoolInitializer &operator=(const ThreadMemoryPoolInitializer &) = delete;
        };

    } // namespace utils
} // namespace hud_3d