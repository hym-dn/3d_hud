/** @file camera.h
 * @brief Camera interface definition for the 3D HUD rendering engine
 *
 * @author Yameng.He
 * @date 2025-01-16
 *
 * This file defines the ICamera interface class, which provides an abstract
 * contract for camera implementations across different rendering backends.
 * The interface supports camera manipulation, transformation, and view matrix
 * generation for 3D graphics rendering pipelines.
 */

#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace hud_3d
{
    namespace rendering
    {
        /**
         * @class ICamera
         * @brief Abstract camera interface for 3D rendering
         *
         * ICamera defines a polymorphic interface for camera objects used in the
         * 3D HUD rendering engine. It provides methods for camera positioning,
         * rotation control, and view matrix generation. Concrete implementations
         * (e.g., PerspectiveCamera, OrthographicCamera) must inherit from this
         * interface and implement all pure virtual methods.
         *
         * @note This class uses move-only semantics to prevent object slicing
         * and ensure proper polymorphic behavior. Copy operations are explicitly
         * deleted.
         *
         * @par Design Principles:
         * - Camera transformations are represented using glm types for compatibility
         * - Position and rotation are stored and manipulated independently
         * - View matrix generation is deferred to implementations
         * - All methods are noexcept for performance-critical rendering code
         *
         * @par Example Usage:
         * @code
         * // Create a concrete camera implementation
         * auto camera = std::make_unique<PerspectiveCamera>();
         *
         * // Configure camera position and target
         * camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
         * camera->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
         *
         * // Retrieve view matrix for rendering
         * glm::mat4 viewMatrix = camera->GetViewMatrix();
         * shader->SetUniformMat4("view", viewMatrix);
         * @endcode
         */
        class Camera final
        {
        public:
            /**
             * @name Constructor and Destructor
             * @brief Default construction and virtual destruction
             *
             * Default constructor and virtual destructor are explicitly defaulted.
             * Derived classes must implement their own constructors with specific
             * parameters (e.g., field of view, aspect ratio, near/far planes).
             */
            /**@{*/
            Camera() = default;
            ~Camera() = default;
            /**@}*/

        public:
            /**
             * @name Camera Transformation Methods
             * @brief Control camera position and orientation in 3D space
             *
             * These methods allow manipulation of the camera's spatial properties,
             * including position (world-space coordinates) and rotation (quaternion).
             * All transformations are performed in double-precision and converted
             * to single-precision matrices for rendering.
             */
            /**@{*/
            /**
             * @brief Sets the camera's position in world space
             * @param[in] pos The new camera position as a 3D vector
             */
            void SetPosition(const glm::vec3 &pos) noexcept;

            /**
             * @brief Retrieves the current camera position
             * @return Reference to the camera's position vector
             */
            const glm::vec3 &GetPosition() const noexcept;

            /**
             * @brief Sets the camera's rotation using a quaternion
             * @param[in] rot The new rotation as a unit quaternion
             * @note The quaternion must be normalized; results are undefined for non-unit quaternions
             */
            void SetRotation(const glm::quat &rot) noexcept;

            /**
             * @brief Retrieves the current camera rotation
             * @return Reference to the camera's rotation quaternion
             */
            const glm::quat &GetRotation() const noexcept;

            /**
             * @brief Orients the camera to look at a specific target point
             * @param[in] target The world-space position to look at
             *
             * Automatically calculates and updates the camera's rotation quaternion
             * to point from the current position toward the target. This is a convenience
             * method that computes the forward vector and constructs an appropriate
             * rotation quaternion.
             *
             * @par Implementation Note:
             * Typically implemented as:
             * @code
             * glm::vec3 forward = glm::normalize(target - position);
             * glm::quat rotation = glm::rotation(glm::vec3(0,0,-1), forward);
             * SetRotation(rotation);
             * @endcode
             */
            void LookAt(const glm::vec3 &target) noexcept;
            /**@}*/

            /**
             * @name View Matrix Generation
             * @brief Generate transformation matrices for rendering
             *
             * These methods compute the mathematical transformations required by
             * the rendering pipeline. View matrices transform world coordinates
             * to camera/view coordinates, which is essential for 3D rendering.
             */
            /**@{*/

            /**
             * @brief Generates and returns the view transformation matrix
             * @return The 4x4 view matrix transforming world to view space
             *
             * This matrix is used by the rendering pipeline to transform vertices
             * from world coordinates to camera-relative coordinates. It combines
             * translation (inverse of position) and rotation (inverse of orientation).
             *
             * @par Mathematical Definition:
             * ViewMatrix = Translate(-position) * Rotate(rotation)
             *
             * @par Performance:
             * This method may cache results and should be called once per frame
             * rather than for each object.
             */
            const glm::mat4 &GetViewMatrix() const noexcept;
            /**@}*/

        private:
            /**
             * @name Copy and Move Control
             * @brief Prevent object slicing and ensure polymorphic behavior
             *
             * All copy and move operations are explicitly deleted to prevent
             * object slicing and maintain proper polymorphic behavior. Camera
             * objects should always be managed through smart pointers (std::unique_ptr
             * or std::shared_ptr) rather than by value.
             *
             * @note This follows the C++ Core Guidelines for polymorphic base classes:
             * "A polymorphic class should suppress copying"
             *
             * @see <a href="http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-copy-virtual">
             * C++ Core Guidelines: Rule of Five for polymorphic classes
             * </a>
             */
            /**@{*/
            Camera(Camera &&) = delete;
            Camera &operator=(Camera &&) = delete;
            Camera(const Camera &) = delete;
            Camera &operator=(const Camera &) = delete;
            /**@}*/

        private:
            // Camera spatial properties
            glm::vec3 position_{0.0f, 0.0f, 5.0f};
            glm::quat rotation_{1.0f, 0.0f, 0.0f, 0.0f};
            // View matrix cache for performance optimization
            mutable glm::mat4 cached_view_matrix_{1.0f};
            mutable bool view_matrix_dirty_{true};
        };
    }
}