/**
 * @file window_manager_impl.cpp
 * @brief Window manager implementation for creating and managing multiple windows
 *
 * @details
 * Implements the WindowManager class methods for window lifecycle management,
 * event processing, and state queries. This file contains platform-specific
 * window creation logic based on compile-time platform detection.
 *
 * @par Platform Detection:
 * The CreateWindow method uses preprocessor directives to instantiate the
 * appropriate platform-specific window implementation at compile time.
 *
 * @par Error Handling:
 * Uses the Result<T> type for error propagation, providing detailed error
 * codes and messages for debugging.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#include "window_manager_impl.h"
#include "utils/log/log_manager.h"

// Platform-specific window implementations
#ifdef __3D_HUD_PLATFORM_WINDOWS__
#include "windows/win32_window.h"
// #elif defined(__3D_HUD_PLATFORM_LINUX__)
// #include "linux/x11_window.h"
// #elif defined(__3D_HUD_PLATFORM_ANDROID__)
// #include "android/android_window.h"
// #elif defined(__3D_HUD_PLATFORM_QNX__)
// #include "qnx/screen_window.h"
#endif

namespace hud_3d
{
    namespace platform
    {
        IWindowManager &IWindowManager::GetInstance() noexcept
        {
            static WindowManager instance;
            return instance;
        }

        WindowManager::WindowManager() noexcept
            : IWindowManager(),
              windows_{},
              window_active_{false},
              window_count_(0),
              next_window_id_(0)

        {
            LOG_3D_HUD_INFO("WindowManager initialized with capacity: {}", MAX_WINDOWS);
        }

        WindowManager::~WindowManager() noexcept
        {
            // Destroy all active windows
            for (uint32_t i = 0; i < MAX_WINDOWS; ++i)
            {
                if (window_active_[i])
                {
                    DestroyWindow(i + 1);
                }
            }
            LOG_3D_HUD_INFO("WindowManager shut down");
        }

        uint32_t WindowManager::CreateNewWindow(const WindowDesc &desc) noexcept
        {
            if (window_count_ >= MAX_WINDOWS)
            {
                LOG_3D_HUD_ERROR("Maximum window count reached: {}", MAX_WINDOWS);
                return INVALID_WINDOW_ID; // Invalid window ID
            }

            const uint32_t raw_id = next_window_id_.fetch_add(1, std::memory_order_relaxed);
            const uint32_t window_id = raw_id + 1; // Window IDs start from 1

            // Validate window ID using the interface function
            if (!IsValidWindowId(window_id))
            {
                LOG_3D_HUD_ERROR("Window ID out of valid range: {}", window_id);
                return INVALID_WINDOW_ID;
            }

            const uint32_t index = raw_id; // Convert to 0-based index

            LOG_3D_HUD_INFO("Creating window {} ({}x{}), API={}",
                            static_cast<uint32_t>(window_id), desc.width, desc.height,
                            static_cast<int>(desc.api));

// Create platform-specific window implementation
#ifdef __3D_HUD_PLATFORM_WINDOWS__
            windows_[index] = std::make_unique<Win32Window>();
// #elif defined(__3D_HUD_PLATFORM_LINUX__)
//             windows_[index] = std::make_unique<X11Window>();
// #elif defined(__3D_HUD_PLATFORM_ANDROID__)
//             if (!desc.native_window)
//             {
//                 LOG_3D_HUD_ERROR("Android requires native_window in WindowDesc");
//                 return WindowId::INVALID;
//             }
//             windows_[index] = std::make_unique<AndroidWindow>(desc.native_window);
// #elif defined(__3D_HUD_PLATFORM_QNX__)
//             windows_[index] = std::make_unique<QNXWindow>();
#else
            LOG_3D_HUD_ERROR("Unsupported platform");
            return INVALID_WINDOW_ID;
#endif

            if (!windows_[index])
            {
                LOG_3D_HUD_ERROR("Failed to create window instance");
                return INVALID_WINDOW_ID;
            }

            // Initialize the window
            if (!windows_[index]->Initialize(desc))
            {
                LOG_3D_HUD_ERROR("Failed to initialize window {}", static_cast<uint32_t>(window_id));
                windows_[index].reset();
                return INVALID_WINDOW_ID;
            }

            // Set window ID
            windows_[index]->SetWindowId(window_id);

            window_active_[index] = true;
            window_count_++;
            LOG_3D_HUD_INFO("Window {} created successfully", static_cast<uint32_t>(window_id));

            return window_id;
        }

        void WindowManager::DestroyWindow(const uint32_t window_id) noexcept
        {
            if (!IsValidWindowId(window_id))
            {
                LOG_3D_HUD_WARN("Attempt to destroy invalid window: {}", window_id);
                return;
            }

            const uint32_t index = window_id - 1; // Convert to 0-based index

            if (!window_active_[index])
            {
                LOG_3D_HUD_WARN("Attempt to destroy inactive window: {}", window_id);
                return;
            }

            LOG_3D_HUD_INFO("Destroying window {}", window_id);

            if (windows_[index])
            {
                windows_[index]->Shutdown();
                windows_[index].reset();
            }

            window_active_[index] = false;
            window_count_--;
        }

        IWindow *WindowManager::GetWindow(const uint32_t window_id) noexcept
        {
            if (!IsValidWindowId(window_id))
            {
                LOG_3D_HUD_WARN("Attempt to get invalid window: {}", window_id);
                return nullptr;
            }

            const uint32_t index = window_id - 1; // Convert to 0-based index

            if (!window_active_[index])
            {
                LOG_3D_HUD_WARN("Attempt to get inactive window: {}", window_id);
                return nullptr;
            }

            return windows_[index].get();
        }

        const IWindow *WindowManager::GetWindow(const uint32_t window_id) const noexcept
        {
            return const_cast<WindowManager *>(this)->GetWindow(window_id);
        }

        void WindowManager::PollEvents() noexcept
        {
            for (uint32_t i = 0; i < MAX_WINDOWS; ++i)
            {
                if (window_active_[i] && windows_[i])
                {
                    windows_[i]->PollEvents();
                }
            }
        }

        uint32_t WindowManager::GetWindowCount() const noexcept
        {
            return window_count_;
        }

        bool WindowManager::ShouldClose() const noexcept
        {
            for (uint32_t i = 0; i < MAX_WINDOWS; ++i)
            {
                if (window_active_[i] && windows_[i] && windows_[i]->ShouldClose())
                {
                    return true;
                }
            }
            return false;
        }

    } // namespace platform
} // namespace hud_3d
