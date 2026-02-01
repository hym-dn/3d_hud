/**
 * @file window_base.cpp
 * @brief Platform-agnostic window base class implementation
 *
 * @details
 * Implements the platform-agnostic methods of WindowBase, including view
 * management and state management functionality that does not require
 * platform-specific code.
 *
 * @par Implementation Details:
 * This file provides the implementation for view management methods
 * (AddView, RemoveView, GetView, GetViews) which are common across all
 * platforms. Platform-specific functionality is implemented in derived
 * classes like Win32Window.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-10
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#include "utils/log/log_manager.h"
#include "rendering/view.h"
#include "window_base.h"

namespace hud_3d
{
    namespace platform
    {
        uint32_t WindowBase::AddView(std::unique_ptr<rendering::IView> view) noexcept
        {
            if (!view)
            {
                LOG_3D_HUD_ERROR("Attempt to add null view");
                return 0xFFFFFFFF;
            }
            uint32_t view_id = static_cast<uint32_t>(views_.size());
            views_.push_back(std::move(view));
            LOG_3D_HUD_INFO("View added successfully with ID: {}", view_id);
            return view_id;
        }

        void WindowBase::RemoveView(const uint32_t view_id) noexcept
        {
            if (view_id >= views_.size())
            {
                LOG_3D_HUD_WARN("Invalid view ID: {}", view_id);
                return;
            }

            views_.erase(views_.begin() + view_id);
            LOG_3D_HUD_INFO("View removed successfully: {}", view_id);
        }

        rendering::IView *WindowBase::GetView(const uint32_t view_id) noexcept
        {
            if (view_id >= views_.size())
            {
                return nullptr;
            }
            return views_[view_id].get();
        }

        const std::vector<std::unique_ptr<rendering::IView>> &WindowBase::GetViews() const noexcept
        {
            return views_;
        }

        bool WindowBase::ShouldClose() const noexcept
        {
            return should_close_.load();
        }

        void WindowBase::BeginFrame() noexcept
        {
            // Default: no-op
        }

        void WindowBase::EndFrame() noexcept
        {
            // Default: no-op
        }

        uint32_t WindowBase::GetWindowId() const noexcept
        {
            return window_id_;
        }

        void WindowBase::SetWindowId(uint32_t id) noexcept
        {
            window_id_ = id;
        }

        void WindowBase::SetShouldClose(bool should_close) noexcept
        {
            should_close_.store(should_close);
        }

        void WindowBase::SetWindowDesc(const WindowDesc &desc) noexcept
        {
            window_desc_ = desc;
        }

        const WindowDesc &WindowBase::GetWindowDesc() const noexcept
        {
            return window_desc_;
        }

        const GraphicsConfig &WindowBase::GetGraphicsConfig() const noexcept
        {
            return graphics_config_;
        }

        void WindowBase::SetGraphicsConfig(const GraphicsConfig &config) noexcept
        {
            graphics_config_ = config;
        }

        bool WindowBase::IsExternalWindow() const noexcept
        {
            return window_desc_.external_window;
        }

        bool WindowBase::ShouldProcessEvents() const noexcept
        {
            return !window_desc_.external_window;
        }

    } // namespace platform
} // namespace hud_3d
