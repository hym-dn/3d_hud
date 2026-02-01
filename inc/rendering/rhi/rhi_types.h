#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <type_traits>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <utility>
#include "utils/common/result.h"

// 前向声明
namespace hud_3d
{
    namespace platform
    {
        class IGraphicsContext;
    } // namespace platform

    namespace rhi
    {
        template <typename T>
        using Result = hud_3d::utils::Result<T>;

        template <typename Derived>
        struct ResourceHandle
        {
            uint32_t id = 0;

            bool IsValid() const noexcept
            {
                return id != 0;
            }

            bool operator==(const Derived &other) const noexcept = default;
            bool operator!=(const Derived &other) const noexcept = default;
        };

        struct ShaderHandle : ResourceHandle<ShaderHandle>
        {
        };

        struct TextureHandle : ResourceHandle<TextureHandle>
        {
        };

        struct BufferHandle : ResourceHandle<BufferHandle>
        {
        };

        struct FramebufferHandle : ResourceHandle<FramebufferHandle>
        {
        };

        struct VertexArrayHandle : ResourceHandle<VertexArrayHandle>
        {
        };

        static_assert(std::is_trivial_v<ShaderHandle>, "ShaderHandle must be trivial");
        static_assert(std::is_standard_layout_v<ShaderHandle>, "ShaderHandle must be standard layout");
        static_assert(std::is_trivial_v<TextureHandle>, "TextureHandle must be trivial");
        static_assert(std::is_standard_layout_v<TextureHandle>, "TextureHandle must be standard layout");
        static_assert(std::is_trivial_v<BufferHandle>, "BufferHandle must be trivial");
        static_assert(std::is_standard_layout_v<BufferHandle>, "BufferHandle must be standard layout");
        static_assert(std::is_trivial_v<FramebufferHandle>, "FramebufferHandle must be trivial");
        static_assert(std::is_standard_layout_v<FramebufferHandle>, "FramebufferHandle must be standard layout");
        static_assert(std::is_trivial_v<VertexArrayHandle>, "VertexArrayHandle must be trivial");
        static_assert(std::is_standard_layout_v<VertexArrayHandle>, "VertexArrayHandle must be standard layout");

        /**
         * @brief 纹理像素格式枚举
         *
         * 支持多种纹理格式，包括：
         * - 无符号归一化格式（UNORM）：8位、16位、32位
         * - 有符号归一化格式（SNORM）：16位
         * - 浮点格式（FLOAT）：16位、32位
         * - 整数格式（INT/UINT）：8位、16位、32位
         * - sRGB格式：带伽马校正的颜色空间
         * - 深度/模板格式（DEPTH/STENCIL）
         * - 压缩格式（BC/DXT）：块压缩，节省显存带宽
         *
         * 格式命名规则：{组件}{位深度}{类型}
         * 例如：RGBA8 = RGBA各8位无符号归一化
         *       RG16F = RG各16位浮点
         *       BC3 = BC3压缩格式（DXT5）
         */
        enum class TextureFormat : uint32_t
        {
            // ========== 无符号归一化格式（UNORM） ==========
            R8,     ///< 单通道8位无符号归一化 (0-255映射到0.0-1.0)
            RG8,    ///< 双通道8位无符号归一化
            RGB8,   ///< 三通道8位无符号归一化 (24位)
            RGBA8,  ///< 四通道8位无符号归一化 (32位)
            R16,    ///< 单通道16位无符号归一化 (0-65535映射到0.0-1.0)
            RG16,   ///< 双通道16位无符号归一化
            RGB16,  ///< 三通道16位无符号归一化 (48位)
            RGBA16, ///< 四通道16位无符号归一化 (64位)

            // ========== 有符号归一化格式（SNORM） ==========
            R8_SNORM,     ///< 单通道8位有符号归一化 (-128-127映射到-1.0-1.0)
            RG8_SNORM,    ///< 双通道8位有签名归一化
            RGB8_SNORM,   ///< 三通道8位有签名归一化
            RGBA8_SNORM,  ///< 四通道8位有签名归一化
            R16_SNORM,    ///< 单通道16位有签名归一化
            RG16_SNORM,   ///< 双通道16位有签名归一化
            RGB16_SNORM,  ///< 三通道16位有签名归一化
            RGBA16_SNORM, ///< 四通道16位有签名归一化

            // ========== 浮点格式（FLOAT） ==========
            R16F,    ///< 单通道16位浮点（半精度）
            RG16F,   ///< 双通道16位浮点
            RGB16F,  ///< 三通道16位浮点
            RGBA16F, ///< 四通道16位浮点
            R32F,    ///< 单通道32位浮点（单精度）
            RG32F,   ///< 双通道32位浮点
            RGB32F,  ///< 三通道32位浮点（96位）
            RGBA32F, ///< 四通道32位浮点（128位）

