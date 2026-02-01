/**
 * @file window_manager.h
 * @brief Window manager interface for creating and managing multiple windows
 *
 * @details
 * WindowManager provides a centralized interface for creating, managing, and
 * destroying multiple windows within the 3D HUD application. It maintains a
 * registry of windows and provides lifecycle management, event polling, and
 * state queries.
 *
 * @par Design Philosophy:
 * The WindowManager follows the singleton registry pattern, maintaining a
 * fixed-size array of window instances. It abstracts platform-specific window
 * creation details and provides a unified interface for window management.
 *
 * @par Key Responsibilities:
 * - Window creation and destruction
 * - Window registry management
 * - Global event polling
 * - Window state queries
 * - Resource cleanup
 *
 * @par Thread Safety:
 * This class is not thread-safe. All methods should be called from the
 * main application thread.
 *
 * @see IWindow for individual window interface
 * @see WindowDesc for window creation parameters
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#include <cstdint>
#include "platform/window.h"

namespace hud_3d
{
    namespace platform
    {
        /**
         * @brief Check if a WindowId is valid
         * @param id Window identifier to check
         * @return true if the ID is valid, false otherwise
         *
         * @details
         * Validates that a WindowId falls within the acceptable range.
         * This function is constexpr and can be used at compile-time.
         */
        [[nodiscard]] constexpr bool IsValidWindowId(const uint32_t id) noexcept
        {
            constexpr uint32_t MIN_VALID = 1;
            constexpr uint32_t MAX_VALID = 8; // Matches MAX_WINDOWS
            const uint32_t id_value = static_cast<uint32_t>(id);
            return id_value >= MIN_VALID && id_value <= MAX_VALID;
        }

        /**
         * @class WindowManager
         * @brief Manages multiple window instances for the application
         *
         * @details
         * WindowManager provides centralized management for multiple windows,
         * handling creation, destruction, event polling, and state queries.
         * It maintains a fixed-size registry of windows and ensures proper
         * resource cleanup.
         *
         * @par Singleton Pattern:
         * WindowManager follows the singleton pattern to ensure only one
         * instance exists throughout the application lifecycle. This prevents
         * conflicts in window management and resource allocation.
         *
         * @par Window Registry:
         * Windows are stored in a fixed-size array with a maximum capacity
         * defined by MAX_WINDOWS. Each window is assigned a unique ID upon
         * creation, which is used for subsequent operations.
         *
         * @par Platform Abstraction:
         * The CreateNewWindow method automatically creates the appropriate
         * platform-specific window implementation (Win32Window on Windows,
         * X11Window on Linux, etc.) based on the build configuration.
         *
         * @par Resource Management:
         * All windows are managed through unique_ptr for automatic lifetime
         * control. The WindowManager ensures proper cleanup during destruction.
         */
        class IWindowManager
        {
        public:
            /**
             * @name Singleton Instance Management
             * @brief Create and access the singleton instance
             */
            /**@{*/
            /**
             * @brief Creates and returns the singleton WindowManager instance
             * @return Reference to the singleton WindowManager instance
             *
             * @details
             * Implements the singleton pattern by creating the WindowManager
             * instance on first call and returning the same instance on
             * subsequent calls. This ensures only one WindowManager exists
             * throughout the application lifecycle.
             *
             * @note Thread safety: This method is not thread-safe. It should
             *       be called from the main thread during application initialization.
             * @note The instance is automatically destroyed when the application exits
             */
            [[nodiscard]] static IWindowManager &GetInstance() noexcept;
            /**@}*/

        public:
            /**
             * @name Constructor and Destructor
             * @brief Default construction and destruction
             */
            /**@{*/
            IWindowManager() = default;
            virtual ~IWindowManager() = default;
            /**@}*/

        public:
            /**
             * @name Window Lifecycle Management
             * @brief Create and destroy windows
             */
            /**@{*/

            /**
             * @brief Creates a new window with specified parameters
             * @param[in] desc Window descriptor containing creation parameters
             * @return Result containing window ID on success, error code on failure
             *
             * @details
             * Creates a platform-specific window implementation and adds it to the
             * internal registry. The returned window ID can be used to retrieve
             * or manipulate the window.
             *
             * @par Platform Selection:
             * Automatically creates the appropriate window implementation based on
             * the build platform:
             * - Windows: Win32Window
             * - Linux: X11Window (planned)
             * - Android: AndroidWindow (planned)
             *
             * @par Error Conditions:
             * - Returns error if MAX_WINDOWS limit reached
             * - Returns error if window initialization fails
             * - Returns error if descriptor contains invalid parameters
             *
             * @note Window IDs are not reused after window destruction
             * @note The first created window typically becomes the primary window
             */
            [[nodiscard]] virtual uint32_t CreateNewWindow(const WindowDesc &desc) noexcept = 0;

            /**
             * @brief Destroys a window and releases its resources
             * @param[in] window_id ID of the window to destroy
             *
             * @details
             * Shuts down the specified window and removes it from the registry.
             * All associated resources (graphics context, native window, etc.)
             * are properly released.
             *
             * @warning The window_id must be valid. Using an invalid ID may cause
             * undefined behavior. Always check the return value of CreateNewWindow().
             *
             * @note Window IDs are not reused after destruction
             * @note Automatically called for all remaining windows during WindowManager destruction
             */
            virtual void DestroyWindow(const uint32_t window_id) noexcept = 0;
            /**@}*/

        public:
            /**
             * @name Window Access
             * @brief Retrieve window instances
             */
            /**@{*/
            /**
             * @brief Retrieves a window by its ID (mutable)
             * @param[in] window_id ID of the window to retrieve
             * @return Pointer to the window, or nullptr if not found
             *
             * @details
             * Provides mutable access to a window for operations that may modify
             * its state. The returned pointer remains valid until the window is
             * destroyed.
             *
             * @note Always check the return value for nullptr before use
             * @note The pointer is non-owning; WindowManager maintains ownership
             */
            [[nodiscard]] virtual IWindow *GetWindow(const uint32_t window_id) noexcept = 0;

            /**
             * @brief Retrieves a window by its ID (const)
             * @param[in] window_id ID of the window to retrieve
             * @return Const pointer to the window, or nullptr if not found
             *
             * @details
             * Provides const access to a window for inspection without modification.
             * Useful for checking window properties or state.
             *
             * @note Always check the return value for nullptr before use
             * @note The pointer is non-owning; WindowManager maintains ownership
             */
            [[nodiscard]] virtual const IWindow *GetWindow(const uint32_t window_id) const noexcept = 0;
            /**@}*/

        public:
            /**
             * @name Event Processing
             * @brief Process window events
             */
            /**@{*/

            /**
             * @brief Polls events for all managed windows
             *
             * @details
             * Processes pending events for all active windows in the registry.
             * This should be called once per frame in the main application loop
             * to keep all windows responsive to user input and system events.
             *
             * @par Implementation:
             * Delegates to each window's PollEvents() method in sequence.
             *
             * @note Failure to call this method regularly may cause windows to
             * become unresponsive or miss important system events
             * @note This method does not block; it processes all pending events and returns
             */
            virtual void PollEvents() noexcept = 0;
            /**@}*/

        public:
            /**
             * @name State Queries
             * @brief Query window manager state
             */
            /**@{*/
            /**
             * @brief Gets the number of active windows
             * @return Current number of managed windows
             *
             * @details
             * Returns the count of windows currently in the registry.
             * This includes all windows that have been created but not yet destroyed.
             */
            [[nodiscard]] virtual uint32_t GetWindowCount() const noexcept = 0;

            /**
             * @brief Checks if any window requests application exit
             * @return true if any window should close, false otherwise
             *
             * @details
             * Checks the close state of all managed windows. Returns true if
             * any window has received a close event (e.g., user clicked close button).
             * This method should be called in the main loop to determine when to exit.
             *
             * @par Typical Usage:
             * @code
             * while (!window_manager.ShouldClose()) {
             *     window_manager.PollEvents();
             *     // ... render frame ...
             * }
             * @endcode
             *
             * @note Returns false if no windows are managed
             */
            [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;
            /**@}*/

        private:
            /**
             * @name Copy and Move Control
             * @brief Prevent copying, prevent moving
             *
             * @details
             * Copy operations are deleted to prevent resource management issues
             * with multiple WindowManager instances. Move operations are defaulted
             * to support container storage and return value optimization.
             */
            /**@{*/
            IWindowManager(const IWindowManager &) = delete;
            IWindowManager &operator=(const IWindowManager &) = delete;
            IWindowManager(IWindowManager &&) = delete;
            IWindowManager &operator=(IWindowManager &&) = delete;
            /**@}*/
        };

    } // namespace platform
} // namespace hud_3d
