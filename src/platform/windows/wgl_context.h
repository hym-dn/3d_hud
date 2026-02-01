/**
 * @file wgl_context.h
 * @brief Windows WGL graphics context implementation header
 *
 * @details
 * Declares the Windows-specific WGL graphics context implementation.
 * This header should only be included by platform-specific source files.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */
#pragma once

#include <windows.h>
#include "glad/gl.h"
#include "platform/graphics_context.h"

namespace hud_3d
{
    namespace platform
    {
        /**
         * @class WGLContext
         * @brief Windows WGL graphics context implementation
         *
         * @details
         * Implements platform-specific graphics context using WGL for OpenGL rendering on Windows.
         * This class directly uses the WindowsConfig structure from our platform configuration.
         *
         * WGLContext manages the complete lifecycle of an OpenGL rendering context on Windows,
         * including pixel format selection, context creation, WGL extension loading, and
         * resource cleanup. It supports both traditional pixel format selection and advanced
         * ARB pixel format extensions for fine-grained control over rendering capabilities.
         *
         * @par Key Features:
         * - Automatic pixel format selection with sensible defaults
         * - Support for WGL extensions (WGL_ARB_pixel_format, WGL_EXT_swap_control)
         * - Double buffering and vertical synchronization control
         * - Resource management with RAII compliance
         *
         * @par Lifecycle:
         * 1. Initialize() - Create and setup the WGL context
         * 2. MakeCurrent() - Bind context for rendering
         * 3. SwapBuffers() - Present rendered frame
         * 4. Destroy() - Clean up resources
         *
         * @see IGraphicsContext
         * @see GraphicsConfig
         * @see WindowsConfig
         */
        class WGLContext : public IGraphicsContext
        {
        public:
            /**
             * @enum WGLConstants
             * @brief WGL ARB extension constants for type-safe pixel format attributes
             *
             * @details
             * Defines WGL_ARB_pixel_format extension constants using GLenum for compile-time
             * type safety. These constants are used when configuring pixel format attributes
             * through the ARB extension interface.
             */
            enum WGLConstants : GLenum
            {
                // Pixel format attributes
                WGL_DRAW_TO_WINDOW_ARB = 0x2001, ///< Buffer can draw to window
                WGL_SUPPORT_OPENGL_ARB = 0x2010, ///< Support OpenGL rendering
                WGL_DOUBLE_BUFFER_ARB = 0x2011,  ///< Enable double buffering
                WGL_PIXEL_TYPE_ARB = 0x2013,     ///< Pixel data type
                WGL_COLOR_BITS_ARB = 0x2014,     ///< Color buffer bit depth
                WGL_DEPTH_BITS_ARB = 0x2022,     ///< Depth buffer bit depth
                WGL_STENCIL_BITS_ARB = 0x2023,   ///< Stencil buffer bit depth
                WGL_SAMPLE_BUFFERS_ARB = 0x2041, ///< Enable multisampling
                WGL_SAMPLES_ARB = 0x2042,        ///< Number of multisamples
                WGL_ACCELERATION_ARB = 0x2003,   ///< Hardware acceleration type
                WGL_SWAP_METHOD_ARB = 0x2007,    ///< Buffer swap method

                // Pixel type and value constants
                WGL_TYPE_RGBA_ARB = 0x202B,         ///< RGBA pixel format
                WGL_FULL_ACCELERATION_ARB = 0x2027, ///< Full hardware acceleration
                WGL_SWAP_EXCHANGE_ARB = 0x2028,     ///< Exchange swap method
            };

        public:
            /**
             * @brief Default constructor for WGLContext
             * @details
             * Constructs a WGLContext in uninitialized state. The context must be initialized
             * with a valid GraphicsConfig before use.
             */
            WGLContext() = default;

            /**
             * @brief Destructor for WGLContext
             * @details
             * Automatically releases all WGL resources including rendering context,
             * device context, and loaded extension function pointers.
             * @note This destructor ensures proper cleanup and prevents resource leaks
             * @throws No exceptions are thrown from this destructor
             */
            ~WGLContext() override;

