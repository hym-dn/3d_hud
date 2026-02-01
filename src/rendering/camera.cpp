#include "camera.h"

namespace hud_3d
{
    namespace rendering
    {
        void Camera::SetPosition(const glm::vec3 &pos) noexcept
        {
            position_ = pos;
            view_matrix_dirty_ = true; // Mark cache as dirty
        }

        const glm::vec3 &Camera::GetPosition() const noexcept
        {
            return position_;
        }

        void Camera::SetRotation(const glm::quat &rot) noexcept
        {
            rotation_ = rot;
            view_matrix_dirty_ = true; // Mark cache as dirty
        }

        const glm::quat &Camera::GetRotation() const noexcept
        {
            return rotation_;
        }

        void Camera::LookAt(const glm::vec3 &target) noexcept
        {
            // Calculate the direction vector from camera to target
            glm::vec3 direction = target - position_;

            // Handle edge case: avoid division by zero when target equals position
            float lengthSquared = glm::dot(direction, direction);
            if (lengthSquared < 1e-6f)
            {
                // Target and camera position are the same, don't update rotation
                return;
            }

            // Normalize the direction vector
            direction = glm::normalize(direction);

            // Define world-space up vector (static to avoid reallocation)
            static const glm::vec3 world_up(0.0f, 1.0f, 0.0f);

            // Calculate right vector, ensuring it's not parallel to direction
            glm::vec3 right = glm::cross(world_up, direction);
            float right_length = glm::length(right);

            if (right_length < 1e-6f)
            {
                // Camera is looking straight up or down, use alternative up vector
                right = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), direction);
                right = glm::normalize(right);
            }
            else
            {
                right = right / right_length;
            }

            // Recompute up vector to ensure orthogonality
            glm::vec3 up = glm::cross(direction, right);

            // Construct rotation matrix from basis vectors
            glm::mat3 rotation_matrix;
            rotation_matrix[0] = right;
            rotation_matrix[1] = up;
            rotation_matrix[2] = direction;

            // Convert rotation matrix to quaternion
            rotation_ = glm::quat_cast(rotation_matrix);

            // Mark view matrix cache as dirty since rotation changed
            view_matrix_dirty_ = true;
        }

        const glm::mat4 &Camera::GetViewMatrix() const noexcept
        {
            // Check if cached view matrix is still valid
            if (view_matrix_dirty_)
            {
                // Calculate inverse rotation (transpose of rotation matrix)
                // For orthonormal matrices, transpose equals inverse
                glm::mat4 inv_rot_matrix = glm::transpose(glm::mat4_cast(rotation_));

                // Calculate inverse translation (negative position)
                glm::mat4 translate_matrix = glm::translate(glm::mat4(1.0f), -position_);

                // Cache the view matrix: inverse rotation * inverse translation
                // This transforms world coordinates to view coordinates
                cached_view_matrix_ = inv_rot_matrix * translate_matrix;

                // Mark cache as clean
                view_matrix_dirty_ = false;
            }

            // Return cached view matrix
            return cached_view_matrix_;
        }

    }
}