            // ========== 整数格式（INT/UINT） ==========
            R8I,      ///< 单通道8位有符号整数
            RG8I,     ///< 双通道8位有符号整数
            RGB8I,    ///< 三通道8位有符号整数
            RGBA8I,   ///< 四通道8位有符号整数
            R8UI,     ///< 单通道8位无符号整数
            RG8UI,    ///< 双通道8位无符号整数
            RGB8UI,   ///< 三通道8位无符号整数
            RGBA8UI,  ///< 四通道8位无符号整数
            R16I,     ///< 单通道16位有符号整数
            RG16I,    ///< 双通道16位有符号整数
            RGB16I,   ///< 三通道16位有符号整数
            RGBA16I,  ///< 四通道16位有符号整数
            R16UI,    ///< 单通道16位无符号整数
            RG16UI,   ///< 双通道16位无符号整数
            RGB16UI,  ///< 三通道16位无符号整数
            RGBA16UI, ///< 四通道16位无符号整数
            R32I,     ///< 单通道32位有符号整数
            RG32I,    ///< 双通道32位有符号整数
            RGB32I,   ///< 三通道32位有符号整数
            RGBA32I,  ///< 四通道32位有符号整数
            R32UI,    ///< 单通道32位无符号整数
            RG32UI,   ///< 双通道32位无符号整数
            RGB32UI,  ///< 三通道32位无符号整数
            RGBA32UI, ///< 四通道32位无符号整数

            // ========== sRGB格式（伽马校正颜色空间） ==========
            SRGB8,        ///< 三通道8位sRGB颜色空间
            SRGB8_ALPHA8, ///< 四通道8位sRGB颜色空间带Alpha

            // ========== 深度/模板格式 ==========
            Depth16,          ///< 16位深度缓冲
            Depth24,          ///< 24位深度缓冲
            Depth32F,         ///< 32位浮点深度缓冲
            Depth24Stencil8,  ///< 24位深度 + 8位模板（通常为D24S8）
            Depth32FStencil8, ///< 32位浮点深度 + 8位模板（D32FS8）
            StencilIndex8,    ///< 8位模板缓冲

            // ========== 压缩格式（块压缩） ==========
            BC1,       ///< BC1/DXT1压缩（RGB 4bpp，无Alpha或1位Alpha）
            BC1_SRGB,  ///< BC1 sRGB压缩
            BC2,       ///< BC2/DXT3压缩（RGBA 8bpp，显式Alpha）
            BC2_SRGB,  ///< BC2 sRGB压缩
            BC3,       ///< BC3/DXT5压缩（RGBA 8bpp，插值Alpha）
            BC3_SRGB,  ///< BC3 sRGB压缩
            BC4_UNORM, ///< BC4压缩（单通道无符号归一化）
            BC4_SNORM, ///< BC4压缩（单通道有签名归一化）
            BC5_UNORM, ///< BC5压缩（双通道无符号归一化）
            BC5_SNORM, ///< BC5压缩（双通道有签名归一化）
            BC6H_UF16, ///< BC6H压缩（HDR浮点，无符号16位浮点）
            BC6H_SF16, ///< BC6H压缩（HDR浮点，有符号16位浮点）
            BC7_UNORM, ///< BC7压缩（高质量RGBA 8bpp）
            BC7_SRGB,  ///< BC7 sRGB压缩

            // ========== 特殊格式 ==========
            RGB10A2_UNORM, ///< RGB各10位 + Alpha 2位无符号归一化
            RGB10A2_UINT,  ///< RGB各10位 + Alpha 2位无符号整数
            RGB9E5,        ///< 共享指数HDR格式（RGB各9位 + 5位指数）
            R11G11B10F,    ///< 各通道不同精度的浮点格式
        };

