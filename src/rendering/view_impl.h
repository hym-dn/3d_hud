/** @file view_impl.h
 * @brief Concrete View implementation for the 3D HUD rendering engine
 *
 * @author Yameng.He
 * @date 2025-01-16
 *
 * This file defines the View class, which is the concrete implementation of the
 * IView interface. The View class encapsulates all rendering configuration including
 * viewport settings, projection matrices, camera transformations, and per-frame
 * update logic for a specific rendering perspective.
 *
 * @par Implementation Details:
 * The View class follows the principle of composition over inheritance by containing
 * a Camera object for transformation management and maintaining its own projection
 * state. It provides matrix caching for performance optimization and implements
 * all pure virtual methods from the IView interface.
 */

#pragma once

#include "camera.h"
#include "rendering/view.h"

namespace hud_3d
{
    namespace rendering
    {
        /**
         * @class View
         * @brief Concrete implementation of the IView interface
         *
         * View provides a complete implementation of the view system for the 3D HUD
         * rendering engine. It manages viewport configuration, projection matrices,
         * camera transformations, and rendering state. The class is designed to be
         * lightweight yet fully functional, suitable for multiple views per window.
         *
         * @par Key Features:
         * - Complete IView interface implementation
         * - Internal Camera object for transformation management
         * - Projection matrix caching for performance
         * - ViewDesc parameter encapsulation
         * - Frame update support for animations
         *
         * @par Memory Management:
         * View objects are typically created on the heap and managed through
         * std::unique_ptr or std::shared_ptr. They are move-only to prevent
         * object slicing and ensure proper polymorphic behavior.
         *
         * @par Thread Safety:
         * This implementation is not thread-safe. All methods should be called
         * from the main rendering thread. External synchronization is required
         * for multi-threaded access.
         */
        class View : public IView
        {
        public:
            /**
             * @name Constructor and Destructor
             * @brief Default construction and virtual destruction
             *
             * Default constructor creates a view with default-initialized members.
             * The view is not ready for use until Initialize() is called with a
             * valid ViewDesc. The destructor properly cleans up all resources.
             */
            /**@{*/
            View() noexcept = default;
            virtual ~View() noexcept override = default;
            /**@}*/

        public:
            /**
             * @name Lifecycle Management
             * @brief Initialize and configure the view
             *
             * These methods manage the view's lifecycle from initialization through
             * parameter retrieval. Initialize() must be called before any rendering
             * operations, and GetDesc() provides access to the current configuration.
             */
            /**@{*/

            /**
             * @brief Initializes the view with configuration parameters
             * @param[in] desc View descriptor containing viewport, projection, and clear settings
             * @return true if initialization succeeded, false otherwise
             *
             * Validates the descriptor parameters, creates projection matrices, and
             * prepares internal resources. If initialization fails, the view remains
             * in an invalid state and subsequent method calls may produce undefined
             * behavior. The camera is reset to its default position and orientation.
             *
             * @note This method should be called exactly once after construction.
             * Re-initialization requires destroying and recreating the view object.
             */
            [[nodiscard]] virtual bool Initialize(const ViewDesc &desc) noexcept override;

            /**
             * @brief Retrieves the view descriptor
             * @return Const reference to the current view descriptor
             *
             * Provides read-only access to the view's configuration parameters.
             * This method is useful for inspecting current viewport dimensions,
             * projection settings, or clear behavior without exposing mutable access.
             *
             * @par Performance:
             * Returns a const reference to avoid copying the descriptor structure.
             * The reference remains valid for the lifetime of the View object.
             */
            virtual const ViewDesc &GetDesc() const noexcept override;

            /**@}*/

            /**
             * @name Camera Integration
             * @brief Connect camera transformations to the view
             *
             * These methods delegate camera operations to the internal Camera object.
             * They provide a convenient interface for manipulating the view's
             * transformation without direct camera access.
             */
            /**@{*/

            /**
             * @brief Sets the camera position for view matrix generation
             * @param[in] position Camera position in world space coordinates
             *
             * Delegates to the internal Camera object's SetPosition method.
             * This updates the eye point from which the scene is rendered.
             *
             * @see Camera::SetPosition() for detailed behavior
             */
            virtual void SetCameraPosition(const glm::vec3 &position) noexcept override;

            /**
             * @brief Sets the camera rotation for view matrix generation
             * @param[in] rotation Camera orientation as a unit quaternion
             *
             * Delegates to the internal Camera object's SetRotation method.
             * The quaternion must be normalized for correct results.
             *
             * @see Camera::SetRotation() for detailed behavior
             */
            virtual void SetCameraRotation(const glm::quat &rotation) noexcept override;

            /**
             * @brief Orients the camera to look at a specific target point
             * @param[in] target World-space position to look at
             *
             * Delegates to the internal Camera object's LookAt method.
             * Automatically calculates rotation to point toward the target.
             *
             * @see Camera::LookAt() for detailed behavior
             */
            virtual void LookAt(const glm::vec3 &target) noexcept override;

            /**@}*/

            /**
             * @name Projection Configuration
             * @brief Control perspective projection parameters
             *
             * These methods manage the projection transformation applied to vertices
             * during rendering. They update the internal projection matrix and viewport
             * configuration.
             */
            /**@{*/

