#include "window.h"
#include "utils/log/log_manager.h"

namespace hud3d {

uint32_t IWindow::AddView(IView* view) {
    if (!view) {
        LOG_3D_HUD_ERROR("Attempt to add null view");
        return 0xFFFFFFFF;
    }

    // 查找空位或添加到末尾
    for (size_t i = 0; i < views_.size(); ++i) {
        if (!views_[i]) {
            views_[i].reset(view);
            return static_cast<uint32_t>(i);
        }
    }

    views_.emplace_back(view);
    return static_cast<uint32_t>(views_.size() - 1);
}

void IWindow::RemoveView(uint32_t view_id) {
    if (view_id >= views_.size()) {
        LOG_3D_HUD_WARN("Invalid view ID: {}", view_id);
        return;
    }

    if (!views_[view_id]) {
        LOG_3D_HUD_WARN("View {} already removed", view_id);
        return;
    }

    LOG_3D_HUD_INFO("Removing view {}", view_id);
    views_[view_id].reset();
}

IView* IWindow::GetView(uint32_t view_id) {
    if (view_id >= views_.size()) {
        LOG_3D_HUD_WARN("Invalid view ID: {}", view_id);
        return nullptr;
    }
    return views_[view_id].get();
}

} // namespace hud3d