        /**
         * @brief 缓冲区使用标志位（位掩码）
         *
         * 用于指定缓冲区的用途和优化提示，支持多种标志的组合。
         * 设计原则：
         * - 用途标志（Usage flags）：定义缓冲区在渲染管线中的具体用途
         * - 频率提示（Frequency hints）：指导驱动程序进行内存优化
         *
         * 使用示例：
         * @code
         * // 静态顶点缓冲区
         * BufferUsage usage = BufferUsage::VertexBuffer | BufferUsage::Static;
         * // 动态统一缓冲区
         * BufferUsage usage = BufferUsage::UniformBuffer | BufferUsage::Dynamic;
         * // 传输源缓冲区（用于拷贝到纹理）
         * BufferUsage usage = BufferUsage::TransferSrc;
         * @endcode
         *
         * 与OpenGL映射关系：
         * - StaticDraw  ≈ VertexBuffer|IndexBuffer|UniformBuffer + Static
         * - DynamicDraw ≈ VertexBuffer|IndexBuffer|UniformBuffer + Dynamic
         * - StreamDraw  ≈ VertexBuffer|IndexBuffer|UniformBuffer + Stream
         *
         * 与Vulkan映射关系：
         * - VK_BUFFER_USAGE_TRANSFER_SRC_BIT  = TransferSrc
         * - VK_BUFFER_USAGE_TRANSFER_DST_BIT  = TransferDst
         * - VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT = UniformBuffer
         * - 等等
         */
        enum class BufferUsage : uint32_t
        {
            None = 0, ///< 无标志
            // ========== 用途标志 ==========
            TransferSrc = 1 << 0,        ///< 可作为传输源（拷贝到其他缓冲区/纹理）
            TransferDst = 1 << 1,        ///< 可作为传输目标（从其他缓冲区/纹理拷贝）
            UniformTexelBuffer = 1 << 2, ///< 可作为统一缓冲区的纹理缓冲区（UBO texel）
            StorageTexelBuffer = 1 << 3, ///< 可作为存储缓冲区的纹理缓冲区（SSBO texel）
            UniformBuffer = 1 << 4,      ///< 统一缓冲区（UBO）
            StorageBuffer = 1 << 5,      ///< 存储缓冲区（SSBO）
            IndexBuffer = 1 << 6,        ///< 索引缓冲区
            VertexBuffer = 1 << 7,       ///< 顶点缓冲区
            IndirectBuffer = 1 << 8,     ///< 间接参数缓冲区
            // ========== 频率提示 ==========
            Static = 1 << 9,   ///< 数据不常更改（初始化后基本不变）
            Dynamic = 1 << 10, ///< 数据经常更改（每帧或多次每帧）
            Stream = 1 << 11,  ///< 数据每帧更改（流式更新）
            // ========== 常用组合 ==========
            StaticDraw = 720,   ///< 静态绘制（兼容旧版） VertexBuffer|IndexBuffer|UniformBuffer|Static
            DynamicDraw = 1232, ///< 动态绘制（兼容旧版） VertexBuffer|IndexBuffer|UniformBuffer|Dynamic
            StreamDraw = 2256,  ///< 流式绘制（兼容旧版） VertexBuffer|IndexBuffer|UniformBuffer|Stream
        };
        
        /**
         * @brief 纹理使用标志位（位掩码）
         *
         * 用于指定纹理的预期用途，指导驱动程序进行内存优化和访问限制。
         * 设计原则：
         * - 用途标志（Usage flags）：定义纹理在渲染管线中的具体用途
         * - 跨API兼容：映射到Vulkan/D3D12/Metal的相应标志
         *
         * 使用示例：
         * @code
         * // 可采样纹理（默认）
         * TextureUsage usage = TextureUsage::Sampled;
         * // 渲染目标（颜色附件）
         * TextureUsage usage = TextureUsage::ColorAttachment;
         * // 存储纹理（计算着色器写入）
         * TextureUsage usage = TextureUsage::Storage;
         * @endcode
         *
         * 与OpenGL映射关系：
         * - Sampled ≈ GL_TEXTURE_2D (绑定到纹理单元)
         * - ColorAttachment ≈ GL_COLOR_ATTACHMENT0
         * - DepthStencilAttachment ≈ GL_DEPTH_ATTACHMENT
         * - Storage ≈ GL_IMAGE_TEXTURE (图像加载/存储)
         *
         * 与Vulkan映射关系：
         * - VK_IMAGE_USAGE_SAMPLED_BIT = Sampled
         * - VK_IMAGE_USAGE_STORAGE_BIT = Storage
         * - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = ColorAttachment
         * - 等等
         */
        enum class TextureUsage : uint32_t
        {
            None = 0, ///< 无标志
            // ========== 基本用途 ==========
            Sampled = 1 << 0,                ///< 纹理可作为采样器输入（GL_TEXTURE_2D等）
            Storage = 1 << 1,                ///< 纹理可作为图像存储（GL_IMAGE_TEXTURE）
            TransferSrc = 1 << 2,            ///< 纹理可作为传输源（glCopyTexImage2D）
            TransferDst = 1 << 3,            ///< 纹理可作为传输目标（glTexImage2D）
            ColorAttachment = 1 << 4,        ///< 纹理可作为颜色附件（GL_COLOR_ATTACHMENT0）
            DepthStencilAttachment = 1 << 5, ///< 纹理可作为深度/模板附件（GL_DEPTH_ATTACHMENT）
            InputAttachment = 1 << 6,        ///< 纹理可作为输入附件（Vulkan专用）
            TransientAttachment = 1 << 7,    ///< 纹理作为临时附件（内存可优化）
            // ========== 常用组合 ==========
            Default = Sampled,                                       ///< 默认用途：可采样
            RenderTarget = ColorAttachment | DepthStencilAttachment, ///< 渲染目标
            ComputeStorage = Storage,                                ///< 计算着色器存储
        };

