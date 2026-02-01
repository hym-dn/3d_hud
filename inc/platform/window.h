/** @file window.h
 * @brief Window interface and descriptor definitions for the 3D HUD platform layer
 *
 * @author 3D HUD Development Team
 * @date 2025-01-16
 *
 * This file defines the IWindow interface class and WindowDesc structure for the
 * platform abstraction layer. The window system provides cross-platform window
 * creation, management, and event handling capabilities for the 3D HUD rendering engine.
 * It serves as the bridge between platform-specific window implementations (Win32, X11, etc.)
 * and the core rendering engine.
 *
 * @par Architecture Overview:
 * The window system follows a polymorphic interface pattern where platform-specific
 * implementations (e.g., Win32Window, X11Window) inherit from IWindow and provide
 * concrete implementations of window creation, event handling, and buffer management.
 * This design enables the rendering engine to operate agnostically of the underlying
 * platform.
 */

#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "platform/graphics_context.h"

namespace hud_3d
{
    // Forward declarations to avoid circular dependencies
    namespace rendering
    {
        class IView;
        struct ViewDesc;
    }

    namespace platform
    {
        /**
         * @struct WindowDesc
         * @brief Window descriptor structure containing creation parameters
         *
         * WindowDesc encapsulates all parameters required to create a window,
         * including dimensions, title, and platform-specific behavior flags.
         * This structure is passed to IWindow::Initialize() and provides a clean,
         * type-safe way to configure window creation without relying on platform-specific APIs.
         *
         * @par Default Configuration:
         * Provides sensible defaults for a 1280x720 window with VSYNC enabled,
         * resizable borders, and windowed mode. These defaults can be overridden
         * during initialization for custom window configurations.
         *
         * @par Platform-Specific Notes:
         * The native_window field is used for platform integration (e.g., passing
         * ANativeWindow* on Android). On most platforms, this should remain nullptr.
         */
        struct WindowDesc
        {
            uint32_t width = 1280;               ///< Window width in pixels
            uint32_t height = 720;               ///< Window height in pixels
            const char *title = "3D HUD";        ///< Window title string (UTF-8)
            ContextAPI api = ContextAPI::OpenGL; ///< Graphics API to use for rendering
            bool enable_vsync = true;            ///< Enable vertical synchronization
            bool fullscreen = false;             ///< Start in fullscreen mode
            bool resizable = true;               ///< Allow window resizing
            void *native_window = nullptr;       ///< Platform-specific native window handle (e.g., HWND, Window, ANativeWindow*)
            bool external_window = false;        ///< true if window is externally created, false if engine should create it
        };

        /**
         * @class IWindow
         * @brief Abstract window interface for cross-platform window management
         *
         * IWindow defines a polymorphic interface for window objects that encapsulate
         * platform-specific window creation, event handling, and buffer management.
         * Concrete implementations (Win32Window, X11Window, etc.) inherit from this
         * interface and provide platform-specific functionality while maintaining
         * a consistent API for the rendering engine.
         *
         * @par Key Responsibilities:
         * - Window creation and lifecycle management
         * - Event polling and dispatching
         * - Buffer swapping and synchronization
         * - View management for multi-viewport rendering
         * - Graphics context association
         *
         * @par Design Patterns:
         * This interface exemplifies the Strategy pattern, allowing the rendering
         * engine to use windows interchangeably without platform-specific code.
         * It also incorporates elements of the Factory pattern through the
         * WindowManager's creation methods.
         *
         * @par Thread Safety:
         * This interface is not thread-safe. All methods should be called from
         * the main application thread. Platform implementations may use internal
         * synchronization for window system interactions, but this is not guaranteed.
         *
         * @see WindowManager for window creation and management
         * @see IGraphicsContext for graphics context integration
         */
        class IWindow
        {
        public:
            /**
             * @name Constructor and Destructor
             * @brief Default construction and virtual destruction
             *
             * Default constructor and virtual destructor are explicitly defaulted.
             * Concrete implementations must provide their own initialization logic
             * in Initialize() rather than in constructors.
             */
            /**@{*/
            IWindow() = default;
            virtual ~IWindow() = default;
            /**@}*/

        public:
            /**
             * @name Lifecycle Management
             * @brief Window creation, destruction, and state management
             */
            /**@{*/

            /**
             * @brief Initializes the window with specified parameters
             * @param[in] desc Window descriptor containing dimensions, title, and flags
             * @return true if initialization succeeded, false otherwise
             *
             * Creates the actual platform window using the parameters from the descriptor.
             * This method must be called after construction and before any other operations.
             * On failure, the window remains in an uninitialized state and subsequent
             * method calls may produce undefined behavior.
             *
             * @par Platform-Specific Behavior:
             * - Windows: Creates a Win32 window with WS_OVERLAPPEDWINDOW style
             * - Linux: Creates an X11 window with appropriate event masks
             * - Android: Uses the native_window handle from the descriptor
             * - QNX: Creates a screen window using the Screen API
             *
             * @note This method may trigger platform-specific initialization such as
             * window class registration, event loop setup, or graphics context creation.
             */
            [[nodiscard]] virtual bool Initialize(const WindowDesc &desc) noexcept = 0;