        public:
            /**
             * @brief Initialize the WGL graphics context
             * @param config Complete graphics configuration including window handle
             * @return true if initialization succeeded, false otherwise
             *
             * @details
             * Performs the complete initialization sequence:
             * 1. Validates window handle from GraphicsConfig
             * 2. Sets up pixel format for the device context
             * 3. Creates the OpenGL rendering context
             * 4. Makes the context current
             * 5. Loads WGL extension functions
             * 6. Applies platform-specific configuration
             *
             * @pre The GraphicsConfig must contain a valid Windows window handle
             * @post The WGL context is ready for rendering
             */
            [[nodiscard]] virtual bool Initialize(const GraphicsConfig &config) noexcept override;

            /**
             * @brief Make the OpenGL context current for the calling thread
             * @return true if context was made current successfully, false otherwise
             *
             * @details
             * Binds the OpenGL rendering context to the current thread and associates
             * it with the window's device context. This must be called before any
             * OpenGL rendering commands are issued.
             *
             * @note Each thread can have only one current context at a time
             * @note The context must remain current while rendering
             */
            [[nodiscard]] virtual bool MakeCurrent() noexcept override;

            /**
             * @brief Swap front and back buffers
             * @return true if buffers were swapped successfully, false otherwise
             *
             * @details
             * Presents the rendered frame by swapping the front and back buffers.
             * When double buffering is enabled (default), this exchanges the visible
             * front buffer with the rendered back buffer, making the new frame visible.
             *
             * @note This function must be called after rendering to present the frame
             * @note Buffer swapping is synchronized based on the vsync setting
             */
            [[nodiscard]] virtual bool SwapBuffers() noexcept override;

            /**
             * @brief Enable or disable vertical synchronization
             * @param enable true to enable VSync, false to disable
             * @return true if VSync was set successfully, false otherwise
             *
             * @details
             * Controls vertical synchronization using the WGL_EXT_swap_control extension.
             * When enabled (default), buffer swaps are synchronized with the monitor's
             * refresh rate, preventing screen tearing. When disabled, swaps occur
             * immediately, potentially increasing frame rate at the cost of visual quality.
             *
             * @note Requires WGL_EXT_swap_control extension support
             * @note The change takes effect on the next buffer swap
             */
            [[nodiscard]] virtual bool SetVSync(const bool enable) noexcept override;

            /**
             * @brief Apply platform-specific configuration
             * @param config Platform-specific configuration parameters
             * @return true if configuration was applied successfully, false otherwise
             *
             * @details
             * Applies Windows-specific configuration from WindowsConfig structure.
             * Currently handles WGL swap control settings. Note that pixel format
             * changes require context recreation and cannot be applied dynamically.
             *
             * @note Some configuration changes may require context recreation
             * @note Direct3D/Vulkan configurations are not applicable to WGL contexts
             */
            [[nodiscard]] virtual bool ApplyPlatformConfig(const PlatformConfig &config) noexcept override;

            /**
             * @brief Check if context is valid and ready for rendering
             * @return true if context is valid, false otherwise
             *
             * @details
             * Validates that the rendering context, device context, and window handle
             * are all properly initialized and ready for rendering operations.
             *
             * @note This should be checked before attempting any rendering operations
             */
            virtual bool IsValid() const noexcept override;

            /**
             * @brief Get the graphics API being used
             * @return ContextAPI::OpenGL (always returns OpenGL for WGLContext)
             */
            virtual ContextAPI GetAPI() const noexcept override;

            /**
             * @brief Get the platform type
             * @return PlatformType::Windows (always returns Windows for WGLContext)
             */
            virtual PlatformType GetPlatform() const noexcept override;

            /**
             * @brief Get platform-specific function pointer for OpenGL extensions
             * @param function_name Name of the OpenGL extension function to load
             * @return Function pointer, nullptr if function not available
             *
             * @details
             * Loads OpenGL extension functions using wglGetProcAddress.
             * This is the standard mechanism for accessing OpenGL extensions on Windows.
             *
             * @note Function pointers are context-specific and may vary between contexts
             * @note Only works for OpenGL 1.1+ functions and extensions
             */
            [[nodiscard]] virtual void *GetProcAddress(const char *function_name) noexcept override;

            /**
             * @brief Destroy the WGL context and release all resources
             * @details
             * Performs complete cleanup of the WGL context:
             * 1. Makes the context not current
             * 2. Deletes the OpenGL rendering context
             * 3. Releases the device context
             * 4. Clears stored handles and configuration
             *
             * @note Safe to call multiple times (subsequent calls are no-ops)
             * @post The context is no longer valid for rendering
             */
            virtual void Destroy() noexcept override;