        /**
         * @brief 纹理内存布局模式
         *
         * - Optimal: 驱动程序选择的最佳布局（性能最好）
         * - Linear: 线性布局（CPU可读/写，性能较差）
         * - Preinitialized: 预初始化布局（用于纹理数据上传优化）
         */
        enum class TextureTiling : uint8_t
        {
            Optimal = 0,        ///< 驱动程序最优布局（默认）
            Linear = 1,         ///< 线性内存布局（CPU可访问）
            Preinitialized = 2, ///< 预初始化布局（上传优化）
        };

        /**
         * @brief 纹理描述符
         *
         * 描述纹理的维度、格式、用途和其他创建参数。
         * 设计支持多种纹理类型：2D、3D、立方体贴图、纹理数组、多重采样纹理等。
         */
        struct TextureDesc
        {
            uint32_t width = 0;                            ///< 纹理宽度（像素）
            uint32_t height = 0;                           ///< 纹理高度（像素）
            uint32_t depth = 1;                            ///< 纹理深度（3D纹理的切片数，默认1）
            uint32_t array_layers = 1;                     ///< 纹理数组层数（默认1，非数组纹理）
            uint32_t mip_levels = 1;                       ///< mipmap级别数（默认1，无mipmaps）
            uint32_t samples = 1;                          ///< 多重采样数（默认1，无多重采样）
            TextureFormat format = TextureFormat::RGBA8;   ///< 像素格式
            TextureUsage usage = TextureUsage::Default;    ///< 使用标志位
            TextureTiling tiling = TextureTiling::Optimal; ///< 内存布局模式

            // 传统OpenGL风格的状态（未来可能移至独立的Sampler对象）
            bool generate_mipmaps = false; ///< 是否自动生成mipmaps（如果mip_levels>1则忽略）
            bool wrap_repeat = true;       ///< 纹理环绕模式：重复（true）或钳制（false）
            bool filter_linear = true;     ///< 纹理过滤模式：线性（true）或最近邻（false）

            /**
             * @brief 检查描述符是否有效
             * @return true如果宽度、高度、深度、数组层数、mip级别、采样数均有效
             */
            bool IsValid() const
            {
                return width > 0 && height > 0 && depth > 0 &&
                       array_layers > 0 && mip_levels > 0 && samples > 0;
            }

            /**
             * @brief 检查是否为3D纹理
             * @return true如果深度 > 1
             */
            bool Is3D() const
            {
                return depth > 1;
            }

            /**
             * @brief 检查是否为纹理数组
             * @return true如果数组层数 > 1
             */
            bool IsArray() const
            {
                return array_layers > 1;
            }

            /**
             * @brief 检查是否为立方体贴图
             * @return true如果array_layers是6的倍数且深度为1
             * @note 立方体贴图通常作为纹理数组处理，每6个连续层表示一个立方体贴图
             */
            bool IsCubeMap() const
            {
                return depth == 1 && array_layers % 6 == 0;
            }

            /**
             * @brief 检查是否为多重采样纹理
             * @return true如果采样数 > 1
             */
            bool IsMultisample() const
            {
                return samples > 1;
            }
        };

        /**
         * @brief 缓冲区描述符
         *
         * @note BufferTarget 已弃用，用途信息应通过 BufferUsage 中的用途标志指定。
         * 例如，创建静态顶点缓冲区：
         * @code
         * BufferDesc desc{
         *     .size = sizeof(vertices),
         *     .usage = BufferUsage::VertexBuffer | BufferUsage::Static
         * };
         * @endcode
         */
        struct BufferDesc
        {
            uint32_t size = 0;                                                   ///< 缓冲区大小（字节）
            BufferUsage usage = BufferUsage::VertexBuffer | BufferUsage::Static; ///< 使用标志位组合

            bool IsValid() const { return size > 0; }
        };

        struct FramebufferDesc
        {
            TextureHandle color_attachments[8];
            uint32_t color_attachment_count = 0;
            TextureHandle depth_attachment;
            uint32_t width = 0;
            uint32_t height = 0;

