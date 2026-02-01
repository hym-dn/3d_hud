#include "utils/log/log_manager.h"
#include "view_impl.h"

namespace hud_3d
{
    namespace rendering
    {
        bool View::Initialize(const ViewDesc &desc) noexcept
        {
            desc_ = desc;
            if (desc_.viewport_width == 0 || desc_.viewport_height == 0)
            {
                LOG_3D_HUD_ERROR("Invalid viewport dimensions: {}x{}",
                                 desc_.viewport_width, desc_.viewport_height);
                return false;
            }
            float aspect = static_cast<float>(desc_.viewport_width) /
                           static_cast<float>(desc_.viewport_height);
            SetPerspective(desc_.fov_degrees, aspect,
                           desc_.near_plane, desc_.far_plane);
            LOG_3D_HUD_INFO("View initialized: {}x{} at ({}, {})",
                            desc_.viewport_width, desc_.viewport_height,
                            desc_.viewport_x, desc_.viewport_y);
            return true;
        }

        void View::SetCameraPosition(const glm::vec3 &position) noexcept
        {
            camera_.SetPosition(position);
            view_projection_dirty_ = true; // Mark view-projection cache as dirty
        }

        void View::SetCameraRotation(const glm::quat &rotation) noexcept
        {
            camera_.SetRotation(rotation);
            view_projection_dirty_ = true; // Mark view-projection cache as dirty
        }

        void View::LookAt(const glm::vec3 &target) noexcept
        {
            camera_.LookAt(target);
            view_projection_dirty_ = true; // Mark view-projection cache as dirty
        }

        void View::SetPerspective(const float fov, const float aspect, const float near_plane, const float far_plane) noexcept
        {
            projection_matrix_ = glm::perspective(glm::radians(fov), aspect, near_plane, far_plane);
            view_projection_dirty_ = true; // Mark view-projection cache as dirty
        }

        void View::SetViewport(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h) noexcept
        {
            desc_.viewport_x = x;
            desc_.viewport_y = y;
            desc_.viewport_width = w;
            desc_.viewport_height = h;
            float aspect = static_cast<float>(w) / static_cast<float>(h);
            SetPerspective(desc_.fov_degrees, aspect, desc_.near_plane, desc_.far_plane);
            // Note: SetPerspective already marks view_projection_dirty_
        }

        const glm::mat4 &View::GetViewMatrix() const noexcept
        {
            return camera_.GetViewMatrix();
        }

        const glm::mat4 &View::GetProjectionMatrix() const noexcept
        {
            return projection_matrix_;
        }

        const glm::mat4 &View::GetViewProjectionMatrix() const noexcept
        {
            // Check if cached view-projection matrix is still valid
            if (view_projection_dirty_)
            {
                // Calculate view-projection matrix: projection * view
                view_projection_matrix_ = projection_matrix_ * camera_.GetViewMatrix();
                // Mark cache as clean
                view_projection_dirty_ = false;
            }
            return view_projection_matrix_;
        }

        void View::Update(const float delta_time) noexcept
        {
            (void)delta_time;
        }

    } // namespace rendering
} // namespace hud3d
