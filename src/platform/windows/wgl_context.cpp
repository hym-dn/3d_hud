/**
 * @file wgl_context.cpp
 * @brief Windows WGL graphics context implementation (platform layer)
 *
 * @details
 * Implements Windows-specific graphics context using WGL (Windows Graphics Library)
 * for OpenGL rendering on Microsoft Windows platforms. This belongs to the platform
 * abstraction layer, not a separate graphics layer.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#include <stdexcept>
#include <utility>
#include "glad/gl.h"
#include "wgl_context.h"
#include "utils/perf/cpu_profiler.h"
#include "utils/perf/gpu_profiler.h"
#include "utils/log/log_manager.h"

namespace hud_3d
{
    namespace platform
    {
        namespace
        {
            // Anonymous namespace for file-local constants
            constexpr int32_t DEFAULT_COLOR_BITS = 32;
            constexpr int32_t DEFAULT_DEPTH_BITS = 24;
            constexpr int32_t DEFAULT_STENCIL_BITS = 8;
            constexpr int32_t DEFAULT_RED_BITS = 8;
            constexpr int32_t DEFAULT_GREEN_BITS = 8;
            constexpr int32_t DEFAULT_BLUE_BITS = 8;
            constexpr int32_t DEFAULT_ALPHA_BITS = 8;
        }

        WGLContext::~WGLContext()
        {
            Destroy();
        }

        bool WGLContext::Initialize(const GraphicsConfig &config) noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::Initialize");

            if (initialized_)
            {
                LOG_3D_HUD_WARN("WGLContext: Already initialized");
                return true;
            }

            config_ = config;

            // Extract Windows surface handle from configuration
            if (!config_.surface.handle.IsPlatform<SurfaceConfig::SurfaceHandle::WindowsHandle>())
            {
                LOG_3D_HUD_ERROR("WGLContext: Invalid platform handle type");
                return false;
            }

            const auto &win_handle = config_.surface.GetPlatform<SurfaceConfig::SurfaceHandle::WindowsHandle>();

            if (win_handle.is_window_handle)
            {
                window_handle_ = static_cast<HWND>(win_handle.window_handle);
                if (!ValidateWindowHandle(window_handle_))
                {
                    LOG_3D_HUD_ERROR("WGLContext: Invalid window handle");
                    return false;
                }
                device_context_ = GetDC(window_handle_);
                if (!device_context_)
                {
                    LOG_3D_HUD_ERROR("WGLContext: Failed to get device context");
                    return false;
                }
            }
            else
            {
                device_context_ = static_cast<HDC>(win_handle.device_context);
                window_handle_ = nullptr;
                if (!device_context_)
                {
                    LOG_3D_HUD_ERROR("WGLContext: Invalid device context");
                    return false;
                }
            }

            // Set up pixel format
            if (!SetupPixelFormat())
            {
                LOG_3D_HUD_ERROR("WGLContext: Failed to setup pixel format");
                Destroy();
                return false;
            }

            // Create OpenGL context
            if (!CreateContext())
            {
                LOG_3D_HUD_ERROR("WGLContext: Failed to create rendering context");
                Destroy();
                return false;
            }

            // Load WGL extensions
            LoadWGLExtensions();

            // Apply platform-specific configuration
            if (!ApplyPlatformConfig(config_.platform))
            {
                LOG_3D_HUD_WARN("WGLContext: Failed to apply some platform configurations");
            }

            initialized_ = true;
            LOG_3D_HUD_INFO("WGLContext: Successfully initialized");

            return true;
        }

        bool WGLContext::MakeCurrent() noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::MakeCurrent");

            if (!IsValid())
            {
                LOG_3D_HUD_ERROR("WGLContext: Cannot make invalid context current");
                return false;
            }

            return wglMakeCurrent(device_context_, rendering_context_) == TRUE;
        }

        bool WGLContext::SwapBuffers() noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::SwapBuffers");
            HUD_3D_GPU_FRAME_MARK();
            if (!initialized_ || !device_context_)
            {
                LOG_3D_HUD_ERROR("WGLContext: Cannot swap buffers on invalid context");
                return false;
            }
            return ::SwapBuffers(device_context_) == TRUE;
        }

        bool WGLContext::SetVSync(bool enable) noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::SetVSync");
            if (!initialized_)
            {
                LOG_3D_HUD_ERROR("WGLContext: Cannot set VSync on uninitialized context");
                return false;
            }
            // Apply VSync through WGL extension if available
            if (wgl_swap_interval_ext_)
            {
                const BOOL result = wgl_swap_interval_ext_(enable ? 1 : 0);
                if (result == TRUE)
                {
                    LOG_3D_HUD_INFO("WGLContext: VSync {} successfully", enable ? "enabled" : "disabled");
                    return true;
                }
                LOG_3D_HUD_ERROR("WGLContext: Failed to set VSync");
                return false;
            }
            // Fallback: update configuration
            LOG_3D_HUD_WARN("WGLContext: wglSwapIntervalEXT not available, VSync setting stored but not applied");
            config_.enable_vsync = enable;
            return true;
        }

        bool WGLContext::ApplyPlatformConfig(const PlatformConfig &config) noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::ApplyPlatformConfig");

            if (!initialized_)
            {
                LOG_3D_HUD_ERROR("WGLContext: Cannot apply platform config to uninitialized context");
                return false;
            }

            // Only Windows-specific configuration is applicable for WGL context
            if (!config.IsPlatform<WindowsConfig>())
            {
                LOG_3D_HUD_WARN("WGLContext: Non-Windows platform config provided, ignoring");
                return false;
            }

            const auto &win_config = config.GetPlatform<WindowsConfig>();
            bool all_applied = true;

            // Apply WGL swap control setting
            if (win_config.wgl_swap_control && wgl_swap_interval_ext_)
            {
                if (wgl_swap_interval_ext_(config_.enable_vsync ? 1 : 0) != TRUE)
                {
                    LOG_3D_HUD_WARN("WGLContext: Failed to apply WGL swap control");
                    all_applied = false;
                }
            }

            // Note: ARB pixel format cannot be changed after context creation
            if (win_config.pixel_format_arb > 0)
            {
                LOG_3D_HUD_WARN("WGLContext: ARB pixel format changes require context recreation");
                all_applied = false;
            }

            // Direct3D and Vulkan configurations are not applicable to WGL contexts
            if (config_.api == ContextAPI::Direct3D || config_.api == ContextAPI::Vulkan)
            {
                LOG_3D_HUD_WARN("WGLContext: Direct3D/Vulkan config applied to WGL context");
                all_applied = false;
            }

            return all_applied;
        }

        bool WGLContext::IsValid() const noexcept
        {
            return initialized_ && device_context_ && rendering_context_;
        }

        ContextAPI WGLContext::GetAPI() const noexcept
        {
            return config_.api;
        }

        PlatformType WGLContext::GetPlatform() const noexcept
        {
            return PlatformType::Windows;
        }

        void WGLContext::Destroy() noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::Destroy");

            if (!initialized_)
            {
                return;
            }

            if (rendering_context_)
            {
                // Ensure context is not current on any thread before deletion
                wglMakeCurrent(nullptr, nullptr);
                if (!wglDeleteContext(rendering_context_))
                {
                    LOG_3D_HUD_WARN("WGLContext: Failed to delete rendering context");
                }
                rendering_context_ = nullptr;
            }

            if (device_context_ && window_handle_)
            {
                if (!ReleaseDC(window_handle_, device_context_))
                {
                    LOG_3D_HUD_WARN("WGLContext: Failed to release device context");
                }
                device_context_ = nullptr;
            }

            // Note: We don't destroy the window_handle_ since it's provided by the user
            window_handle_ = nullptr;
            initialized_ = false;

            LOG_3D_HUD_INFO("WGLContext: Resources destroyed");
        }

        bool WGLContext::Resize(const uint32_t width, const uint32_t height) noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::Resize");

            if (!initialized_)
            {
                LOG_3D_HUD_WARN("WGLContext: Cannot resize uninitialized context");
                return false;
            }

            if (!window_handle_)
            {
                LOG_3D_HUD_WARN("WGLContext: Cannot resize, window handle is null");
                return false;
            }

            // 更新图形配置中的尺寸
            config_.width = width;
            config_.height = height;

            LOG_3D_HUD_INFO("WGLContext: Resized to {}x{}", width, height);
            return true;
        }

        bool WGLContext::ValidateWindowHandle(HWND window_handle) const noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::ValidateWindowHandle");

            if (!IsWindow(window_handle))
            {
                LOG_3D_HUD_ERROR("WGLContext: Invalid window handle");
                return false;
            }

            // Check if window has valid dimensions
            RECT rect{};
            if (!GetClientRect(window_handle, &rect))
            {
                LOG_3D_HUD_ERROR("WGLContext: Failed to get client rect");
                return false;
            }

            // Ensure window has non-zero dimensions
            const bool has_valid_dims = (rect.right > rect.left) && (rect.bottom > rect.top);
            if (!has_valid_dims)
            {
                LOG_3D_HUD_WARN("WGLContext: Window has zero dimensions");
            }

            return has_valid_dims;
        }

        bool WGLContext::SetupPixelFormat() noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::SetupPixelFormat");

            PIXELFORMATDESCRIPTOR pfd = {};
            pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.iLayerType = PFD_MAIN_PLANE;

            // Use ColorBufferConfig from graphics configuration
            if (config_.api_config.IsAPI<APIConfig::OpenGLConfig>())
            {
                const auto &opengl_config = config_.api_config.GetAPI<APIConfig::OpenGLConfig>();
                const auto &color_buffer = opengl_config.color_buffer;

                // Apply color buffer configuration
                pfd.cColorBits = color_buffer.red_bits + color_buffer.green_bits + color_buffer.blue_bits;
                pfd.cRedBits = color_buffer.red_bits;
                pfd.cGreenBits = color_buffer.green_bits;
                pfd.cBlueBits = color_buffer.blue_bits;
                pfd.cAlphaBits = color_buffer.alpha_bits;
                pfd.cDepthBits = color_buffer.depth_bits;
                pfd.cStencilBits = color_buffer.stencil_bits;
            }
            else
            {
                // Fallback to default values if not using OpenGL
                pfd.cColorBits = DEFAULT_COLOR_BITS;
                pfd.cRedBits = DEFAULT_RED_BITS;
                pfd.cGreenBits = DEFAULT_GREEN_BITS;
                pfd.cBlueBits = DEFAULT_BLUE_BITS;
                pfd.cAlphaBits = DEFAULT_ALPHA_BITS;
                pfd.cDepthBits = DEFAULT_DEPTH_BITS;
                pfd.cStencilBits = DEFAULT_STENCIL_BITS;
            }

            const int pixel_format = ChoosePixelFormat(device_context_, &pfd);
            if (pixel_format == 0)
            {
                LOG_3D_HUD_ERROR("WGLContext: ChoosePixelFormat failed with error: {}", GetLastError());
                return false;
            }

            if (!SetPixelFormat(device_context_, pixel_format, &pfd))
            {
                LOG_3D_HUD_ERROR("WGLContext: SetPixelFormat failed with error: {}", GetLastError());
                return false;
            }

            LOG_3D_HUD_INFO("WGLContext: Pixel format {} configured successfully", pixel_format);
            return true;
        }

        bool WGLContext::CreateContext() noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::CreateContext");

            rendering_context_ = wglCreateContext(device_context_);
            if (!rendering_context_)
            {
                LOG_3D_HUD_ERROR("WGLContext: wglCreateContext failed with error: {}", GetLastError());
                return false;
            }

            LOG_3D_HUD_INFO("WGLContext: Rendering context created successfully");
            return true;
        }

        void WGLContext::LoadWGLExtensions() noexcept
        {
            HUD_3D_CPU_PROFILE_ZONE("WGLContext::LoadWGLExtensions");

            // Make context current to load extensions
            if (!MakeCurrent())
            {
                LOG_3D_HUD_ERROR("WGLContext: Cannot make context current for extension loading");
                return;
            }

            // Load WGL swap interval extension
            wgl_swap_interval_ext_ = reinterpret_cast<wglSwapIntervalEXT_t>(
                wglGetProcAddress("wglSwapIntervalEXT"));

            // Load WGL ARB pixel format extensions
            wgl_choose_pixel_format_arb_ = reinterpret_cast<wglChoosePixelFormatARB_t>(
                wglGetProcAddress("wglChoosePixelFormatARB"));
            wgl_get_pixel_format_attrib_iv_arb_ = reinterpret_cast<wglGetPixelFormatAttribivARB_t>(
                wglGetProcAddress("wglGetPixelFormatAttribivARB"));

            // Log extension availability
            LOG_3D_HUD_INFO("WGLContext: Extensions loaded - wglSwapIntervalEXT: {}, wglChoosePixelFormatARB: {}, wglGetPixelFormatAttribivARB: {}",
                            wgl_swap_interval_ext_ != nullptr,
                            wgl_choose_pixel_format_arb_ != nullptr,
                            wgl_get_pixel_format_attrib_iv_arb_ != nullptr);
        }

        void *WGLContext::GetProcAddress(const char *function_name) noexcept
        {
            if (!IsValid())
            {
                return nullptr;
            }

            // First try WGL extension function
            void *proc = wglGetProcAddress(function_name);
            if (proc != nullptr)
            {
                return proc;
            }

            // Fallback to standard OpenGL functions from system library
            static HMODULE opengl_module = GetModuleHandleA("opengl32.dll");
            if (opengl_module)
            {
                proc = ::GetProcAddress(opengl_module, function_name);
            }

            return proc;
        }
    }
}