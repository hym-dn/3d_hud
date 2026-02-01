#pragma once

#include "rhi_types.h"

namespace hud_3d
{
    namespace rhi
    {
        // 前向声明
        class IResourceManager;

        // RHI设备接口（核心抽象层）
        class IRHIDevice
        {
        public:
            virtual ~IRHIDevice() = default;

            // ========== 设备生命周期 ==========
            /**
             * @brief 初始化RHI设备
             * @param primary_context 主图形上下文（必须已初始化并与窗口关联）
             * @return 成功返回Success
             *
             * @note
             * - 设备将使用primary_context进行初始化和资源创建
             * - primary_context成为默认的渲染上下文
             * - 初始化后会自动创建一个资源管理器
             */
            virtual Result<void> Initialize(platform::IGraphicsContext *primary_context) = 0;

            /**
             * @brief 关闭设备并释放所有资源
             *
             * @note
             * - 自动解绑所有窗口
             * - 清理所有资源（共享和私有）
             * - 不销毁IGraphicsContext（由调用者管理）
             */
            virtual void Shutdown() = 0;

            /**
             * @brief 检查设备是否已初始化
             */
            virtual bool IsInitialized() const = 0;

            // ========== 设备信息 ==========

            /**
             * @brief 获取图形API版本字符串
             */
            virtual const char *GetAPIVersion() const = 0;

            /**
             * @brief 获取GPU厂商信息
             */
            virtual const char *GetVendor() const = 0;

            /**
             * @brief 获取GPU渲染器名称
             */
            virtual const char *GetRenderer() const = 0;

            /**
             * @brief 获取设备能力信息
             */
            virtual DeviceCapabilities GetCapabilities() const = 0;

            // ========== 资源管理器访问 ==========

            /**
             * @brief 获取资源管理器
             * @return 资源管理器指针，设备未初始化时返回nullptr
             *
             * @note
             * - 通过 IResourceManager 创建所有资源（纹理、着色器、缓冲区、FBO、VAO）
             * - IRHIDevice 只负责绑定和使用资源，不负责创建/销毁
             */
            virtual IResourceManager *GetResourceManager() = 0;
            virtual const IResourceManager *GetResourceManager() const = 0;

            // ========== 多窗口支持 ==========

            /**
             * @brief 为窗口绑定图形上下文
             * @param window_id 窗口ID（必须唯一，范围1-MaxWindows）
             * @param context 图形上下文（必须已初始化并与窗口关联）
             * @return 成功返回Success
             *
             * @note
             * - context的生命周期由调用者管理，设备只使用不拥有
             * - 绑定后会自动创建对应的资源上下文
             * - 新窗口的资源上下文默认与主上下文共享资源（即使用相同的共享组）
             * - 重复绑定同一window_id会返回错误
             */
            virtual Result<void> BindToWindow(uint32_t window_id,
                                              platform::IGraphicsContext *context) = 0;

            /**
             * @brief 从窗口解绑
             * @param window_id 窗口ID
             * @return 成功返回Success
             *
             * @note
             * - 解绑前会自动清理该窗口的私有资源
             * - 不会销毁IGraphicsContext
             */
            virtual Result<void> UnbindFromWindow(uint32_t window_id) = 0;

            /**
             * @brief 获取窗口绑定信息
             * @param window_id 窗口ID
             * @return 绑定信息，未绑定返回无效结构
             */
            virtual WindowBinding GetWindowBinding(uint32_t window_id) const = 0;

            /**
             * @brief 检查窗口是否已绑定
             */
            virtual bool IsWindowBound(uint32_t window_id) const = 0;

            /**
             * @brief 获取已绑定窗口数量
             */
            virtual uint32_t GetBoundWindowCount() const = 0;

            /**
             * @brief 获取最大支持窗口数
             */
            virtual uint32_t GetMaxWindows() const = 0;

            // ========== 窗口操作 ==========

            /**
             * @brief 交换窗口缓冲区（显示渲染结果）
             * @param window_id 窗口ID
             * @return 成功返回Success
             *
             * @note 内部调用关联的IGraphicsContext::SwapBuffers()
             */
            virtual Result<void> SwapBuffers(uint32_t window_id) = 0;

            /**
             * @brief 设置垂直同步
             * @param window_id 窗口ID
             * @param enable true启用，false禁用
             * @return 成功返回Success
             */
            virtual Result<void> SetVSync(uint32_t window_id, bool enable) = 0;

            /**
             * @brief 获取窗口尺寸
             * @param window_id 窗口ID
             * @param width 输出宽度
             * @param height 输出高度
             * @return 成功返回Success
             */
            virtual Result<void> GetWindowSize(uint32_t window_id,
                                               uint32_t &width,
                                               uint32_t &height) const = 0;

            // ========== 上下文管理 ==========

