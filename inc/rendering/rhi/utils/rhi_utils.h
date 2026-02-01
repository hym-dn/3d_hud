#pragma once

/**
 * @file rhi_utils.h
 * @brief RHI Utility Toolbox – 常用辅助函数和类型查询
 * 
 * 本模块提供与RHI层类型相关的编译时和运行时辅助函数，包括纹理格式分析、
 * 资源句柄验证、枚举转换等。这些函数不包含任何渲染命令，仅用于辅助开发。
 */

#include "rhi_types.h"
#include "texture_utils.h"

namespace hud_3d
{
    namespace rhi
    {
        // 位运算支持函数（原在 rhi_types.h 中）
        constexpr BufferUsage operator|(BufferUsage lhs, BufferUsage rhs) noexcept
        {
            return static_cast<BufferUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
        }
        constexpr BufferUsage operator&(BufferUsage lhs, BufferUsage rhs) noexcept
        {
            return static_cast<BufferUsage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
        }
        constexpr BufferUsage operator^(BufferUsage lhs, BufferUsage rhs) noexcept
        {
            return static_cast<BufferUsage>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs));
        }
        constexpr BufferUsage operator~(BufferUsage flags) noexcept
        {
            return static_cast<BufferUsage>(~static_cast<uint32_t>(flags));
        }
        constexpr BufferUsage &operator|=(BufferUsage &lhs, BufferUsage rhs) noexcept
        {
            lhs = lhs | rhs;
            return lhs;
        }
        constexpr BufferUsage &operator&=(BufferUsage &lhs, BufferUsage rhs) noexcept
        {
            lhs = lhs & rhs;
            return lhs;
        }
        constexpr BufferUsage &operator^=(BufferUsage &lhs, BufferUsage rhs) noexcept
        {
            lhs = lhs ^ rhs;
            return lhs;
        }

        // TextureUsage 位运算支持函数
        constexpr TextureUsage operator|(TextureUsage lhs, TextureUsage rhs) noexcept
        {
            return static_cast<TextureUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
        }
        constexpr TextureUsage operator&(TextureUsage lhs, TextureUsage rhs) noexcept
        {
            return static_cast<TextureUsage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
        }
        constexpr TextureUsage operator^(TextureUsage lhs, TextureUsage rhs) noexcept
        {
            return static_cast<TextureUsage>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs));
        }
        constexpr TextureUsage operator~(TextureUsage flags) noexcept
        {
            return static_cast<TextureUsage>(~static_cast<uint32_t>(flags));
        }
        constexpr TextureUsage &operator|=(TextureUsage &lhs, TextureUsage rhs) noexcept
        {
            lhs = lhs | rhs;
            return lhs;
        }
        constexpr TextureUsage &operator&=(TextureUsage &lhs, TextureUsage rhs) noexcept
        {
            lhs = lhs & rhs;
            return lhs;
        }
        constexpr TextureUsage &operator^=(TextureUsage &lhs, TextureUsage rhs) noexcept
        {
            lhs = lhs ^ rhs;
            return lhs;
        }

        namespace utils
        {
            // 未来可在此添加其他工具函数的声明
            // 例如：BufferFormat工具、资源统计工具、调试标记工具等

        } // namespace utils
    } // namespace rhi
} // namespace hud_3d