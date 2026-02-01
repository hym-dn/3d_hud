/**
 * @file graphics_context.h
 * @brief Platform-specific graphics context management interface
 *
 * @details
 * Defines the platform-specific interface for graphics context creation and management.
 * This file belongs to the platform abstraction layer, not a separate graphics layer.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#include <memory>
#include <vector>
#include <variant>

namespace hud_3d
{
    namespace platform
    {
        /**
         * @enum ContextAPI
         * @brief Supported graphics rendering APIs
         *
         * @details
         * Defines the available graphics APIs that can be used by the rendering engine.
         * Each API has different capabilities and platform support requirements.
         */
        enum class ContextAPI
        {
            OpenGL,    ///< Desktop OpenGL API (Windows, Linux)
            OpenGL_ES, ///< Embedded Systems OpenGL (Android, QNX)
            Vulkan,    ///< Next-generation low-level API (Cross-platform)
            Direct3D,  ///< Microsoft Direct3D API (Windows only)
            Metal      ///< Apple Metal API (macOS, iOS)
        };

        /**
         * @enum PlatformType
         * @brief Supported target platforms for the rendering engine
         *
         * @details
         * Identifies the operating system platforms that the engine can target.
         * Each platform may support different graphics APIs and capabilities.
         */
        enum class PlatformType
        {
            Windows, ///< Microsoft Windows desktop and embedded
            Linux,   ///< Linux-based systems (X11, Wayland)
            Android, ///< Android mobile and embedded platforms
            QNX      ///< QNX real-time operating system
        };

        /**
         * @struct ColorBufferConfig
         * @brief Color buffer and framebuffer configuration parameters
         *
         * @details
         * Defines the color, depth, and stencil buffer configurations for the
         * graphics context. These parameters control the precision and quality
         * of the rendered output.
         */
        struct ColorBufferConfig
        {
            std::int32_t red_bits{8};     ///< Number of bits per red color channel (typically 8)
            std::int32_t green_bits{8};   ///< Number of bits per green color channel (typically 8)
            std::int32_t blue_bits{8};    ///< Number of bits per blue color channel (typically 8)
            std::int32_t alpha_bits{8};   ///< Number of bits for alpha transparency channel
            std::int32_t depth_bits{24};  ///< Precision of depth buffer in bits (16, 24, or 32)
            std::int32_t stencil_bits{8}; ///< Precision of stencil buffer in bits (typically 8)
            std::int32_t samples{0};      ///< Multisample anti-aliasing samples (0=disabled, 2,4,8,16)
        };

        /**
         * @struct ContextConfig
         * @brief Graphics context version and capability settings
         *
         * @details
         * Controls the version and feature set of the graphics context.
         * These settings ensure compatibility with specific hardware capabilities
         * and enable advanced graphics features.
         */
        struct ContextConfig
        {
            std::int32_t major_version{4}; ///< Major version of graphics API (e.g., 4 for OpenGL 4.x)
            std::int32_t minor_version{6}; ///< Minor version of graphics API (e.g., 6 for OpenGL 4.6)

            // Core profile features
            bool core_profile{true};       ///< Use core profile (excludes deprecated functionality)
            bool forward_compatible{true}; ///< Forward-compatible context (excludes features marked for removal)
            bool debug_context{false};     ///< Enable debug context for development and profiling

            // Advanced features
            bool robust_access{false};   ///< Enable robust buffer access (prevents GPU crashes from shader errors)
            bool reset_isolation{false}; ///< Enable GPU reset notification (handles driver recovery)
        };

        /**
         * @struct SurfaceConfig
         * @brief Rendering surface type and capabilities
         *
         * @details
         * Defines the type of rendering surface and its capabilities.
         * Different surface types are suited for different use cases
         * from windowed applications to embedded display systems.
         */
        struct SurfaceConfig
        {
            /**
             * @enum SurfaceType
             * @brief Type of rendering surface
             */
            enum class SurfaceType
            {
                Window,                  ///< Native window surface for desktop applications
                PBuffer,                 ///< Pixel buffer for offscreen rendering
                Pixmap,                  ///< Pixmap surface for image-based rendering
                Scanout                  ///< Direct scanout for embedded and automotive displays
            } type{SurfaceType::Window}; ///< Selected surface type
            bool double_buffered{true};  ///< Enable double buffering for flicker-free rendering
            bool srgb_capable{false};    ///< Support sRGB color space for accurate color reproduction

            /**
             * @struct SurfaceHandle
             * @brief Platform-specific surface handle container
             *
             * @details
             * Contains platform-specific surface handles using std::variant for type safety.
             * Each platform uses its native surface handle type.
             */
            struct SurfaceHandle
            {
                // Windows: HWND (window handle) or HDC (device context)
                struct WindowsHandle
                {
                    void *window_handle{nullptr};  ///< Window handle for windowed rendering
                    void *device_context{nullptr}; ///< Device context for offscreen rendering
                    bool is_window_handle{true};   ///< True if using window handle, false for DC
                };

                // Linux: Window (X11) or Display
                struct LinuxHandle
                {
                    void *display{nullptr};  ///< X11 Display pointer
                    unsigned long window{0}; ///< X11 Window ID
                };

                // Android: ANativeWindow or EGLSurface
                struct AndroidHandle
                {
                    void *native_window{nullptr}; ///< ANativeWindow pointer
                    void *egl_surface{nullptr};   ///< EGLSurface handle
                };

                // QNX: screen_context_t or screen_window_t
                struct QNXHandle
                {
                    void *screen_context{nullptr}; ///< Screen context handle
                    void *screen_window{nullptr};  ///< Screen window handle
                };

                std::variant<WindowsHandle, LinuxHandle, AndroidHandle, QNXHandle> handle;

                template <typename T>
                bool IsPlatform() const { return std::holds_alternative<T>(handle); }

                template <typename T>
                T &GetPlatform() { return std::get<T>(handle); }

                template <typename T>
                const T &GetPlatform() const { return std::get<T>(handle); }

                template <typename T>
                void SetPlatform(const T &platform_handle) { handle = platform_handle; }

                bool IsValid() const
                {
                    return std::visit([](auto &&arg)
                                      {
                                        using T = std::decay_t<decltype(arg)>;
                                        if constexpr (std::is_same_v<T, WindowsHandle>)
                                        {
                                            return arg.window_handle != nullptr || arg.device_context != nullptr;
                                        }
                                        else if constexpr (std::is_same_v<T, LinuxHandle>)
                                        {
                                            return arg.display != nullptr && arg.window != 0;
                                        }
                                        else if constexpr (std::is_same_v<T, AndroidHandle>)
                                        {
                                            return arg.native_window != nullptr;
                                        }
                                        else if constexpr (std::is_same_v<T, QNXHandle>)
                                        {
                                            return arg.screen_context != nullptr;
                                        }
                                        return false; }, handle);
                }
            } handle; ///< Platform-specific surface handle

            // Helper methods for surface validation
            bool IsValid() const
            {
                return handle.IsValid();
            }

            template <typename T>
            bool IsPlatform() const
            {
                return handle.IsPlatform<T>();
            }

            template <typename T>
            T &GetPlatform()
            {
                return handle.GetPlatform<T>();
            }

            template <typename T>
            const T &GetPlatform() const
            {
                return handle.GetPlatform<T>();
            }
        };

        /**
         * @struct WindowsConfig
         * @brief Windows-specific configuration parameters
         *
         * @details
         * Parameters for Windows platforms, supporting both WGL (OpenGL)
         * and Direct3D graphics APIs.
         */
        struct WindowsConfig
        {
            // WGL configuration for OpenGL
            std::int32_t pixel_format_arb{0}; ///< ARB pixel format selection
            bool wgl_swap_control{true};      ///< Enable WGL swap control extension

            // Direct3D configuration
            bool enable_d3d_debug_layer{false}; ///< Enable Direct3D debug layer
            std::int32_t d3d_feature_level{0};  ///< Direct3D feature level (0=auto)
        };

        /**
         * @struct LinuxConfig
         * @brief Linux-specific configuration parameters
         *
         * @details
         * Parameters for Linux platforms, supporting EGL and X11/GLX.
         */
        struct LinuxConfig
        {
            // EGL configuration
            bool enable_robust_access{false};        ///< Enable EGL robust context access
            bool reset_on_video_memory_purge{false}; ///< Reset on video memory purge events

            // X11/GLX configuration
            bool use_x11_visual{true};         ///< Use X11 visual for window creation
            std::int32_t glx_context_flags{0}; ///< GLX context creation flags
        };

        /**
         * @struct AndroidConfig
         * @brief Android-specific configuration parameters
         *
         * @details
         * Parameters for Android platforms, using EGL for graphics context creation.
         */
        struct AndroidConfig
        {
            // EGL configuration
            bool enable_robust_access{false};        ///< Enable EGL robust context access
            bool reset_on_video_memory_purge{false}; ///< Reset on video memory purge events

            // Android-specific
            bool preserve_egl_context{true};      ///< Preserve EGL context on pause
            std::int32_t native_window_format{0}; ///< Native window pixel format
        };

        /**
         * @struct QNXConfig
         * @brief QNX-specific configuration parameters
         *
         * @details
         * Parameters for QNX platforms, using EGL for embedded graphics.
         */
        struct QNXConfig
        {
            // EGL configuration
            bool enable_robust_access{false};        ///< Enable EGL robust context access
            bool reset_on_video_memory_purge{false}; ///< Reset on video memory purge events

            // QNX-specific
            bool use_screen_context{true};     ///< Use Screen graphics context
            std::int32_t screen_display_id{0}; ///< Screen display ID for multi-display
        };

        /**
         * @struct VulkanConfig
         * @brief Vulkan-specific configuration parameters
         *
         * @details
         * Parameters specific to the Vulkan graphics API.
         * These settings apply regardless of the underlying platform.
         */
        struct VulkanConfig
        {
            uint32_t validation_layers{0};         ///< Number of Vulkan validation layers to enable
            bool enable_validation{false};         ///< Enable Vulkan validation for debugging
            bool enable_swapchain_extension{true}; ///< Enable swapchain extension support
        };

        /**
         * @typedef PlatformConfigVariant
         * @brief Platform-specific configuration variant
         *
         * @details
         * A type-safe union of all platform-specific configurations.
         * Only one platform configuration is active at any given time,
         * determined by the compilation target or runtime selection.
         */
        using PlatformConfigVariant = std::variant<WindowsConfig, LinuxConfig, AndroidConfig, QNXConfig>;

        /**
         * @struct PlatformConfig
         * @brief Platform-specific configuration parameters
         *
         * @details
         * Contains configuration parameters specific to the target platform.
         * Uses std::variant to ensure type safety and prevent configuration conflicts.
         */
        struct PlatformConfig
        {
            PlatformConfigVariant config; ///< Active platform configuration
            VulkanConfig vulkan;          ///< Vulkan API configuration (cross-platform)

            /**
             * @brief Check if the current configuration matches a specific platform type
             * @tparam T Platform configuration type (WindowsConfig, LinuxConfig, etc.)
             * @return true if the active configuration is of type T
             */
            template <typename T>
            bool IsPlatform() const
            {
                return std::holds_alternative<T>(config);
            }

            /**
             * @brief Get the active platform configuration
             * @tparam T Platform configuration type
             * @return Reference to the active configuration of type T
             * @throws std::bad_variant_access if the active configuration is not of type T
             */
            template <typename T>
            T &GetPlatform()
            {
                return std::get<T>(config);
            }

            /**
             * @brief Get the active platform configuration (const version)
             * @tparam T Platform configuration type
             * @return Const reference to the active configuration of type T
             * @throws std::bad_variant_access if the active configuration is not of type T
             */
            template <typename T>
            const T &GetPlatform() const
            {
                return std::get<T>(config);
            }

            /**
             * @brief Set the active platform configuration
             * @tparam T Platform configuration type
             * @param platform_config Configuration to set
             */
            template <typename T>
            void SetPlatform(const T &platform_config)
            {
                config = platform_config;
            }

            /**
             * @brief Visit the active platform configuration with a visitor function
             * @tparam Visitor Visitor function type
             * @param visitor Visitor function to apply to the active configuration
             */
            template <typename Visitor>
            auto VisitPlatform(Visitor &&visitor)
            {
                return std::visit(std::forward<Visitor>(visitor), config);
            }

            /**
             * @brief Visit the active platform configuration with a visitor function (const version)
             * @tparam Visitor Visitor function type
             * @param visitor Visitor function to apply to the active configuration
             */
            template <typename Visitor>
            auto VisitPlatform(Visitor &&visitor) const
            {
                return std::visit(std::forward<Visitor>(visitor), config);
            }
        };

        /**
         * @struct APIConfig
         * @brief API-specific configuration parameters
         *
         * @details
         * Contains configuration parameters specific to each graphics API.
         * Uses std::variant to ensure type safety and prevent configuration conflicts.
         */
        struct APIConfig
        {
            /**
             * @struct OpenGLConfig
             * @brief OpenGL-specific configuration parameters
             */
            struct OpenGLConfig
            {
                // Platform-specific OpenGL implementations
                struct WGLConfig
                {
                    std::int32_t pixel_format_arb{0}; ///< ARB pixel format selection
                    bool wgl_swap_control{true};      ///< Enable WGL swap control extension
                };

                struct GLXConfig
                {
                    bool use_x11_visual{true};         ///< Use X11 visual for window creation
                    std::int32_t glx_context_flags{0}; ///< GLX context creation flags
                };

                struct EGLConfig
                {
                    bool enable_robust_access{false};        ///< Enable EGL robust context access
                    bool reset_on_video_memory_purge{false}; ///< Reset on video memory purge events
                };

                // OpenGL version and feature configuration
                ContextConfig context;          ///< Context version and capabilities
                ColorBufferConfig color_buffer; ///< Color and buffer configuration

                // Platform-specific implementation
                std::variant<WGLConfig, GLXConfig, EGLConfig> platform_impl;
            };

            /**
             * @struct VulkanConfig
             * @brief Vulkan-specific configuration parameters
             */
            struct VulkanConfig
            {
                uint32_t validation_layers{0};         ///< Number of Vulkan validation layers to enable
                bool enable_validation{false};         ///< Enable Vulkan validation for debugging
                bool enable_swapchain_extension{true}; ///< Enable swapchain extension support
            };

            /**
             * @struct Direct3DConfig
             * @brief Direct3D-specific configuration parameters
             */
            struct Direct3DConfig
            {
                bool enable_d3d_debug_layer{false}; ///< Enable Direct3D debug layer
                std::int32_t d3d_feature_level{0};  ///< Direct3D feature level (0=auto)
            };

            /**
             * @struct MetalConfig
             * @brief Metal-specific configuration parameters
             */
            struct MetalConfig
            {
                bool enable_gpu_capture{false};  ///< Enable GPU capture for debugging
                std::int32_t mtl_feature_set{0}; ///< Metal feature set selection
            };

            std::variant<OpenGLConfig, VulkanConfig, Direct3DConfig, MetalConfig> config;

            template <typename T>
            bool IsAPI() const { return std::holds_alternative<T>(config); }

            template <typename T>
            T &GetAPI() { return std::get<T>(config); }

            template <typename T>
            const T &GetAPI() const { return std::get<T>(config); }

            template <typename T>
            void SetAPI(const T &api_config) { config = api_config; }
        };

        /**
         * @struct GraphicsConfig
         * @brief Complete graphics configuration for rendering context creation
         *
         * @details
         * Aggregates all configuration parameters needed to create a graphics
         * rendering context. Uses a clear hierarchical structure:
         * 1. Common display settings (resolution, vsync)
         * 2. Surface configuration (including handle)
         * 3. API-specific configuration
         */
        struct GraphicsConfig
        {
            // Basic display configuration (common to all APIs)
            std::int32_t width{1280};           ///< Render target width in pixels
            std::int32_t height{720};           ///< Render target height in pixels
            ContextAPI api{ContextAPI::OpenGL}; ///< Selected graphics API
            bool enable_vsync{true};            ///< Enable vertical synchronization

            // Surface configuration (now includes handle)
            SurfaceConfig surface; ///< Surface type, features, and platform handle

            // Platform-specific configuration
            PlatformConfig platform; ///< Platform-specific configuration parameters

            // API-specific configuration
            APIConfig api_config; ///< API-specific configuration parameters
        };

        /**
         * @class IGraphicsContext
         * @brief Platform-specific graphics context interface for a specific window
         *
         * @details
         * Represents a graphics rendering context associated with a specific window.
         * Each platform implements this interface using its native graphics APIs
         * (WGL, GLX, EGL, etc.). The context is created by WindowSystem for an
         * existing window (created by the application).
         *
         * Key design principles:
         * 1. Graphics context is associated with a specific window/surface
         * 2. Context creation is done by WindowSystem, not by application directly
         * 3. One context per window (or per rendering thread)
         * 4. Contexts can share resources (textures, buffers) when created as shared contexts
         */
        class IGraphicsContext
        {
        public:
            /**
             * @brief Virtual destructor for the IGraphicsContext interface.
             *
             * Ensures proper cleanup of derived class instances through the interface pointer.
             */
            virtual ~IGraphicsContext() = default;

        public:
            /**
             * @brief Initialize the graphics context for the associated window
             * @param config Graphics configuration parameters including window handle
             * @return true if initialization succeeded, false otherwise
             *
             * @note The GraphicsConfig must contain a valid window handle in its
             *       surface configuration. The context will be associated with
             *       this window for the duration of its lifetime.
             */
            [[nodiscard]] virtual bool Initialize(const GraphicsConfig &config) noexcept = 0;

            /**
             * @brief  Make the context current for rendering
             * @return true if context was made current successfully
             */
            [[nodiscard]] virtual bool MakeCurrent() noexcept = 0;
            
            /**
             * @brief Swap the front and back buffers
             * @return true if swap was successful
             */
            [[nodiscard]] virtual bool SwapBuffers() noexcept = 0;

            /**
             * @brief Set vertical synchronization using platform-specific mechanism
             * @param enable true to enable VSync, false to disable
             * @return true if VSync was set successfully
             *
             * @note This uses the platform-specific VSync mechanism (WGL, GLX, EGL, etc.)
             */
            [[nodiscard]] virtual bool SetVSync(const bool enable) noexcept = 0;

            /**
             * @brief Apply platform-specific configuration
             * @param config Platform-specific configuration to apply
             * @return true if configuration was applied successfully
             */
            [[nodiscard]] virtual bool ApplyPlatformConfig(const PlatformConfig &config) noexcept = 0;

            /**
             * @brief Check if context is valid and ready for rendering
             * @return true if context is valid
             */
            virtual bool IsValid() const noexcept = 0;

            /**
             * @brief Get the graphics API being used
             * @return Current graphics API
             */
            virtual ContextAPI GetAPI() const noexcept = 0;

            /**
             * @brief Get the platform type
             * @return Current platform type
             */
            virtual PlatformType GetPlatform() const noexcept = 0;

            /**
             * @brief Get platform-specific function pointer
             * @param function_name Name of the OpenGL/extension function to load
             * @return Function pointer, nullptr if function not available
             */
            virtual void *GetProcAddress(const char *function_name) noexcept = 0;

            /**
             * @brief Resize the graphics context to new dimensions
             * @param width New width in pixels
             * @param height New height in pixels
             * @return true if resize was successful
             *
             * @note This method should be called when the associated window is resized
             *       to update the rendering surface and viewport
             */
            virtual bool Resize(const uint32_t width, const uint32_t height) noexcept = 0;

            /**
             * @brief Destroy the graphics context and release platform-specific resources
             */
            virtual void Destroy() noexcept = 0;

        protected:
            /**
             * @brief Default constructor for the graphics context interface
             * @note This is an abstract base class constructor, meant to be inherited by platform-specific implementations
             */
            IGraphicsContext() = default;
        };
    }
}