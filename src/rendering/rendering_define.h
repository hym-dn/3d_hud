/**
 * @file rendering_define.h
 * @brief Core definitions for the rendering system including command types and structures
 *
 * This file contains the fundamental enums and structures used by the command buffer system.
 * It defines the command types, priorities, and basic data structures that are API-agnostic.
 *
 * @author Yameng.He (Optimized by DeepSeek)
 * @date 2025-12-21
 */

#pragma once

#include <cstdint>

namespace hud_3d
{
    namespace rendering
    {
        // =========================================================================
        // Enums & Basic Types
        // =========================================================================

        /**
         * @brief Enumeration of all supported command types
         *
         * This enum defines all the graphics commands that can be recorded and executed
         * by the command buffer system. Commands are categorized by priority and function.
         */
        enum class CommandType : uint16_t
        {
            // ========== State Setting Commands (High Priority) ==========
            SetViewport = 0, /**< Set viewport dimensions */
            SetClearColor,   /**< Set clear color for framebuffer */
            SetDepthRange,   /**< Set depth range for depth testing */
            SetBlendMode,    /**< Set blending mode for transparency */
            SetCullMode,     /**< Set face culling mode */
            SetDepthTest,    /**< Enable/disable depth testing */
            SetScissor,      /**< Set scissor rectangle */

            // ========== Buffer Operation Commands (High Priority) ==========
            ClearBuffers,       /**< Clear specified buffers */
            ClearColorBuffer,   /**< Clear color buffer only */
            ClearDepthBuffer,   /**< Clear depth buffer only */
            ClearStencilBuffer, /**< Clear stencil buffer only */

            // ========== Resource Binding Commands (Normal Priority) ==========
            BindShader,        /**< Bind shader program */
            BindTexture,       /**< Bind texture to texture unit */
            BindVertexBuffer,  /**< Bind vertex buffer */
            BindIndexBuffer,   /**< Bind index buffer */
            BindUniformBuffer, /**< Bind uniform buffer */
            BindFramebuffer,   /**< Bind framebuffer */

            // ========== Drawing Commands (Normal Priority) ==========
            DrawArrays,            /**< Draw using vertex arrays */
            DrawElements,          /**< Draw using indexed vertices */
            DrawArraysInstanced,   /**< Draw instanced using vertex arrays */
            DrawElementsInstanced, /**< Draw instanced using indexed vertices */

            // ========== Transformation Commands (Normal Priority) ==========
            SetModelMatrix,      /**< Set model transformation matrix */
            SetViewMatrix,       /**< Set view transformation matrix */
            SetProjectionMatrix, /**< Set projection matrix */
            SetNormalMatrix,     /**< Set normal transformation matrix */

            // ========== Material Commands (Normal Priority) ==========
            SetMaterialDiffuse,   /**< Set diffuse material properties */
            SetMaterialSpecular,  /**< Set specular material properties */
            SetMaterialAmbient,   /**< Set ambient material properties */
            SetMaterialShininess, /**< Set material shininess */

            // ========== Lighting Commands (Normal Priority) ==========
            SetLightPosition,    /**< Set light position */
            SetLightColor,       /**< Set light color */
            SetLightAttenuation, /**< Set light attenuation parameters */
            SetLightDirection,   /**< Set light direction */

            // ========== Effect Commands (Low Priority) ==========
            BeginPostProcessing, /**< Begin post-processing pass */
            EndPostProcessing,   /**< End post-processing pass */
            ApplyBloom,          /**< Apply bloom effect */
            ApplyToneMapping,    /**< Apply tone mapping */

            // ========== Debug Commands (Low Priority) ==========
            DrawWireframe,   /**< Draw wireframe overlay */
            DrawBoundingBox, /**< Draw bounding boxes */
            DrawNormals,     /**< Draw surface normals */
            DrawDebugText,   /**< Draw debug text */

