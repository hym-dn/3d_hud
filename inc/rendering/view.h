/** @file view.h
 * @brief View interface and descriptor definitions for the 3D HUD rendering engine
 *
 * @author Yameng.He
 * @date 2025-01-16
 *
 * This file defines the IView interface class and ViewDesc structure, which provide
 * the foundation for view management in the 3D HUD rendering engine. A view represents
 * a rendering configuration including viewport settings, projection parameters, and
 * associated camera transformations for a specific rendering perspective.
 *
 * @par Architecture Overview:
 * The view system separates projection and viewport configuration (View) from
 * camera transformation (Camera), following the principle of single responsibility.
 * This allows multiple views to share the same camera position or a single view
 * to be rendered from different camera perspectives.
 */

#pragma once

#include <cstdint>
#include "glm/glm.hpp"

namespace hud_3d
{
    namespace rendering
    {
        /**
         * @struct ViewDesc
         * @brief View descriptor structure containing rendering configuration parameters
         *
         * ViewDesc is a pure data structure that encapsulates all configuration
         * parameters required to set up a rendering view. It includes viewport
         * dimensions, projection settings, clear behavior, and rendering priority.
         * This structure is passed to IView::Initialize() and can be retrieved
         * via IView::GetDesc() for inspection or modification.
         *
         * @par Default Configuration:
         * The structure provides sensible defaults for a standard 1280x720 viewport
         * with 60-degree field of view, depth buffering enabled, and clear color
         * set to opaque black. These defaults can be overridden during initialization
         * or modified after creation through the IView interface.
         *
         * @par Performance Considerations:
         * This structure is designed to be lightweight and copyable. It is typically
         * created on the stack and passed by const reference to minimize allocations
         * and maximize cache efficiency.
         */
        struct ViewDesc
        {
            /**
             * @name Viewport Configuration
             * @brief Screen-space viewport dimensions and position
             *
             * These parameters define the viewport rectangle in screen coordinates,
             * specifying where the rendered output will appear in the window.
             */
            /**@{*/
            uint32_t viewport_x = 0;        ///< Viewport top-left X coordinate
            uint32_t viewport_y = 0;        ///< Viewport top-left Y coordinate
            uint32_t viewport_width = 1280; ///< Viewport width in pixels
            uint32_t viewport_height = 720; ///< Viewport height in pixels
            /**@}*/

            /**
             * @name Projection Configuration
             * @brief Perspective projection parameters
             *
             * These parameters define the perspective projection transformation,
             * controlling field of view and depth range for 3D rendering.
             */
            /**@{*/
            float fov_degrees = 60.0f; ///< Vertical field of view in degrees
            float near_plane = 0.1f;   ///< Near clipping plane distance
            float far_plane = 1000.0f; ///< Far clipping plane distance
            /**@}*/

            uint32_t render_priority = 0; ///< Rendering order priority (lower values render first)

            /**
             * @name Clear Configuration
             * @brief Framebuffer clearing behavior
             *
             * These parameters control whether and how the color and depth buffers
             * are cleared before rendering the view.
             */
            /**@{*/
            bool clear_color_enabled = true; ///< Enable color buffer clearing
            bool clear_depth_enabled = true; ///< Enable depth buffer clearing
            float clear_color_red = 0.0f;    ///< Clear color red component (0.0 to 1.0)
            float clear_color_green = 0.0f;  ///< Clear color green component (0.0 to 1.0)
            float clear_color_blue = 0.0f;   ///< Clear color blue component (0.0 to 1.0)
            float clear_color_alpha = 1.0f;  ///< Clear color alpha component (0.0 to 1.0)
            /**@}*/
        };

        /**
         * @class IView
         * @brief Abstract view interface for 3D rendering configuration
         *
         * IView defines a polymorphic interface for view objects that encapsulate
         * rendering configuration including viewport settings, projection matrices,
         * and associated camera transformations. A view represents a specific
         * rendering perspective within a window and can be combined with cameras
         * to define complete rendering setups.
         *
         * @par Design Philosophy:
         * The view system separates projection and viewport configuration from
         * camera transformations, enabling flexible rendering setups such as:
         * - Multiple views sharing a single camera
         * - Single view rendered from multiple camera perspectives
         * - Split-screen rendering with different projections
         *
         * @par Thread Safety:
         * This interface is not thread-safe by default. External synchronization
         * is required if methods are called concurrently from multiple threads.
         * Typical usage patterns involve setting up views on the main thread
         * and only calling const methods during rendering.
         */
        class IView
        {
        public:
            /**
             * @name Constructor and Destructor
             * @brief Default construction and virtual destruction
             *
             * Default constructor and virtual destructor are explicitly defaulted.
             * Derived classes must implement their own constructors and initialization logic.
             */
            /**@{*/
            IView() noexcept = default;
            virtual ~IView() noexcept = default;
            /**@}*/

        public:
            /**
             * @name Lifecycle Management
             * @brief Initialize and configure the view
             */
            /**@{*/
            /**
             * @brief Initializes the view with configuration parameters
             * @param[in] desc View descriptor containing viewport, projection, and clear settings
             * @return true if initialization succeeded, false otherwise
             *
             * This method must be called after construction and before any rendering operations.
             * It validates the descriptor parameters, creates projection matrices, and prepares
             * internal resources. If initialization fails, the view remains in an invalid state
             * and subsequent method calls may produce undefined behavior.
             *
             * @note The descriptor is typically created on the stack and passed by const reference
             * to avoid unnecessary copying and allocation overhead.
             */
            [[nodiscard]] virtual bool Initialize(const ViewDesc &desc) noexcept = 0;