            bool IsValid() const
            {
                return width > 0 && height > 0 &&
                       (color_attachment_count > 0 || depth_attachment.IsValid());
            }
        };

        // 渲染状态枚举
        enum class PrimitiveType : uint8_t
        {
            Triangles,
            TriangleStrip,
            Lines,
            LineStrip,
            Points
        };

        enum class ClearFlags : uint32_t
        {
            Color = 0x01,
            Depth = 0x02,
            Stencil = 0x04
        };

        /**
         * @brief 渲染设备能力信息
         */
        struct DeviceCapabilities
        {
            uint32_t max_texture_size = 2048;
            uint32_t max_texture_units = 16;
            uint32_t max_color_attachments = 8;
            uint32_t max_uniform_buffer_bindings = 16;
            uint32_t max_vertex_attributes = 16;

            bool supports_instancing = false;
            bool supports_compute_shader = false;
            bool supports_geometry_shader = false;
            bool supports_debug_output = false;

            uint32_t max_contexts = 8;            ///< 最大支持的渲染上下文数
            uint32_t max_windows = 8;             ///< 最大支持的窗口数
            bool supports_context_sharing = true; ///< 是否支持上下文资源共享
        };

        /**
         * @brief 资源上下文类型
         *
         * - Shared: 共享资源，同一共享组内的窗口/上下文可见
         * - Private: 上下文私有资源
         * - Transient: 短期存在的临时资源（自动回收）
         */
        enum class ResourceContextType : uint8_t
        {
            Shared = 0,   ///< 共享资源（属于共享组）
            Private = 1,  ///< 上下文私有资源
            Transient = 2 ///< 瞬态资源（自动回收）
        };

        /**
         * @brief 资源上下文
         *
         * 用于标识资源所属的上下文，支持多窗口场景下的资源隔离
         */
        struct ResourceContext
        {
            uint32_t context_id = 0; ///< 上下文ID（0=共享资源）
            ResourceContextType type = ResourceContextType::Shared;

            bool IsValid() const
            {
                return type == ResourceContextType::Shared || context_id != 0;
            }

            bool IsShared() const { return type == ResourceContextType::Shared; }

            static ResourceContext Global()
            {
                return {0, ResourceContextType::Shared};
            }

            static ResourceContext Private(uint32_t id)
            {
                return {id, ResourceContextType::Private};
            }
        };

        /**
         * @brief 资源元数据（用于引用计数和生命周期管理）
         */
        struct ResourceMetadata
        {
            std::string name;        ///< 资源名称
            ResourceContext context; ///< 所属上下文
            std::chrono::steady_clock::time_point creation_time;
            std::chrono::steady_clock::time_point last_access_time;

            std::atomic<uint32_t> ref_count{1};              ///< 引用计数
            std::atomic<bool> is_valid{true};                ///< 是否有效
            std::atomic<bool> is_pending_destruction{false}; ///< 是否待销毁

            uint64_t memory_usage = 0; ///< 内存使用量

            void AddReference()
            {
                ref_count.fetch_add(1, std::memory_order_relaxed);
                last_access_time = std::chrono::steady_clock::now();
            }

            bool ReleaseReference()
            {
                uint32_t old = ref_count.fetch_sub(1, std::memory_order_release);
                if (old == 1)
                {
                    std::atomic_thread_fence(std::memory_order_acquire);
                    return true;
                }
                return false;
            }
        };

        /**
         * @brief 管理的资源句柄（带引用计数）
         */
        template <typename HandleType>
        class ManagedResourceHandle
        {
        public:
            ManagedResourceHandle() = default;

            ManagedResourceHandle(HandleType handle, std::shared_ptr<ResourceMetadata> metadata)
                : handle_(handle), metadata_(std::move(metadata))
            {
                if (metadata_)
                    metadata_->AddReference();
            }

            // 拷贝构造 - 增加引用
            ManagedResourceHandle(const ManagedResourceHandle &other)
                : handle_(other.handle_), metadata_(other.metadata_)
            {
                if (metadata_)
                    metadata_->AddReference();
            }

            // 移动构造
            ManagedResourceHandle(ManagedResourceHandle &&other) noexcept
                : handle_(other.handle_), metadata_(std::move(other.metadata_))
            {
                other.handle_ = HandleType{};
            }

            ~ManagedResourceHandle() { Release(); }

            // 拷贝赋值
            ManagedResourceHandle &operator=(const ManagedResourceHandle &other)
            {
                if (this != &other)
                {
                    Release();
                    handle_ = other.handle_;
                    metadata_ = other.metadata_;
                    if (metadata_)
                        metadata_->AddReference();
                }
                return *this;
            }

