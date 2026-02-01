/**
 * @file win32_window.h
 * @brief Windows-specific window implementation header
 *
 * @details
 * Declares the Win32Window class, which provides a Windows-specific implementation
 * of the IWindow interface using the native Win32 API. This header contains the
 * complete interface for creating and managing windows on Windows platforms,
 * including window creation, event handling, and graphics context association.
 *
 * @par Key Features:
 * - Native Win32 window implementation
 * - Multiple graphics API support (OpenGL, Vulkan, Direct3D)
 * - Event processing and window management
 * - Integration with platform graphics contexts
 *
 * @par Platform Notes:
 * This header is Windows-specific and should only be included in Windows builds.
 * Other platforms (Linux, Android, QNX) have their own platform-specific implementations.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#include <Windows.h>
#include <memory>
#include "../window_base.h"
#include "platform/graphics_context.h"

namespace hud_3d
{
    namespace platform
    {
        /**
         * @class Win32Window
         * @brief Windows-specific window implementation using Win32 API
         *
         * @details
         * Win32Window provides a Windows-specific implementation of window functionality,
         * managing window creation, event handling, and graphics context association.
         * It inherits from WindowBase for platform-agnostic functionality and implements
         * platform-specific virtual methods using the native Win32 API.
         *
         * @par Inheritance Hierarchy:
         * - IWindow (abstract interface)
         *   - WindowBase (platform-agnostic base)
         *     - Win32Window (Windows-specific implementation)
         *
         * @par Graphics API Support:
         * The window can work with different graphics APIs (OpenGL, Vulkan, Direct3D)
         * by creating the appropriate context implementation based on WindowDesc::api.
         * This allows the same window code to support multiple rendering backends.
         *
         * @par Configuration:
         * Supports both simple initialization with WindowDesc (using sensible defaults)
         * and advanced configuration through SetGraphicsConfig() for fine-grained control.
         *
         * @par Thread Safety:
         * This class is not thread-safe. All methods must be called from the same thread
         * that created the window, typically the main application thread.
         *
         * @par Platform Requirements:
         * Requires Windows Vista or later for full functionality. Some features may require
         * specific Windows versions or graphics driver support.
         *
         * @see WindowBase for platform-agnostic functionality
         * @see IWindow for the complete interface definition
         */
        class Win32Window : public WindowBase
        {
        public:
            /**
             * @brief Default constructor for Win32Window
             * @details
             * Constructs a Win32Window instance in uninitialized state.
             * The window must be initialized with a valid WindowDesc before use.
             * Automatically retrieves the application instance handle (HINSTANCE).
             *
             * @throws std::runtime_error if HINSTANCE cannot be obtained
             */
            Win32Window() noexcept;

            /**
             * @brief Destructor for Win32Window
             * @details
             * Automatically shuts down the window if it is still initialized.
             * Releases all Win32 resources including the window handle, device context,
             * and graphics context.
             *
             * @note Safe to call even if window was never initialized
             * @note Automatically calls Shutdown() if needed
             */
            virtual ~Win32Window() noexcept override;

        public:
            /**
             * @brief Initialize the Win32 window
             * @param desc Window descriptor containing creation parameters
             * @return true if initialization succeeded, false otherwise
             *
             * @details
             * Performs complete window initialization:
             * 1. Validates that window is not already initialized
             * 2. Stores the window descriptor (via WindowBase)
             * 3. Registers the window class
             * 4. Creates the native Win32 window
             * 5. Creates the appropriate graphics context based on desc.api
             * 6. Initializes the graphics context
             *
             * @pre desc must contain valid dimensions (width > 0, height > 0)
             * @pre desc.api must be a supported graphics API
             * @post Window is ready for rendering and event processing
             *
             * @note If SetGraphicsConfig() was called before Initialize(), the custom
             * configuration will be used instead of defaults
             */
            [[nodiscard]] virtual bool Initialize(const WindowDesc &desc) noexcept override;

            /**
             * @brief Shutdown the Win32 window
             * @details
             * Performs complete cleanup in reverse order of initialization:
             * 1. Destroys the graphics context
             * 2. Destroys the native window
             * 3. Unregisters the window class
             *
             * @note Safe to call multiple times (subsequent calls are no-ops)
             * @note Automatically called by destructor if needed
             * @post Window is no longer valid for rendering or event processing
             */
            virtual void Shutdown() noexcept override;

            /**
             * @brief Process pending window events
             * @details
             * Processes all pending messages in the window's message queue including:
             * - Keyboard input (WM_KEYDOWN, WM_KEYUP)
             * - Mouse input (WM_MOUSEMOVE, WM_LBUTTONDOWN, etc.)
             * - Window events (WM_SIZE, WM_MOVE, WM_CLOSE, etc.)
             * - System events (WM_PAINT, WM_TIMER, etc.)
             *
             * @note This must be called regularly (typically once per frame) to keep
             * the window responsive to user input and system events
             * @note Uses PeekMessage to avoid blocking the calling thread
             */
            virtual void PollEvents() noexcept override;

            /**
             * @brief Swap front and back buffers
             * @details
             * Presents the rendered frame by swapping buffers through the graphics context.
             * This delegates to the associated IGraphicsContext::SwapBuffers().
             *
             * @note Only swaps if context is valid
             * @note Buffer swapping is synchronized based on vsync setting
             */
            virtual void SwapBuffers() noexcept override;

            /**
             * @brief Resize the window
             * @param width New width in pixels
             * @param height New height in pixels
             *
             * @details
             * Resizes the window to the specified dimensions and notifies the
             * graphics context of the size change. The window's client area will
             * be exactly width × height pixels.
             *
             * @note Automatically updates the graphics context viewport
             * @note Does nothing if window handle is null
             */
            virtual void Resize(const uint32_t width, const uint32_t height) noexcept override;

            /**
             * @brief Get the native window handle
             * @return Pointer to HWND (cast to void* for platform abstraction)
             *
             * @details
             * Returns the native Win32 window handle (HWND) as a void pointer.
             * This allows platform-specific code to access the underlying Win32 handle
             * when needed while maintaining the platform-abstract interface.
             *
             * @note Cast returned pointer to HWND for Win32-specific operations
             * @note Returns nullptr if window has not been created
             */
            virtual void *GetNativeHandle() const noexcept override { return hwnd_; }

            /**
             * @brief Get the window's graphics context (mutable)
             * @return Pointer to the IGraphicsContext, or nullptr if not initialized
             *
             * @details
             * Provides mutable access to the graphics context for operations
             * that may modify context state (e.g., making the context current).
             * The context is created during window initialization based on the
             * selected graphics API (OpenGL, Vulkan, Direct3D).
             *
             * @note Returns nullptr if called before Initialize() or after Shutdown()
             * @note The pointer remains valid for the window's lifetime
             */
            [[nodiscard]] virtual IGraphicsContext *GetGraphicsContext() noexcept override { return context_.get(); }

            /**
             * @brief Get the window's graphics context (const)
             * @return Const pointer to the IGraphicsContext, or nullptr if not initialized
             *
             * @details
             * Provides const access to the graphics context for inspection
             * without modification. Useful for checking context validity or
             * querying context properties.
             *
             * @note Returns nullptr if called before Initialize() or after Shutdown()
             * @note The pointer remains valid for the window's lifetime
             */
            [[nodiscard]] virtual const IGraphicsContext *GetGraphicsContext() const noexcept override { return context_.get(); }

        public:
            /**
             * @brief Set advanced graphics configuration
             * @param config Complete graphics configuration to override defaults
             *
             * @details
             * Allows advanced users to provide a complete GraphicsConfig that overrides
             * the default configuration generated from WindowDesc. This enables fine-grained
             * control over all graphics settings including surface configuration,
             * platform-specific parameters, and API-specific settings.
             *
             * @pre Must be called before Initialize() to take effect
             * @pre config must be a complete, valid GraphicsConfig structure
             * @note If not called, Initialize() will generate sensible defaults automatically
             * @note Useful for advanced features like custom pixel formats, debug layers, etc.
             */
            void SetGraphicsConfig(const GraphicsConfig &config) noexcept;

        private:
            /**
             * @brief Window procedure callback for handling Win32 messages
             * @param hwnd Window handle receiving the message
             * @param msg Message identifier (e.g., WM_SIZE, WM_CLOSE)
             * @param wParam Message-specific parameter (depends on msg)
             * @param lParam Message-specific parameter (depends on msg)
             * @return LRESULT result of message processing, typically 0 if handled
             *
             * @details
             * Static window procedure that processes all Win32 messages for the window.
             * Handles essential messages including:
             * - WM_CREATE: Store instance pointer in window user data
             * - WM_SIZE: Handle window resizing
             * - WM_CLOSE/WM_DESTROY: Set close flag and post quit message
             * - WM_KEYDOWN: Handle keyboard input (e.g., ESC to close)
             * - WM_SETFOCUS/WM_KILLFOCUS: Log focus changes
             * - All other messages forwarded to DefWindowProc
             *
             * @note This must be static to serve as a Win32 callback
             * @note Uses GetWindowLongPtr to retrieve instance pointer from window user data
             * @note All unhandled messages are passed to DefWindowProc for default processing
             */
            static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

        private:
            /**
             * @brief Register the Win32 window class
             * @return true if registration succeeded, false otherwise
             *
             * @details
             * Registers a custom window class (HUD3D_WINDOW_CLASS) with the following attributes:
             * - Window procedure: WndProc
             * - Style: CS_HREDRAW | CS_VREDRAW | CS_OWNDC
             * - Background: COLOR_WINDOW
             * - Icons: Default application icon
             * - Cursor: Default arrow cursor
             *
             * @note Must be called before CreateNativeWindow()
             * @note Only needs to be called once per application instance
             * @post Window class is registered and ready for window creation
             */
            [[nodiscard]] bool RegisterWindowClass() noexcept;

            /**
             * @brief Unregister the Win32 window class
             * @details
             * Unregisters the custom window class (HUD3D_WINDOW_CLASS) from the system.
             * This is typically called during shutdown to clean up resources.
             *
             * @note Safe to call even if window class was never registered
             * @post Window class is no longer available for window creation
             */
            void UnregisterWindowClass() noexcept;

            /**
             * @brief Create the native Win32 window
             * @param title Window title string (UTF-8)
             * @param width Client area width in pixels
             * @param height Client area height in pixels
             * @return true if window creation succeeded, false otherwise
             *
             * @details
             * Creates the actual Win32 window with the following characteristics:
             * - Style: WS_OVERLAPPEDWINDOW (resizable, with title bar, borders, system menu)
             * - Extended style: WS_EX_APPWINDOW | WS_EX_WINDOWEDGE
             * - Position: CW_USEDEFAULT (let Windows decide initial position)
             * - Client area: Exactly width × height pixels
             * - Associated instance: Application HINSTANCE
             *
             * Calculates full window dimensions using AdjustWindowRect to ensure
             * the client area matches the requested dimensions.
             *
             * @note Requires RegisterWindowClass() to be called first
             * @note Sets instance_ptr_ to this for WndProc access
             * @post Window is created, shown, and has keyboard focus
             */
            [[nodiscard]] bool CreateNativeWindow(const char *title, uint32_t width, uint32_t height) noexcept;

            /**
             * @brief Create default graphics configuration from WindowDesc
             * @param desc Window descriptor containing basic parameters
             * @return Complete GraphicsConfig with sensible defaults
             *
             * @details
             * Generates a complete GraphicsConfig by expanding the basic WindowDesc
             * parameters with sensible defaults for all configuration categories:
             * 1. Basic display (width, height, api, vsync) - copied from desc
             * 2. Surface configuration (Window type, double buffering, sRGB)
             * 3. Platform configuration (Windows platform with default settings)
             * 4. API configuration (OpenGL 4.6 Core Profile, etc.)
             *
             * @note Advanced users can override these defaults using SetGraphicsConfig()
             * @note This is called automatically by Initialize() if no custom config was set
             *
             * @par Default Values:
             * - Surface: Window type, double buffered, sRGB disabled
             * - Platform: WindowsConfig with WGL swap control enabled
             * - OpenGL: Version 4.6 Core Profile
             */
            [[nodiscard]] GraphicsConfig CreateDefaultGraphicsConfig(const WindowDesc &desc) const noexcept;

        private:
            static constexpr const char *WINDOW_CLASS_NAME = "HUD3D_WINDOW_CLASS"; ///< Win32 window class name
            static Win32Window *instance_ptr_;                                     ///< Singleton for WndProc callback access

        private:
            HWND hwnd_;                                 ///< Native window handle (HWND)
            HINSTANCE hinstance_;                       ///< Application instance handle
            bool is_initialized_;                       ///< Window initialization status
            std::unique_ptr<IGraphicsContext> context_; ///< Graphics context (OpenGL/Vulkan/D3D)
        };

    } // namespace platform
} // namespace hud_3d
