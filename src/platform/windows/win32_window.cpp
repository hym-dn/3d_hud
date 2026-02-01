/**
 * @file win32_window.cpp
 * @brief Windows-specific window implementation
 *
 * @details
 * Implements the Win32Window class using native Win32 API for window creation,
 * event handling, and graphics context management. This file contains the
 * complete implementation of Windows platform window functionality.
 *
 * @par Implementation Overview:
 * The implementation follows a clear initialization sequence:
 * 1. Window class registration
 * 2. Native window creation
 * 3. Graphics context creation based on selected API
 * 4. Graphics context initialization
 *
 * @par Graphics API Support:
 * Currently supports OpenGL via WGLContext. Vulkan and Direct3D support is
 * planned but not yet implemented.
 *
 * @par Error Handling:
 * Uses logging system for error reporting and follows RAII principles for
 * resource management. Failed initialization automatically cleans up resources.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#include <stdexcept>
#include <cstdint>
#include "wgl_context.h"
#include "utils/log/log_manager.h"
#include "win32_window.h"

namespace hud_3d
{
    namespace platform
    {
        // Static member initialization: singleton pointer for WndProc access
        Win32Window *Win32Window::instance_ptr_ = nullptr;

        Win32Window::Win32Window() noexcept
            : WindowBase(),
              hwnd_(nullptr),
              hinstance_(nullptr),
              is_initialized_(false),
              context_{}

        {
            // Get the application instance handle for window creation
            hinstance_ = GetModuleHandle(nullptr);
            if (!hinstance_)
            {
                LOG_3D_HUD_ERROR("Failed to get HINSTANCE.");
            }
        }

        Win32Window::~Win32Window() noexcept
        {
            // Ensure proper cleanup on destruction
            Shutdown();
        }

        bool Win32Window::Initialize(const WindowDesc &desc) noexcept
        {
            // Check for double initialization
            if (is_initialized_)
            {
                LOG_3D_HUD_WARN("Win32Window: Already initialized");
                return false;
            }

            // Store window descriptor using base class method
            // This makes it available to GetWindowDesc() and CreateDefaultGraphicsConfig()
            SetWindowDesc(desc);

            LOG_3D_HUD_INFO("Initializing Win32Window: {} ({}x{}), API={}, External={}",
                            desc.title, desc.width, desc.height,
                            static_cast<int32_t>(desc.api), desc.external_window);

            // Handle external vs internal window creation
            if (desc.external_window)
            {
                // Use externally provided window handle
                if (!desc.native_window)
                {
                    LOG_3D_HUD_ERROR("External window specified but no native_window handle provided");
                    return false;
                }

                hwnd_ = static_cast<HWND>(desc.native_window);

                // Get window dimensions from external window
                RECT client_rect;
                if (GetClientRect(hwnd_, &client_rect))
                {
                    // Update window descriptor with actual client dimensions
                    auto &mutable_desc = const_cast<WindowDesc &>(GetWindowDesc());
                    mutable_desc.width = client_rect.right - client_rect.left;
                    mutable_desc.height = client_rect.bottom - client_rect.top;
                }
                else
                {
                    LOG_3D_HUD_WARN("Failed to get client rect from external window, using descriptor dimensions");
                }

                LOG_3D_HUD_INFO("Using external window handle: {}", reinterpret_cast<uintptr_t>(hwnd_));
            }
            else
            {
                // Create window internally
                // Step 1: Register the window class
                if (!RegisterWindowClass())
                {
                    LOG_3D_HUD_ERROR("Failed to register window class");
                    return false;
                }

                // Step 2: Create the native Win32 window
                if (!CreateNativeWindow(desc.title, desc.width, desc.height))
                {
                    LOG_3D_HUD_ERROR("Failed to create native window");
                    UnregisterWindowClass();
                    return false;
                }
            }

            // Step 3: Create graphics context based on selected API
            // Note: Each graphics API requires its own context implementation:
            // - OpenGL: WGLContext (Windows OpenGL)
            // - Vulkan: VulkanContext (not yet implemented)
            // - Direct3D: D3D11Context/D3D12Context (not yet implemented)
            switch (desc.api)
            {
            case ContextAPI::OpenGL:
                context_ = std::make_unique<WGLContext>();
                break;
            case ContextAPI::Vulkan:
                // TODO: Create Vulkan context when implemented
                // Vulkan uses vkCreateInstance, vkCreateWin32SurfaceKHR, etc.
                LOG_3D_HUD_ERROR("Vulkan context not yet implemented");
                return false;
            case ContextAPI::Direct3D:
                // TODO: Create Direct3D context when implemented
                // Direct3D uses DXGI, D3D11CreateDevice, IDXGISwapChain, etc.
                LOG_3D_HUD_ERROR("Direct3D context not yet implemented");
                return false;
            default:
                LOG_3D_HUD_ERROR("Unsupported graphics API: {}", static_cast<int>(desc.api));
                return false;
            }

            if (!context_)
            {
                LOG_3D_HUD_ERROR("Failed to create graphics context");
                Shutdown();
                return false;
            }

            // Step 4: Create graphics configuration
            // Use custom config if set, otherwise create default from WindowDesc
            auto &graphics_config = const_cast<GraphicsConfig &>(GetGraphicsConfig());
            if (graphics_config.width == 0) // Check if custom config was set
            {
                SetGraphicsConfig(CreateDefaultGraphicsConfig(desc));
            }

            // Update surface handle with actual window handle
            auto &config = const_cast<GraphicsConfig &>(GetGraphicsConfig());
            SurfaceConfig::SurfaceHandle::WindowsHandle win_handle;
            win_handle.window_handle = hwnd_;
            win_handle.device_context = nullptr;
            win_handle.is_window_handle = true;
            config.surface.handle.SetPlatform(win_handle);

            // Step 5: Initialize the graphics context
            if (!context_->Initialize(graphics_config_))
            {
                LOG_3D_HUD_ERROR("Failed to initialize graphics context");
                Shutdown();
                return false;
            }

            // Mark as initialized
            is_initialized_ = true;

            LOG_3D_HUD_INFO("Win32Window initialized successfully");

            return true;
        }

        void Win32Window::Shutdown() noexcept
        {
            // Check if already shut down
            if (!is_initialized_)
            {
                return;
            }

            LOG_3D_HUD_INFO("Shutting down Win32Window.");

            // Step 1: Destroy graphics context first
            if (context_)
            {
                context_->Destroy();
                context_.reset();
            }

            // Step 2: Destroy the native window (only for internally created windows)
            if (hwnd_ && !IsExternalWindow())
            {
                DestroyWindow(hwnd_);
                hwnd_ = nullptr;
            }
            else if (IsExternalWindow())
            {
                // For external windows, just clear the handle but don't destroy the window
                hwnd_ = nullptr;
            }

            // Step 3: Unregister window class (only for internally created windows)
            if (!IsExternalWindow())
            {
                UnregisterWindowClass();
            }

            is_initialized_ = false;
        }

        void Win32Window::PollEvents() noexcept
        {
            // Skip event processing for external windows (application handles events)
            if (!ShouldProcessEvents())
            {
                return;
            }

            MSG msg;
            // Process all pending messages in the window's message queue
            while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
            {
                // Translate virtual-key messages into character messages
                TranslateMessage(&msg);
                // Dispatch message to the window procedure
                DispatchMessage(&msg);
            }
        }

        void Win32Window::SwapBuffers() noexcept
        {
            if (context_ && context_->IsValid())
            {
                // Ignore the return value since window-level swap is fire-and-forget
                (void)context_->SwapBuffers();
            }
        }

        void Win32Window::SetGraphicsConfig(const GraphicsConfig &config) noexcept
        {
            // Prevent setting config after initialization (would require context recreation)
            if (is_initialized_)
            {
                LOG_3D_HUD_WARN("SetGraphicsConfig: Window already initialized, changes will not take effect");
                return;
            }
            // Store custom configuration using base class method
            WindowBase::SetGraphicsConfig(config);
            LOG_3D_HUD_INFO("Custom graphics configuration set");
        }

        GraphicsConfig Win32Window::CreateDefaultGraphicsConfig(const WindowDesc &desc) const noexcept
        {
            GraphicsConfig config;

            // Step 1: Copy basic parameters from WindowDesc
            config.width = desc.width;
            config.height = desc.height;
            config.api = desc.api;
            config.enable_vsync = desc.enable_vsync;

            // Step 2: Configure surface settings
            config.surface.type = SurfaceConfig::SurfaceType::Window; // Windowed surface
            config.surface.double_buffered = true;                    // Enable double buffering
            config.surface.srgb_capable = false;                      // Disable sRGB by default

            // Step 3: Set platform-specific configuration (Windows)
            config.platform.config = WindowsConfig{};

            // Step 4: Configure API-specific settings
            switch (desc.api)
            {
            case ContextAPI::OpenGL:
                // Use modern OpenGL 4.6 Core Profile for best compatibility
                if (config.api_config.IsAPI<APIConfig::OpenGLConfig>())
                {
                    auto &opengl_config = config.api_config.GetAPI<APIConfig::OpenGLConfig>();
                    opengl_config.context.major_version = 4;
                    opengl_config.context.minor_version = 6;
                    opengl_config.context.core_profile = true;
                    opengl_config.context.forward_compatible = true;
                }
                break;
            case ContextAPI::Vulkan:
                // TODO: Set Vulkan-specific defaults when VulkanContext is implemented
                break;
            case ContextAPI::Direct3D:
                // TODO: Set Direct3D-specific defaults when D3DContext is implemented
                break;
            default:
                break;
            }

            LOG_3D_HUD_INFO("Default graphics configuration created for API: {}",
                            static_cast<int>(desc.api));

            return config;
        }

        void Win32Window::Resize(const uint32_t width, const uint32_t height) noexcept
        {
            if (!hwnd_)
            {
                LOG_3D_HUD_WARN("Cannot resize: window handle is null");
                return;
            }

            LOG_3D_HUD_INFO("Resizing window to {}x{}", width, height);

            SetWindowPos(hwnd_, nullptr, 0, 0, width, height,
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

            if (context_)
            {
                context_->Resize(width, height);
            }
        }

        bool Win32Window::RegisterWindowClass() noexcept
        {
            // Initialize window class structure
            WNDCLASSEX wc = {};
            wc.cbSize = sizeof(WNDCLASSEX);

            // Set class style: redraw on resize and allocate unique device context
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

            // Set window procedure and instance handle
            wc.lpfnWndProc = WndProc;
            wc.hInstance = hinstance_;

            // Set default cursor and background
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);

            // Set class name
            wc.lpszClassName = WINDOW_CLASS_NAME;

            // Set default icons
            wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

            // Register the window class with Windows
            if (!RegisterClassEx(&wc))
            {
                LOG_3D_HUD_ERROR("RegisterClassEx failed with error: {}", GetLastError());
                return false;
            }

            return true;
        }

        void Win32Window::UnregisterWindowClass() noexcept
        {
            UnregisterClass(WINDOW_CLASS_NAME, hinstance_);
        }

        bool Win32Window::CreateNativeWindow(const char *title, uint32_t width, uint32_t height) noexcept
        {
            // Step 1: Define desired client area rectangle
            RECT window_rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};

            // Step 2: Set window style with borders and clipping
            DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

            // Step 3: Calculate full window size including borders and title bar
            // This ensures the client area matches the requested dimensions
            AdjustWindowRect(&window_rect, style, FALSE);

            // Step 4: Create the actual Win32 window
            hwnd_ = CreateWindowEx(
                WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,   // Extended style
                WINDOW_CLASS_NAME,                    // Registered class name
                title,                                // Window title
                style,                                // Window style
                CW_USEDEFAULT, CW_USEDEFAULT,         // Position (let Windows decide)
                window_rect.right - window_rect.left, // Full window width
                window_rect.bottom - window_rect.top, // Full window height
                nullptr,                              // No parent window
                nullptr,                              // No menu
                hinstance_,                           // Application instance
                this);                                // Pass this pointer for WM_CREATE

            // Check if window creation failed
            if (!hwnd_)
            {
                LOG_3D_HUD_ERROR("CreateWindowEx failed with error: {}", GetLastError());
                return false;
            }

            // Store instance pointer for WndProc access
            instance_ptr_ = this;

            // Show and activate the window
            ShowWindow(hwnd_, SW_SHOW);
            SetForegroundWindow(hwnd_);
            SetFocus(hwnd_);

            LOG_3D_HUD_INFO("Native window created successfully: {} ({}x{})",
                            title, width, height);

            return true;
        }

        LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
        {
            // Get window instance pointer (try singleton first, then user data)
            Win32Window *window = instance_ptr_;

            // Process Win32 messages
            switch (msg)
            {
            case WM_CREATE:
            {
                // Store instance pointer in window user data for future access
                CREATESTRUCT *create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
                if (create_struct && create_struct->lpCreateParams)
                {
                    SetWindowLongPtr(hwnd, GWLP_USERDATA,
                                     reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
                }
                return 0;
            }

            case WM_DESTROY:
            case WM_CLOSE:
            {
                // Set close flag and post quit message to terminate message loop
                if (window)
                {
                    window->SetShouldClose(true);
                }
                PostQuitMessage(0);
                return 0;
            }

            case WM_SIZE:
            {
                // Handle window resize (ignore minimize events)
                if (window && wParam != SIZE_MINIMIZED)
                {
                    uint32_t width = LOWORD(lParam);  // Extract width from lParam
                    uint32_t height = HIWORD(lParam); // Extract height from lParam
                    window->Resize(width, height);    // Delegate to Resize method
                }
                return 0;
            }

            case WM_KEYDOWN:
            {
                // Handle Escape key to close window
                if (wParam == VK_ESCAPE && window)
                {
                    window->SetShouldClose(true);
                }
                return 0;
            }

            case WM_SETFOCUS:
            {
                // Log focus gain for debugging
                LOG_3D_HUD_INFO("Window gained focus");
                return 0;
            }

            case WM_KILLFOCUS:
            {
                // Log focus loss for debugging
                LOG_3D_HUD_INFO("Window lost focus");
                return 0;
            }

            default:
            {
                // Try to get window pointer from user data if not available
                if (!window)
                {
                    window = reinterpret_cast<Win32Window *>(
                        GetWindowLongPtr(hwnd, GWLP_USERDATA));
                }
            }
            }

            // Pass unhandled messages to default window procedure
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

    } // namespace platform
} // namespace hud_3d
