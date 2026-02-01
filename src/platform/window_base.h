/**
 * @file window_base.h
 * @brief Platform-agnostic window base class implementation
 *
 * @details
 * WindowBase provides a platform-agnostic implementation of common IWindow
 * functionality. It handles state management, configuration storage, and
 * provides default implementations for methods that don't require platform-specific code.
 *
 * @par Design Philosophy:
 * This class implements the Template Method pattern, where platform-specific
 * implementations inherit from WindowBase and override the pure virtual methods
 * that require platform-specific code (Initialize, Shutdown, PollEvents, etc.).
 *
 * @par Platform-Specific Implementation:
 * Derived classes (Win32Window, X11Window, etc.) must implement:
 * - Initialize() - Platform window creation
 * - Shutdown() - Platform resource cleanup
 * - PollEvents() - Platform event processing
 * - SwapBuffers() - Platform buffer swapping
 * - Resize() - Platform window resizing
 * - GetNativeHandle() - Platform handle access
 * - AddView/RemoveView - Platform rendering setup
 *
 * @see Win32Window for Windows-specific implementation example
 * @see IWindow for the complete interface definition
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include "rendering/view.h"
#include "platform/window.h"

namespace hud_3d
{
    namespace platform
    {
        class IGraphicsContext;

        /**
         * @class WindowBase
         * @brief Platform-agnostic window base class
         *
         * @details
         * WindowBase provides default implementations for IWindow methods that
         * are platform-agnostic, such as state management, configuration storage,
         * and view management. Platform-specific functionality is deferred to
         * pure virtual methods that must be implemented by derived classes.
         *
         * @par State Management:
         * - Initialization status tracking
         * - Window descriptor storage
         * - Graphics configuration storage
         * - Close request flag management
         *
         * @par View Management:
         * - View container management
         * - View ID assignment and tracking
         * - View lifecycle management
         *
         * @par Thread Safety:
         * This class is not thread-safe. All methods should be called from the
         * main application thread unless otherwise documented.
         */
        class WindowBase : public IWindow
        {
        public:
            /**
             * @brief Default constructor
             * @details
             * Constructs a WindowBase in uninitialized state.
             * The window must be initialized with a valid WindowDesc before use.
             */
            WindowBase() = default;

            /**
             * @brief Virtual destructor
             * @details
             * Automatically shuts down the window if it is still initialized.
             * Derived class destructors should call Shutdown() to ensure proper cleanup.
             */
            virtual ~WindowBase() = default;

        public:
            // Platform-agnostic implementations
            /**
             * @brief Check if window should close
             * @return true if window close was requested, false otherwise
             *
             * @details
             * Returns the current close request state. This is typically set by
             * platform-specific implementations when the user requests window closure.
             *
             * @note This method is thread-safe and can be called from any thread
             */
            [[nodiscard]] virtual bool ShouldClose() const noexcept override;

            /**
             * @brief Begin a new frame
             * @details
             * Default implementation does nothing. Derived classes can override to
             * implement custom per-frame initialization logic.
             */
            virtual void BeginFrame() noexcept override;

            /**
             * @brief End the current frame
             * @details
             * Default implementation does nothing. Derived classes can override to
             * implement custom per-frame cleanup logic.
             */
            virtual void EndFrame() noexcept override;

            /**
             * @brief Get the window's unique identifier
             * @return Window ID assigned by the WindowManager
             *
             * @details
             * Returns the ID that uniquely identifies this window within the
             * WindowManager's window registry.
             */
            [[nodiscard]] virtual uint32_t GetWindowId() const noexcept override;

            /**
             * @brief Set the window's unique identifier
             * @param id Window ID to assign
             *
             * @details
             * Assigns a unique identifier to the window. This is typically called
             * by the WindowManager during window creation.
             *
             * @warning This should not be called by user code directly
             */
            virtual void SetWindowId(const uint32_t id) noexcept override;

            /**
             * @brief Adds a new view to the window
             * @param[in] desc View descriptor containing viewport and projection settings
             * @return View ID (index) of the newly created view, or 0xFFFFFFFF on failure
             *
             * @details
             * Creates a new View object using the provided descriptor and adds it to
             * the window's internal view list. The returned ID can be used to retrieve
             * or remove the view later. Views are stored in a vector and automatically
             * managed via unique_ptr for proper lifetime control.
             *
             * @par View ID Assignment:
             * View IDs are assigned sequentially starting from 0. The ID corresponds to
             * the index in the internal views_ vector.
             *
             * @par Error Handling:
             * Returns 0xFFFFFFFF if view creation fails (e.g., invalid descriptor).
             * The window's state remains unchanged on failure.
             *
             * @note The first view added typically becomes the primary rendering view
             * @note Views can be added and removed dynamically at runtime
             */
            [[nodiscard]] virtual uint32_t AddView(std::unique_ptr<rendering::IView> view) noexcept override;

            /**
             * @brief Removes a view from the window
             * @param[in] view_id ID of the view to remove
             *
             * @details
             * Destroys the specified view and removes it from the window's view list.
             * After this call, the view_id becomes invalid and should not be used.
             * The view is automatically deleted via unique_ptr management.
             *
             * @par ID Invalidation:
             * Removing a view may change the IDs of subsequent views in the vector.
             * Always retrieve fresh IDs after removing views.
             *
             * @warning The view_id must be valid (less than views_.size()).
             * Using an invalid ID will trigger an assertion in debug builds.
             * @note Does nothing if view_id is out of range
             */
            virtual void RemoveView(const uint32_t view_id) noexcept override;

            /**
             * @brief Retrieves a view by its ID
             * @param[in] view_id ID of the view to retrieve
             * @return Pointer to the View object, or nullptr if not found
             *
             * @details
             * Provides access to a view for configuration or rendering. The returned
             * pointer remains valid until the view is removed from the window.
             *
             * @par Usage Pattern:
             * @code
             * auto* view = window->GetView(view_id);
             * if (view) {
             *     view->SetCameraPosition(position);
             *     auto matrix = view->GetViewProjectionMatrix();
             * }
             * @endcode
             *
             * @note Always check the return value for nullptr before using
             * @note The pointer is non-owning; the WindowBase maintains ownership
             */
            [[nodiscard]] virtual rendering::IView *GetView(const uint32_t view_id) noexcept override;

            /**
             * @brief Gets all views in the window
             * @return Const reference to the vector of view unique pointers
             *
             * @details
             * Provides read-only access to all views for iteration or inspection.
             * This is useful for rendering all views or checking view counts.
             *
             * @par Performance:
             * Returns a const reference to avoid copying the vector. The reference
             * remains valid for the lifetime of the Window object.
             *
             * @par Iteration Example:
             * @code
             * for (const auto& view : window->GetViews()) {
             *     if (view) {
             *         RenderView(view.get());
             *     }
             * }
             * @endcode
             */
            [[nodiscard]] virtual const std::vector<std::unique_ptr<rendering::IView>> &GetViews() const noexcept override;

            /**
             * @brief Check if this is an externally-managed window
             * @return true if window was created externally, false if engine-created
             *
             * @details
             * Returns whether the window was created by the application (external)
             * or by the engine. For external windows, the application is responsible
             * for event handling and window lifecycle.
             */
            [[nodiscard]] virtual bool IsExternalWindow() const noexcept override;

            /**
             * @brief Check if window events should be processed
             * @return true if PollEvents should process events, false otherwise
             *
             * @details
             * For externally-created windows, the application typically handles
             * event processing. This method returns false for external windows to
             * avoid duplicate event processing.
             *
             * @note External windows should set external_window = true in WindowDesc
             * @see WindowDesc::external_window
             */
            [[nodiscard]] virtual bool ShouldProcessEvents() const noexcept override;

        protected:
            /**
             * @brief Set the close request flag
             * @param should_close true to request window closure, false to clear
             *
             * @details
             * Platform-specific implementations should call this method when
             * a close event is received (e.g., user clicks close button, Alt+F4).
             * This method is thread-safe.
             */
            void SetShouldClose(bool should_close) noexcept;

            /**
             * @brief Set the window descriptor
             * @param desc WindowDesc to store
             *
             * @details
             * Stores a WindowDesc for use during initialization and future reference.
             * This is called by platform-specific Initialize() implementations.
             */
            void SetWindowDesc(const WindowDesc &desc) noexcept;

            /**
             * @brief Get the stored window descriptor
             * @return const reference to the WindowDesc
             *
             * @details
             * Returns the WindowDesc provided to Initialize(). This can be used
             * by derived classes to access window configuration parameters.
             */
            [[nodiscard]] const WindowDesc &GetWindowDesc() const noexcept;

            /**
             * @brief Get the stored graphics configuration
             * @return const reference to the GraphicsConfig
             *
             * @details
             * Returns the GraphicsConfig (custom or default) used for context creation.
             * This can be used by derived classes to access graphics configuration.
             */
            [[nodiscard]] const GraphicsConfig &GetGraphicsConfig() const noexcept;

            /**
             * @brief Set the graphics configuration
             * @param config GraphicsConfig to store
             *
             * @details
             * Stores a GraphicsConfig for use during context creation.
             * This is typically called by SetGraphicsConfig() or CreateDefaultGraphicsConfig().
             */
            void SetGraphicsConfig(const GraphicsConfig &config) noexcept;

        protected:
            std::atomic<bool> should_close_{false};                 ///< Window close request flag (thread-safe)
            uint32_t window_id_{0xFFFFFFFF};                        ///< Window ID (0xFFFFFFFF = invalid)
            WindowDesc window_desc_{};                              ///< Stored window descriptor
            GraphicsConfig graphics_config_{};                      ///< Stored graphics configuration
            std::vector<std::unique_ptr<rendering::IView>> views_{}; ///< View container for multi-viewport rendering
        };
    }
}