            /**
             * @brief 获取当前活跃的图形上下文
             * @return 当前上下文，如果没有则返回nullptr
             */
            virtual platform::IGraphicsContext *GetCurrentContext() const = 0;

            /**
             * @brief 获取指定窗口的图形上下文
             * @param window_id 窗口ID
             * @return 关联的上下文，未绑定返回nullptr
             */
            virtual platform::IGraphicsContext *GetWindowContext(uint32_t window_id) const = 0;

            /**
             * @brief 切换到指定窗口的上下文
             * @param window_id 窗口ID
             * @return 成功返回Success
             *
             * @note
             * - 同时切换图形上下文和资源上下文：
             *   1. 调用 IGraphicsContext::MakeCurrent()
             *   2. 调用 IResourceManager::MakeCurrent(context_id)
             * - 线程安全：每个线程可有自己的当前上下文
             * - 必须在绑定窗口后调用
             */
            virtual Result<void> MakeCurrent(uint32_t window_id) = 0;

            /**
             * @brief 清除当前线程的当前上下文
             * @return 成功返回Success
             */
            virtual Result<void> ClearCurrent() = 0;

            /**
             * @brief 获取当前窗口ID（当前上下文对应的窗口）
             * @return 窗口ID，无当前上下文返回0
             */
            virtual uint32_t GetCurrentWindowId() const = 0;

            // ========== 渲染状态 ==========

            /**
             * @brief 设置视口
             * @param x 视口左下角X坐标
             * @param y 视口左下角Y坐标
             * @param width 视口宽度
             * @param height 视口高度
             */
            virtual Result<void> SetViewport(uint32_t x, uint32_t y,
                                             uint32_t width, uint32_t height) = 0;

            /**
             * @brief 设置裁剪区域
             */
            virtual Result<void> SetScissor(uint32_t x, uint32_t y,
                                            uint32_t width, uint32_t height) = 0;

            /**
             * @brief 设置清除颜色
             */
            virtual Result<void> SetClearColor(float r, float g, float b, float a) = 0;

            /**
             * @brief 清除缓冲区
             * @param flags 清除标志（COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT | STENCIL_BUFFER_BIT）
             */
            virtual Result<void> Clear(uint32_t flags) = 0;

            // ========== 资源绑定 ==========

            virtual Result<void> BindShader(ManagedShaderHandle shader) = 0;
            virtual Result<void> BindTexture(ManagedTextureHandle texture, uint32_t slot) = 0;
            virtual Result<void> BindVertexBuffer(ManagedBufferHandle buffer,
                                                  uint32_t slot,
                                                  uint32_t stride,
                                                  uint32_t offset) = 0;
            virtual Result<void> BindIndexBuffer(ManagedBufferHandle buffer,
                                                 uint32_t index_size,
                                                 uint32_t offset) = 0;
            virtual Result<void> BindFramebuffer(ManagedFramebufferHandle fbo) = 0;
            virtual Result<void> BindVertexArray(ManagedVertexArrayHandle vao) = 0;

            // ========== 绘制命令 ==========

            virtual Result<void> DrawArrays(PrimitiveType type,
                                            uint32_t first,
                                            uint32_t count) = 0;
            virtual Result<void> DrawElements(PrimitiveType type,
                                              uint32_t count,
                                              uint32_t index_size,
                                              uint32_t offset) = 0;
            virtual Result<void> DrawArraysInstanced(PrimitiveType type,
                                                     uint32_t first,
                                                     uint32_t count,
                                                     uint32_t instance_count) = 0;

            // ========== Uniform设置 ==========

            virtual Result<void> SetUniform(ManagedShaderHandle shader,
                                            const char *name,
                                            const void *data,
                                            uint32_t size) = 0;
            virtual Result<void> SetUniformInt(ManagedShaderHandle shader,
                                               const char *name,
                                               int32_t value) = 0;
            virtual Result<void> SetUniformFloat(ManagedShaderHandle shader,
                                                 const char *name,
                                                 float value) = 0;
            virtual Result<void> SetUniformVec3(ManagedShaderHandle shader,
                                                const char *name,
                                                const float *value) = 0;
            virtual Result<void> SetUniformMat4(ManagedShaderHandle shader,
                                                const char *name,
                                                const float *matrix) = 0;

            // ========== 同步操作 ==========

            /**
             * @brief 提交所有挂起的命令
             */
            virtual Result<void> Flush() = 0;

            /**
             * @brief 等待所有命令执行完成
             */
            virtual Result<void> Finish() = 0;

            // ========== 调试功能 ==========

            virtual void EnableDebugOutput(bool enable) = 0;
            virtual void PushDebugGroup(const char *name) = 0;
            virtual void PopDebugGroup() = 0;
        };
        
    } // namespace rhi
} // namespace hud_3d