            // 移动赋值
            ManagedResourceHandle &operator=(ManagedResourceHandle &&other) noexcept
            {
                if (this != &other)
                {
                    Release();
                    handle_ = other.handle_;
                    metadata_ = std::move(other.metadata_);
                    other.handle_ = HandleType{};
                }
                return *this;
            }

            HandleType Get() const { return handle_; }

            bool IsValid() const
            {
                return handle_.IsValid() && metadata_ && metadata_->is_valid.load();
            }

            explicit operator bool() const { return IsValid(); }

            std::string_view GetName() const
            {
                return metadata_ ? metadata_->name : "Invalid";
            }

            ResourceContext GetContext() const
            {
                return metadata_ ? metadata_->context : ResourceContext{};
            }

            bool IsShared() const
            {
                return metadata_ && metadata_->context.IsShared();
            }

            void Reset()
            {
                Release();
                handle_ = HandleType{};
                metadata_.reset();
            }

        private:
            void Release()
            {
                if (metadata_ && metadata_->ReleaseReference())
                {
                    metadata_->is_pending_destruction.store(true);
                }
            }

            HandleType handle_{};
            std::shared_ptr<ResourceMetadata> metadata_;
        };

        // 类型别名
        using ManagedShaderHandle = ManagedResourceHandle<ShaderHandle>;
        using ManagedTextureHandle = ManagedResourceHandle<TextureHandle>;
        using ManagedBufferHandle = ManagedResourceHandle<BufferHandle>;
        using ManagedFramebufferHandle = ManagedResourceHandle<FramebufferHandle>;
        using ManagedVertexArrayHandle = ManagedResourceHandle<VertexArrayHandle>;

        /**
         * @brief 窗口绑定信息
         *
         * 维护窗口与图形上下文的映射关系
         */
        struct WindowBinding
        {
            uint32_t window_id = 0;                        ///< 窗口唯一标识
            platform::IGraphicsContext *context = nullptr; ///< 关联的图形上下文（裸指针，由调用者管理生命周期）
            ResourceContext resource_context;              ///< 资源上下文（使用结构体类型，更类型安全）
            bool is_active = false;                        ///< 是否处于活跃状态

            bool IsValid() const
            {
                return window_id != 0 && context != nullptr;
            }
        };

        /**
         * @brief 图形上下文与资源上下文的关联信息
         *
         * 维护平台图形上下文(IGraphicsContext)与资源上下文的映射关系
         */
        struct ContextAssociation
        {
            uint32_t resource_context_id = 0;                       ///< 资源上下文ID
            platform::IGraphicsContext *graphics_context = nullptr; ///< 关联的图形上下文
            uint32_t window_id = 0;                                 ///< 关联的窗口ID
            bool is_primary = false;                                ///< 是否是主上下文

            bool IsValid() const
            {
                return resource_context_id != 0 && graphics_context != nullptr;
            }
        };

        /**
         * @brief 资源统计信息
         *
         * 提供详细的资源使用统计，便于调试和性能分析
         */
        struct ResourceStats
        {
            // 总体统计
            uint32_t total_resources = 0;    ///< 总资源数量
            uint64_t total_memory_bytes = 0; ///< 总内存使用量（字节）

            // 按类型统计
            uint32_t texture_count = 0;      ///< 纹理数量
            uint32_t shader_count = 0;       ///< 着色器数量
            uint32_t buffer_count = 0;       ///< 缓冲区数量
            uint32_t framebuffer_count = 0;  ///< 帧缓冲数量
            uint32_t vertex_array_count = 0; ///< 顶点数组数量

            // 按共享类型统计
            uint32_t shared_count = 0;    ///< 共享资源数量
            uint32_t private_count = 0;   ///< 私有资源数量
            uint32_t transient_count = 0; ///< 瞬态资源数量

            // 内存细分
            uint64_t texture_memory = 0;     ///< 纹理内存使用
            uint64_t buffer_memory = 0;      ///< 缓冲区内存使用
            uint64_t framebuffer_memory = 0; ///< 帧缓冲内存使用

            // 上下文统计
            uint32_t context_count = 0;        ///< 注册的上下文数量
            uint32_t pending_destructions = 0; ///< 待销毁的资源数量
        };

        /**
         * @brief 资源上下文共享模式
         *
         * - Shared: 加入共享组（需指定group_id），同一组的上下文共享资源
         * - Isolated: 完全独立，不与其他上下文共享
         */
        enum class ResourceSharingMode : uint8_t
        {
            Shared = 0,  ///< 加入共享组（需指定group_id>0）
            Isolated = 1 ///< 完全独立资源，不与其他上下文共享
        };