            /**
             * @brief Configures perspective projection parameters
             * @param[in] fov Vertical field of view in radians
             * @param[in] aspect Aspect ratio (width / height)
             * @param[in] near_plane Distance to near clipping plane
             * @param[in] far_plane Distance to far clipping plane
             *
             * Creates a perspective projection matrix and stores it internally.
             * This matrix transforms view-space coordinates to clip space using
             * perspective division. All parameters are validated before use.
             *
             * @par Implementation Note:
             * The projection matrix is cached and recalculated only when parameters
             * change. This method marks the projection cache as dirty.
             */
            virtual void SetPerspective(const float fov, const float aspect,
                                        const float near_plane, const float far_plane) noexcept override;

            /**
             * @brief Sets the viewport dimensions
             * @param[in] x Viewport top-left X coordinate
             * @param[in] y Viewport top-left Y coordinate
             * @param[in] w Viewport width in pixels
             * @param[in] h Viewport height in pixels
             *
             * Updates the viewport rectangle stored in the descriptor.
             * This affects where the rendered output appears in the window
             * and influences the aspect ratio calculation for projection.
             *
             * @note Does not automatically update the projection matrix.
             * Call SetPerspective() after changing viewport to maintain correct aspect ratio.
             */
            virtual void SetViewport(const uint32_t x, const uint32_t y,
                                     const uint32_t w, const uint32_t h) noexcept override;

            /**@}*/

            /**
             * @name Matrix Generation
             * @brief Generate transformation matrices for rendering
             *
             * These methods compute and return the mathematical transformations
             * required by the rendering pipeline. Matrices are cached internally
             * and only recalculated when underlying parameters change.
             */
            /**@{*/

            /**
             * @brief Generates the view transformation matrix
             * @return 4x4 view matrix transforming world to camera space
             *
             * Delegates to the internal Camera object's GetViewMatrix method.
             * Returns the cached view matrix representing the current camera
             * transformation.
             *
             * @see Camera::GetViewMatrix() for detailed implementation
             */
            virtual const glm::mat4 &GetViewMatrix() const noexcept override;

            /**
             * @brief Generates the projection transformation matrix
             * @return 4x4 projection matrix transforming camera to clip space
             *
             * Returns the internally cached projection matrix. This matrix is
             * recalculated only when SetPerspective() is called with different
             * parameters.
             *
             * @par Performance:
             * This method returns a cached value and has O(1) complexity.
             * No matrix calculation is performed during the call.
             */
            virtual const glm::mat4 &GetProjectionMatrix() const noexcept override;

            /**
             * @brief Generates the combined view-projection matrix
             * @return 4x4 view-projection matrix (projection * view)
             *
             * Returns the product of projection and view matrices. This combined
             * matrix transforms vertices directly from world space to clip space,
             * which is more efficient than applying view and projection separately.
             *
             * @par Implementation:
             * This method calculates the product on-demand but may cache the result
             * for subsequent calls if view or projection parameters haven't changed.
             */
            virtual const glm::mat4 &GetViewProjectionMatrix() const noexcept override;

            /**@}*/

            /**
             * @name Update and Animation
             * @brief Per-frame update logic
             */
            /**@{*/

            /**
             * @brief Updates the view state for the current frame
             * @param[in] delta_time Time elapsed since last frame in seconds
             *
             * Called once per frame to update view-specific logic and animations.
             * The delta_time parameter allows for framerate-independent updates.
             * This method delegates to the internal Camera object's Update method.
             *
             * @par Current Implementation:
             * Currently a no-op placeholder. Override in derived classes to implement
             * custom per-frame behavior such as camera animations, smooth transitions,
             * or time-dependent effects.
             */
            virtual void Update(const float delta_time) noexcept override;

            /**@}*/

        private:
            /**
             * @name Member Variables
             * @brief Internal state and configuration
             *
             * These member variables store the view's configuration and state.
             * They are initialized to default values and updated through the
             * public interface methods.
             */
            /**@{*/

            /**
             * @brief View descriptor containing configuration parameters
             *
             * Stores all viewport, projection, and clear settings. This structure
             * is populated during Initialize() and can be accessed via GetDesc().
             */
            ViewDesc desc_{};

            /**
             * @brief Internal camera object for transformation management
             *
             * The Camera object handles all view matrix calculations and
             * provides transformation services to the view. This composition
             * pattern separates concerns between view configuration and
             * camera manipulation.
             */
            Camera camera_{};

            /**
             * @brief Cached projection matrix
             *
             * Stores the projection matrix calculated from perspective parameters.
             * This matrix is recalculated only when SetPerspective() is called
             * with different parameters, providing performance optimization.
             */
            glm::mat4 projection_matrix_{1.0f};

            /**
             * @brief Cached view-projection matrix
             *
             * Stores the combined view-projection matrix (projection * view).
             * This matrix is recalculated when either view or projection matrices
             * change, providing performance optimization for rendering loops.
             */
            mutable glm::mat4 view_projection_matrix_{1.0f};

            /**
             * @brief View-projection matrix dirty flag
             *
             * Indicates whether the view-projection matrix cache is valid.
             * Set to true when view or projection parameters change, forcing
             * recalculation on next access.
             */
            mutable bool view_projection_dirty_{true};

            /**@}*/
        };
    }
}