            // ========== Synchronization Commands (High Priority) ==========
            FlushCommands,  /**< Flush command queue */
            FinishCommands, /**< Finish all pending commands */
            InsertFence,    /**< Insert synchronization fence */

            MaxCommandType = 48 /**< Total number of command types */
        };

        /**
         * @brief Command execution priority levels
         *
         * Commands are executed in priority order: High -> Normal -> Low
         * This ensures critical state changes happen before drawing operations.
         */
        enum class CommandPriority : uint8_t
        {
            High = 0,   /**< High priority commands (state changes, clears) */
            Normal = 1, /**< Normal priority commands (drawing, transformations) */
            Low = 2,    /**< Low priority commands (effects, debug) */
            Count = 3
        };

        /**
         * @brief 16-byte aligned command header structure
         *
         * Each command in the buffer starts with this header, which contains
         * metadata about the command and a pointer to its execution function.
         */
        struct alignas(16) CommandHeader
        {
            uint16_t size;            /**< Total size of command (header + data), must be multiple of 16 */
            CommandType type;         /**< Type of command */
            CommandPriority priority; /**< Execution priority */
            uint8_t padding1;         /**< Padding to 8-byte boundary */
            uint16_t padding2;        /**< Padding to 16-byte boundary */

            /**
             * @brief Function pointer to command execution logic
             * @param header_ptr Pointer to the command header
             */
            void (*Execute)(const void *header_ptr);
        };

        // Ensure proper alignment and size
        static_assert(sizeof(CommandHeader) == 16, "CommandHeader must be exactly 16 bytes");
        static_assert(alignof(CommandHeader) == 16, "CommandHeader must be 16-byte aligned");

        /**
         * @brief Statistics structure for command buffer performance tracking
         */
        struct CommandBufferStats
        {
            uint32_t commands_recorded{0};  /**< Number of commands recorded */
            uint32_t commands_executed{0};  /**< Number of commands executed */
            uint64_t total_bytes_used{0};   /**< Total memory used by commands */
            uint32_t page_count{0};         /**< Number of memory pages allocated */
            uint32_t memory_allocations{0}; /**< Number of memory allocations */
        };

        // =========================================================================
        // Command Data Structures - Simplified Design
        // =========================================================================

        /**
         * @brief Generic command storage structure with 16-byte alignment
         *
         * This template structure combines a command header with the actual
         * command data, ensuring proper alignment and providing execution logic.
         * It serves as the fundamental building block for all command types
         * in the command buffer system.
         *
         * @tparam DataT Type of command data structure (must have Execute() method)
         * @tparam TypeID Command type identifier
         * @tparam Prio Command execution priority (default: Normal)
         */
        template <typename DataT, CommandType TypeID, CommandPriority Prio = CommandPriority::Normal>
        struct alignas(16) CommandStorage
        {
            CommandHeader header; /**< Command metadata and execution function */
            DataT data;           /**< Actual command data */

            /**
             * @brief Static execution function for this command type
             *
             * This function is called by the command buffer system to execute
             * the command. It extracts the command data and calls its Execute method.
             * The function is designed to be compatible with the CommandHeader
             * execution function pointer interface.
             *
             * @param ptr Pointer to the CommandStorage structure
             */
            static void Execute(const void *ptr)
            {
                const auto *storage = static_cast<const CommandStorage *>(ptr);
                storage->data.Execute();
            }

            /**
             * @brief Construct a CommandStorage with forwarded arguments
             *
             * Initializes the command data and sets up the header with
             * proper size, type, priority, and execution function.
             * Uses perfect forwarding to avoid unnecessary copies.
             *
             * @tparam Args Argument types for command data construction
             * @param args Arguments to forward to command data constructor
             */
            template <typename... Args>
            CommandStorage(Args &&...args)
                : data{std::forward<Args>(args)...}
            {
                header.size = sizeof(CommandStorage);
                header.type = TypeID;
                header.priority = Prio;
                header.execute = &Execute;
            }
        };
        
    } // namespace rendering
} // namespace hud_3d