            /**
             * @brief Shuts down and destroys the window
             *
             * Releases all platform-specific resources associated with the window,
             * including the native window handle, event callbacks, and graphics context.
             * After this call, the window object should not be used unless re-initialized.
             *
             * @note This method is typically called during application shutdown or
             * when dynamically destroying windows. It should gracefully handle
             * partially initialized states.
             */
            virtual void Shutdown() noexcept = 0;

            /**
             * @brief Checks if the window should close
             * @return true if a close event was received, false otherwise
             *
             * Polls the window's close state, typically set when the user clicks
             * the close button or presses Alt+F4. This method should be called
             * within the application main loop to determine when to exit.
             *
             * @par Implementation Note:
             * This method typically checks a flag set by the window procedure
             * or event handler when a close event is received.
             */
            [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;

            /**
             * @brief Retrieves the window's unique identifier
             * @return Window ID assigned by the WindowManager
             *
             * Returns the ID that uniquely identifies this window within the
             * WindowManager's window registry. This ID is used for lookup and
             * management operations.
             *
             * @note The ID is typically set by the WindowManager during creation
             * and remains constant for the window's lifetime.
             */
            virtual uint32_t GetWindowId() const noexcept = 0;

            /**
             * @brief Sets the window's unique identifier
             * @param[in] id Window ID to assign
             *
             * Assigns a unique identifier to the window. This is typically called
             * by the WindowManager during window creation and should not be called
             * directly by user code.
             *
             * @warning Changing the ID after creation may break WindowManager's
             * internal registry and should be avoided.
             */
            virtual void SetWindowId(const uint32_t id) noexcept = 0;

            /**
             * @brief Gets the native platform window handle
             * @return Platform-specific window handle (e.g., HWND, Window, ANativeWindow*)
             *
             * Returns the underlying native window handle for platform-specific
             * operations. The return type is void* to maintain platform independence,
             * but it should be cast to the appropriate type for the current platform.
             *
             * @par Platform Types:
             * - Windows: cast to HWND
             * - Linux: cast to Window (X11) or xcb_window_t
             * - Android: cast to ANativeWindow*
             * - QNX: cast to screen_window_t
             */
            virtual void *GetNativeHandle() const noexcept = 0;

            /**@}*/

            /**
             * @name Frame Management
             * @brief Control rendering frame lifecycle
             *
             * These methods manage the per-frame rendering cycle, including
             * event polling, buffer management, and presentation.
             */
            /**@{*/

            /**
             * @brief Polls and processes window events
             *
             * Processes all pending window events including keyboard input,
             * mouse movement, window resizing, and system events. This method
             * should be called once per frame before rendering.
             *
             * @par Implementation Details:
             * - Windows: Dispatches messages from the Win32 message queue
             * - Linux: Processes X11 events or xcb events
             * - Android: Handles input events from the Java layer
             * - QNX: Processes screen events
             *
             * @note Failure to call this method regularly may cause the window
             * to become unresponsive or miss important system events.
             */
            virtual void PollEvents() noexcept = 0;

            /**
             * @brief Swaps front and back buffers
             *
             * Presents the rendered frame by swapping the front (displayed) and
             * back (rendering) buffers. This method should be called after all
             * rendering is complete for the current frame.
             *
             * @par Synchronization:
             * If enable_vsync is true, this method will block until the vertical
             * blanking interval to prevent tearing. Otherwise, it swaps immediately.
             *
             * @note This method must be called within a valid OpenGL or graphics context.
             * Calling it without an active context will produce undefined behavior.
             */
            virtual void SwapBuffers() noexcept = 0;

            /**
             * @brief Called at the beginning of each frame
             *
             * Frame start notification hook. Override in derived classes to implement
             * custom per-frame initialization logic such as input processing,
             * timing updates, or state resets.
             *
             * @par Typical Usage:
             * - Reset per-frame counters
             * - Update frame timing
             * - Process accumulated input
             * - Clear render targets if needed
             */
            virtual void BeginFrame() noexcept = 0;

            /**
             * @brief Called at the end of each frame
             *
             * Frame end notification hook. Override in derived classes to implement
             * custom per-frame cleanup logic such as performance profiling,
             * state caching, or resource management.
             *
             * @par Typical Usage:
             * - Update FPS counters
             * - Log frame metrics
             * - Cache frequently accessed data
             * - Trigger garbage collection if needed
             */
            virtual void EndFrame() noexcept = 0;

            /**@}*/

            /**
             * @name Window Management
             * @brief Control window size and state
             */
            /**@{*/

            /**
             * @brief Resizes the window to specified dimensions
             * @param[in] width New window width in pixels
             * @param[in] height New window height in pixels
             *
             * Changes the window's dimensions and triggers a resize event.
             * This method updates the internal dimensions and may cause the
             * graphics context to be resized as well.
             *
             * @par Implementation Note:
             * This method typically calls the platform-specific resize API
             * (e.g., SetWindowPos on Windows, XResizeWindow on X11) and then
             * triggers a WM_SIZE or ConfigureNotify event.
             *
             * @note The actual window size may differ from the requested size
             * due to platform constraints or user preferences (e.g., minimum/maximum size limits).
             */
            virtual void Resize(const uint32_t width, const uint32_t height) noexcept = 0;

            /**@}*/

            /**
             * @name View Management
             * @brief Manage multiple rendering views within the window
             *
             * These methods provide functionality for managing multiple views
             * within a single window, enabling split-screen rendering, picture-in-picture,
             * or multi-viewport configurations.
             */
            /**@{*/

            /**
             * @brief Adds a new view to the window
             * @param[in] view Unique pointer to the view object created by rendering layer
             * @return View ID (index) of the newly added view, or 0xFFFFFFFF on failure
             *
             * Adds an existing View object created by the rendering layer to the window's
             * view list. The returned ID can be used to retrieve or remove the view later.
             *
             * @par View Management:
             * Views are stored in a vector and managed by unique_ptr for automatic
             * lifetime control. The WindowManager typically maintains ownership.
             *
             * @return uint32_t View ID for subsequent operations, or 0xFFFFFFFF if addition failed
             */
            virtual uint32_t AddView(std::unique_ptr<rendering::IView> view) noexcept = 0;

            /**
             * @brief Removes a view from the window
             * @param[in] view_id ID of the view to remove
             *
             * Destroys the specified view and removes it from the window's view list.
             * After this call, the view_id becomes invalid and should not be used.
             *
             * @warning The view_id must be valid. Using an invalid ID may cause undefined
             * behavior or crashes. Always check the return value of GetView() before
             * calling this method.
             */
            virtual void RemoveView(const uint32_t view_id) noexcept = 0;

            /**
             * @brief Retrieves a view by its ID
             * @param[in] view_id ID of the view to retrieve
             * @return Pointer to the View object, or nullptr if not found
             *
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
             */
            virtual rendering::IView *GetView(const uint32_t view_id) noexcept = 0;

            /**
             * @brief Gets all views in the window
             * @return Const reference to the vector of view unique pointers
             *
             * Provides read-only access to all views for iteration or inspection.
             * This is useful for rendering all views or checking view counts.
             *
             * @par Performance:
             * Returns a const reference to avoid copying the vector. The reference
             * remains valid for the lifetime of the Window object.
             *
             * @code
             * for (const auto& view : window->GetViews()) {
             *     if (view) {
             *         RenderView(view.get());
             *     }
             * }
             * @endcode
             */
            virtual const std::vector<std::unique_ptr<rendering::IView>> &GetViews() const noexcept = 0;

            /**@}*/

            /**
             * @name Graphics Context Integration
             * @brief Access the window's graphics context
             *
             * These methods provide access to the IGraphicsContext associated
             * with the window, enabling OpenGL or other graphics API operations.
             */
            /**@{*/

            /**
             * @brief Gets the window's graphics context (mutable)
             * @return Pointer to the IGraphicsContext, or nullptr if not initialized
             *
             * Provides mutable access to the graphics context for operations
             * that may modify context state (e.g., making the context current).
             * The context is created during window initialization.
             *
             * @note The context pointer remains valid for the window's lifetime
             * but may become invalid after Shutdown() is called.
             */
            [[nodiscard]] virtual IGraphicsContext *GetGraphicsContext() noexcept = 0;

            /**
             * @brief Gets the window's graphics context (const)
             * @return Const pointer to the IGraphicsContext, or nullptr if not initialized
             *
             * Provides const access to the graphics context for inspection
             * without modification. Useful for checking context validity or
             * querying context properties.
             */
            virtual const IGraphicsContext *GetGraphicsContext() const noexcept = 0;
            /**@}*/

            /**
             * @name External Window Support
             * @brief Support for externally created windows
             *
             * These methods enable integration with external window systems like Qt, ImGui, etc.
             * When using external windows, the application is responsible for window creation
             * and event handling, while the engine handles rendering context management.
             */
            /**@{*/

            /**
             * @brief Check if this is an externally-managed window
             * @return true if window was created externally, false if engine-created
             *
             * @details
             * Returns whether the window was created by the application (external)
             * or by the engine. For external windows, the application is responsible
             * for event handling and window lifecycle.
             */
            [[nodiscard]] virtual bool IsExternalWindow() const noexcept = 0;

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
            [[nodiscard]] virtual bool ShouldProcessEvents() const noexcept = 0;

            /**@}*/

        private:
            /**
             * @name Copy and Move Control
             * @brief Prevent object slicing and ensure polymorphic behavior
             *
             * All copy and move operations are explicitly deleted to prevent
             * object slicing and maintain proper polymorphic behavior. Window
             * objects should be managed through smart pointers rather than by value.
             *
             * @note Following C++ Core Guidelines for polymorphic base classes.
             * "A polymorphic class should suppress copying" - C++ Core Guidelines
             */
            /**@{*/
            IWindow(IWindow &&) = delete;
            IWindow(const IWindow &) = delete;
            IWindow &operator=(IWindow &&) = delete;
            IWindow &operator=(const IWindow &) = delete;
            /**@}*/
        };

    } // namespace platform
} // namespace hud3d