            /**
             * @brief Retrieves the view descriptor
             * @return Const reference to the current view descriptor
             *
             * Provides read-only access to the view's configuration parameters.
             * This method is useful for inspecting current viewport dimensions,
             * projection settings, or clear behavior.
             */
            virtual const ViewDesc &GetDesc() const noexcept = 0;
            /**@}*/

            /**
             * @name Camera Integration
             * @brief Connect camera transformations to the view
             *
             * These methods allow the view to be associated with a camera's spatial
             * transformations. The view uses these transformations when generating
             * view and view-projection matrices for rendering.
             */
            /**@{*/
            /**
             * @brief Sets the camera position for view matrix generation
             * @param[in] position Camera position in world space coordinates
             *
             * Updates the camera position used when computing view transformations.
             * This position represents the eye point in 3D space from which the scene
             * is being rendered.
             */
            virtual void SetCameraPosition(const glm::vec3 &position) noexcept = 0;

            /**
             * @brief Sets the camera rotation for view matrix generation
             * @param[in] rotation Camera orientation as a unit quaternion
             *
             * Updates the camera rotation using quaternion representation.
             * The quaternion must be normalized for correct results. This rotation
             * defines the camera's orientation in world space.
             */
            virtual void SetCameraRotation(const glm::quat &rotation) noexcept = 0;

            /**
             * @brief Orients the camera to look at a specific target point
             * @param[in] target World-space position to look at
             *
             * Convenience method that automatically calculates and sets the camera
             * rotation to point from the current position toward the target.
             * This is equivalent to calling SetCameraRotation() with a computed
             * look-at quaternion.
             */
            virtual void LookAt(const glm::vec3 &target) noexcept = 0;
            /**@}*/

            /**
             * @name Projection Configuration
             * @brief Control perspective projection parameters
             *
             * These methods manage the projection transformation applied to vertices
             * during rendering, controlling field of view, aspect ratio, and depth range.
             */
            /**@{*/
            /**
             * @brief Configures perspective projection parameters
             * @param[in] fov Vertical field of view in radians
             * @param[in] aspect Aspect ratio (width / height)
             * @param[in] near_plane Distance to near clipping plane
             * @param[in] far_plane Distance to far clipping plane
             *
             * Creates a perspective projection matrix with the specified parameters.
             * This matrix transforms view-space coordinates to clip space and defines
             * the perspective division for 3D rendering. All parameters must be positive
             * and near_plane must be less than far_plane.
             *
             * @note This method may trigger rebuilding of internal cached projection matrices.
             */
            virtual void SetPerspective(const float fov, const float aspect,
                                        const float near_plane, const float far_plane) noexcept = 0;

            /**
             * @brief Sets the viewport dimensions
             * @param[in] x Viewport top-left X coordinate
             * @param[in] y Viewport top-left Y coordinate
             * @param[in] w Viewport width in pixels
             * @param[in] h Viewport height in pixels
             *
             * Defines the screen-space rectangle where the view will be rendered.
             * The viewport transformation maps normalized device coordinates to
             * screen coordinates within this rectangle.
             */
            virtual void SetViewport(const uint32_t x, const uint32_t y,
                                     const uint32_t w, const uint32_t h) noexcept = 0;
            /**@}*/

            /**
             * @name Matrix Generation
             * @brief Generate transformation matrices for rendering
             *
             * These methods compute and return the mathematical transformations
             * required by the rendering pipeline. Matrices are typically cached
             * internally and only recalculated when underlying parameters change.
             */
            /**@{*/

            /**
             * @brief Generates the view transformation matrix
             * @return 4x4 view matrix transforming world to camera space
             *
             * Returns the view matrix based on the currently set camera position
             * and rotation. This matrix transforms world-space coordinates to
             * camera-relative coordinates for rendering.
             *
             * @par Implementation Note:
             * Implementations should cache this matrix and only recalculate when
             * camera parameters change to avoid redundant computations.
             */
            virtual const glm::mat4 &GetViewMatrix() const noexcept = 0;

            /**
             * @brief Generates the projection transformation matrix
             * @return 4x4 projection matrix transforming camera to clip space
             *
             * Returns the projection matrix based on the current viewport dimensions
             * and projection parameters (FOV, near/far planes). This matrix defines
             * the perspective or orthographic transformation applied to vertices.
             */
            virtual const glm::mat4 &GetProjectionMatrix() const noexcept = 0;

            /**
             * @brief Generates the combined view-projection matrix
             * @return 4x4 view-projection matrix (projection * view)
             *
             * Returns the product of the projection and view matrices.
             * This combined matrix transforms vertices directly from world space
             * to clip space in a single operation, which is more efficient than
             * applying view and projection matrices separately.
             */
            virtual const glm::mat4 &GetViewProjectionMatrix() const noexcept = 0;

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
             * Called once per frame to update view-specific logic, animations,
             * or time-dependent behaviors. The delta_time parameter allows for
             * framerate-independent animations and smooth transitions.
             *
             * @par Typical Usage:
             * - Update camera animations or transitions
             * - Recalculate dependent matrices if needed
             * - Update any frame-specific state
             */
            virtual void Update(const float delta_time) noexcept = 0;

            /**@}*/

        private:
            /**
             * @name Copy and Move Control
             * @brief Prevent object slicing and ensure polymorphic behavior
             *
             * All copy and move operations are explicitly deleted to prevent
             * object slicing and maintain proper polymorphic behavior. View
             * objects should always be managed through smart pointers rather
             * than by value.
             *
             * @note Following C++ Core Guidelines for polymorphic base classes.
             */
            /**@{*/
            IView(const IView &) = delete;
            IView &operator=(const IView &) = delete;
            IView(IView &&) = delete;
            IView &operator=(IView &&) = delete;
            /**@}*/
        };

    } // namespace rendering
} // namespace hud3d