        /**
         * @brief 多上下文资源管理接口
         *
         * 设计原则：
         * - 资源上下文(ResourceContext)是逻辑概念，用于资源隔离和共享
         * - 图形上下文(IGraphicsContext)是平台概念，用于实际渲染
         * - 一个图形上下文可以关联一个资源上下文
         * - 共享资源在同一共享组内的所有关联图形上下文中可见
         * - 私有资源只在创建它的资源上下文中可见
         *
         * 与IRHIDevice的关系：
         * - IRHIDevice管理窗口与图形上下文的绑定
         * - IResourceManager管理资源上下文与图形上下文的关联
         * - 渲染时，IRHIDevice::MakeCurrent()会同时切换图形上下文和资源上下文
         */
        class IResourceManager
        {
        public:
            virtual ~IResourceManager() = default;

            // ========== 上下文关联管理 ==========

            /**
             * @brief 注册图形上下文并创建/加入共享组
             * @param graphics_context 平台图形上下文（必须已初始化）
             * @param window_id 关联的窗口ID
             * @param sharing_mode 资源共享模式（默认 Shared）
             * @param group_id 共享组ID（Shared模式必须>0，Isolated模式忽略）
             * @param is_primary 是否作为共享组起点（必须至少有一个起点）
             * @return 新资源上下文的ID，失败返回0
             *
             * @note
             * - 每个IGraphicsContext需要注册后才能创建资源
             * - 注册时会创建内部资源上下文用于跟踪资源
             * - OpenGL实现中会创建共享列表的上下文
             * - 共享内容：纹理、缓冲区、着色器
             * - 不共享：帧缓冲对象(FBO)、顶点数组对象(VAO)、查询对象
             * - Shared模式：同一group_id的上下文形成共享组，可以互访共享资源
             * - Isolated模式：完全独立，只能访问私有资源
             * - 不同group_id的共享组之间资源完全隔离
             * - group_id=0保留，表示"无共享组"
             */
            virtual uint32_t RegisterGraphicsContext(
                platform::IGraphicsContext *graphics_context,
                uint32_t window_id,
                ResourceSharingMode sharing_mode = ResourceSharingMode::Shared,
                uint32_t group_id = 0,
                bool is_primary = false) = 0;

            /**
             * @brief 注销图形上下文
             * @param resource_context_id 资源上下文ID
             * @return 成功返回Success
             *
             * @note
             * - 会自动清理该上下文的所有私有资源
             * - 不会销毁IGraphicsContext（由调用者管理）
             */
            virtual Result<void> UnregisterGraphicsContext(uint32_t resource_context_id) = 0;

            /**
             * @brief 获取图形上下文关联的资源上下文ID
             * @param graphics_context 图形上下文
             * @return 资源上下文ID，未注册返回0
             */
            virtual uint32_t GetResourceContextId(
                platform::IGraphicsContext *graphics_context) const = 0;

            /**
             * @brief 获取资源上下文关联的图形上下文
             * @param resource_context_id 资源上下文ID
             * @return 图形上下文指针，未找到返回nullptr
             */
            virtual platform::IGraphicsContext *GetGraphicsContext(
                uint32_t resource_context_id) const = 0;

            /**
             * @brief 获取资源上下文关联的窗口ID
             * @param resource_context_id 资源上下文ID
             * @return 窗口ID，未找到返回0
             */
            virtual uint32_t GetWindowId(uint32_t resource_context_id) const = 0;

            /**
             * @brief 获取上下文关联信息
             * @param resource_context_id 资源上下文ID
             * @return 关联信息，未找到返回无效结构
             */
            virtual ContextAssociation GetContextAssociation(
                uint32_t resource_context_id) const = 0;

            // ========== 当前上下文管理 ==========

            /**
             * @brief 设置当前活跃的资源上下文
             * @param resource_context_id 资源上下文ID
             * @return 成功返回Success
             *
             * @note
             * - 线程安全：每个线程可以有自己的当前上下文
             * - 通常由IRHIDevice::MakeCurrent()调用，不需要手动调用
             */
            virtual Result<void> MakeCurrent(uint32_t resource_context_id) = 0;

            /**
             * @brief 获取当前线程的活跃资源上下文ID
             * @return 当前资源上下文ID，无当前上下文返回0
             */
            virtual uint32_t GetCurrentContext() const = 0;

            /**
             * @brief 获取当前线程的活跃图形上下文
             * @return 当前图形上下文，无当前上下文返回nullptr
             */
            virtual platform::IGraphicsContext *GetCurrentGraphicsContext() const = 0;