            /**
             * @brief Resize the rendering surface
             * @param width New width in pixels
             * @param height New height in pixels
             * @return true if resize was successful
             *
             * @details
             * Updates the viewport dimensions for the OpenGL context.
             * This should be called when the associated window is resized to ensure
             * proper rendering to the new dimensions.
             *
             * @note The window's backbuffer size is automatically updated by the OS
             * @note This updates the OpenGL viewport state
             */
            virtual bool Resize(const uint32_t width, const uint32_t height) noexcept override;

        private:
            /**
             * @brief Set up the pixel format for the device context
             * @return true if pixel format was set up successfully, false otherwise
             *
             * @details
             * Configures the pixel format for the window's device context using either:
             * 1. Traditional ChoosePixelFormat() for automatic selection (default)
             * 2. wglChoosePixelFormatARB() for advanced pixel format control
             *
             * Sets up color buffer, depth buffer, stencil buffer, and multisampling
             * parameters based on the GraphicsConfig settings.
             *
             * @note Pixel format cannot be changed after context creation
             * @pre Device context must be valid
             */
            bool SetupPixelFormat() noexcept;

            /**
             * @brief Create the OpenGL rendering context
             * @return true if context creation succeeded, false otherwise
             *
             * @details
             * Creates an OpenGL rendering context compatible with the configured
             * pixel format and associates it with the device context.
             *
             * @pre Pixel format must be set up successfully
             * @post Rendering context is ready for activation
             */
            bool CreateContext() noexcept;

            /**
             * @brief Load WGL extension functions
             * @details
             * Dynamically loads WGL extension function pointers using wglGetProcAddress.
             * Currently loads:
             * - wglSwapIntervalEXT (for vsync control)
             * - wglChoosePixelFormatARB (advanced pixel format selection)
             * - wglGetPixelFormatAttribivARB (pixel format queries)
             *
             * @note Extensions are loaded after context creation and must be current
             * @note Logs extension availability for debugging
             */
            void LoadWGLExtensions() noexcept;

            /**
             * @brief Validate the window handle
             * @param window_handle Window handle to validate
             * @return true if window handle is valid, false otherwise
             *
             * @details
             * Validates that the window handle is non-null and belongs to a valid
             * window that can support OpenGL rendering.
             *
             * @note Performs basic handle validation only
             * @note Does not verify window class or style compatibility
             */
            bool ValidateWindowHandle(HWND window_handle) const noexcept;

        private: // Disable copy semantics, move semantics for RAII compliance
            WGLContext(const WGLContext &) = delete;
            WGLContext &operator=(const WGLContext &) = delete;
            WGLContext(WGLContext &&) = delete;
            WGLContext &operator=(WGLContext &&) = delete;

        private:
            HDC device_context_{nullptr};      ///< Windows device context handle
            HGLRC rendering_context_{nullptr}; ///< OpenGL rendering context handle
            HWND window_handle_{nullptr};      ///< Associated window handle
            GraphicsConfig config_;            ///< Complete graphics configuration
            bool initialized_{false};          ///< Context initialization status

            // WGL extension function pointers
            using wglSwapIntervalEXT_t = BOOL(WINAPI *)(int interval);
            wglSwapIntervalEXT_t wgl_swap_interval_ext_{nullptr}; ///< WGL swap interval extension

            // WGL ARB pixel format extension function pointers
            using wglChoosePixelFormatARB_t = BOOL(WINAPI *)(HDC hdc, const int *piAttribIList,
                                                             const FLOAT *pfAttribFList, UINT nMaxFormats,
                                                             int *piFormats, UINT *nNumFormats);
            using wglGetPixelFormatAttribivARB_t = BOOL(WINAPI *)(HDC hdc, int iPixelFormat, int iLayerPlane,
                                                                  UINT nAttributes, const int *piAttributes,
                                                                  int *piValues);
            wglChoosePixelFormatARB_t wgl_choose_pixel_format_arb_{nullptr};             ///< WGL choose pixel format extension
            wglGetPixelFormatAttribivARB_t wgl_get_pixel_format_attrib_iv_arb_{nullptr}; ///< WGL get pixel format attributes extension
        };
    }
}