            /**
             * @brief 清除当前线程的当前上下文
             * @return 成功返回Success
             */
            virtual Result<void> ClearCurrent() = 0;

            /**
             * @brief 检查指定资源上下文是否有效
             */
            virtual bool IsContextValid(uint32_t resource_context_id) const = 0;

            /**
             * @brief 获取所有已注册的资源上下文数量
             */
            virtual uint32_t GetRegisteredContextCount() const = 0;

            /**
             * @brief 遍历所有上下文关联信息
             * @param callback 回调函数，返回false停止遍历
             */
            virtual void ForEachContext(
                std::function<bool(const ContextAssociation &)> callback) const = 0;

            // ========== 共享资源管理 ==========

            /**
             * @brief 创建共享纹理（同一共享组内的所有上下文可见）
             * @param name 资源名称（用于查找和调试）
             * @param desc 纹理描述符
             * @param data 初始数据（可选）
             * @return 管理的纹理句柄
             *
             * @note 资源存储在当前活跃上下文所属的共享组中，仅该组内的上下文可以访问
             */
            virtual Result<ManagedTextureHandle> CreateSharedTexture(
                std::string_view name,
                const TextureDesc &desc,
                const void *data = nullptr) = 0;

            /**
             * @brief 创建共享着色器（同一共享组内的所有上下文可见）
             * @param name 资源名称
             * @param vertex_src 顶点着色器源码
             * @param fragment_src 片段着色器源码
             * @return 管理的着色器句柄
             *
             * @note 资源存储在当前活跃上下文所属的共享组中，仅该组内的上下文可以访问
             */
            virtual Result<ManagedShaderHandle> CreateSharedShader(
                std::string_view name,
                std::string_view vertex_src,
                std::string_view fragment_src) = 0;

            /**
             * @brief 创建共享缓冲区（同一共享组内的所有上下文可见）
             * @param name 资源名称
             * @param desc 缓冲区描述符
             * @param data 初始数据
             * @return 管理的缓冲区句柄
             *
             * @note 资源存储在当前活跃上下文所属的共享组中，仅该组内的上下文可以访问
             */
            virtual Result<ManagedBufferHandle> CreateSharedBuffer(
                std::string_view name,
                const BufferDesc &desc,
                const void *data = nullptr) = 0;

            /**
             * @brief 按名称查找共享纹理
             * @param name 资源名称
             * @return 纹理句柄，不存在则返回无效句柄
             */
            virtual ManagedTextureHandle FindSharedTexture(std::string_view name) = 0;

            /**
             * @brief 按名称查找共享着色器
             */
            virtual ManagedShaderHandle FindSharedShader(std::string_view name) = 0;

            // ========== 私有资源管理 ==========

            /**
             * @brief 创建私有帧缓冲（绑定到特定上下文）
             * @param context 资源上下文
             * @param name 资源名称
             * @param desc 帧缓冲描述符
             * @return 管理的帧缓冲句柄
             *
             * @note 帧缓冲总是私有的，不能跨上下文共享
             */
            virtual Result<ManagedFramebufferHandle> CreatePrivateFramebuffer(
                ResourceContext context,
                std::string_view name,
                const FramebufferDesc &desc) = 0;

            /**
             * @brief 创建私有顶点数组（绑定到特定上下文）
             * @param context 资源上下文
             * @param name 资源名称
             * @return 管理的顶点数组句柄
             */
            virtual Result<ManagedVertexArrayHandle> CreatePrivateVertexArray(
                ResourceContext context,
                std::string_view name) = 0;

            // ========== 资源查询 ==========

            /**
             * @brief 检查资源是否存在于指定上下文
             */
            virtual bool HasResource(ResourceContext context, std::string_view name) = 0;

            /**
             * @brief 获取资源统计信息
             * @return 资源统计结构体，包含详细的资源使用情况
             *
             * @note
             * - 线程安全：返回快照，不影响运行时性能
             * - 可用于性能分析和内存泄漏检测
             */
            virtual ResourceStats GetStats() const = 0;

            // ========== 资源清理 ==========

            /**
             * @brief 清理指定上下文的所有私有资源
             * @param context 要清理的上下文
             * @return 成功返回Success
             *
             * @note 通常在窗口销毁时调用
             */
            virtual Result<void> CleanupContext(ResourceContext context) = 0;

            /**
             * @brief 执行待处理的资源销毁
             * @note 应在每帧末尾调用，清理引用计数归零的资源
             */
            virtual void ProcessPendingDestructions() = 0;

            /**
             * @brief 打印资源使用情况报告（调试用）
             */
            virtual void PrintResourceReport() const = 0;
        };

    } // namespace rhi
} // namespace hud_3d
