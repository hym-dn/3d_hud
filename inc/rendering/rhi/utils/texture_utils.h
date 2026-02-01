#pragma once

#include "rhi_types.h"
#include <cstdint>
#include <type_traits>

namespace hud_3d
{
    namespace rhi
    {
        namespace utils
        {

            // ============================================================================
            // Texture Format Utilities
            // ============================================================================

            /**
             * @brief Check if a texture format is compressed (block-compressed).
             * @param format The texture format to test.
             * @return true if the format is block-compressed (BC/DXT family), false otherwise.
             */
            [[nodiscard]] constexpr bool IsCompressedFormat(TextureFormat format) noexcept
            {
                using enum TextureFormat;
                switch (format)
                {
                case BC1:
                case BC1_SRGB:
                case BC2:
                case BC2_SRGB:
                case BC3:
                case BC3_SRGB:
                case BC4_UNORM:
                case BC4_SNORM:
                case BC5_UNORM:
                case BC5_SNORM:
                case BC6H_UF16:
                case BC6H_SF16:
                case BC7_UNORM:
                case BC7_SRGB:
                    return true;
                default:
                    return false;
                }
            }

            /**
             * @brief Check if a texture format is a depth/stencil format.
             * @param format The texture format to test.
             * @return true if the format is depth-only, stencil-only, or depth-stencil combined.
             */
            [[nodiscard]] constexpr bool IsDepthStencilFormat(TextureFormat format) noexcept
            {
                using enum TextureFormat;
                switch (format)
                {
                case Depth16:
                case Depth24:
                case Depth32F:
                case Depth24Stencil8:
                case Depth32FStencil8:
                case StencilIndex8:
                    return true;
                default:
                    return false;
                }
            }

            /**
             * @brief Check if a texture format uses sRGB color space (gamma-corrected).
             * @param format The texture format to test.
             * @return true if the format is sRGB, false otherwise.
             */
            [[nodiscard]] constexpr bool IsSRGBFormat(TextureFormat format) noexcept
            {
                using enum TextureFormat;
                switch (format)
                {
                case SRGB8:
                case SRGB8_ALPHA8:
                case BC1_SRGB:
                case BC2_SRGB:
                case BC3_SRGB:
                case BC7_SRGB:
                    return true;
                default:
                    return false;
                }
            }

            /**
             * @brief Check if a texture format stores integer data (signed/unsigned integer).
             * @param format The texture format to test.
             * @return true if the format is integer-based, false for floating-point or normalized formats.
             */
            [[nodiscard]] constexpr bool IsIntegerFormat(TextureFormat format) noexcept
            {
                using enum TextureFormat;
                switch (format)
                {
                case R8I:
                case RG8I:
                case RGB8I:
                case RGBA8I:
                case R8UI:
                case RG8UI:
                case RGB8UI:
                case RGBA8UI:
                case R16I:
                case RG16I:
                case RGB16I:
                case RGBA16I:
                case R32I:
                case RG32I:
                case RGB32I:
                case RGBA32I:
                case R32UI:
                case RG32UI:
                case RGB32UI:
                case RGBA32UI:
                case RGB10A2_UINT:
                    return true;
                default:
                    return false;
                }
            }

            /**
             * @brief Check if a texture format stores floating-point data.
             * @param format The texture format to test.
             * @return true if the format is floating-point, false otherwise.
             */
            [[nodiscard]] constexpr bool IsFloatFormat(TextureFormat format) noexcept
            {
                using enum TextureFormat;
                switch (format)
                {
                case R16F:
                case RG16F:
                case RGB16F:
                case RGBA16F:
                case R32F:
                case RG32F:
                case RGB32F:
                case RGBA32F:
                case BC6H_UF16:
                case BC6H_SF16:
                case RGB9E5:
                case R11G11B10F:
                    return true;
                default:
                    return false;
                }
            }

            /**
             * @brief Get the number of bytes per pixel for an uncompressed format.
             * @param format The texture format.
             * @return Bytes per pixel, or 0 for compressed/unknown formats.
             * @note For compressed formats, use GetCompressedBlockBytes() instead.
             */
            [[nodiscard]] constexpr uint32_t GetBytesPerPixel(TextureFormat format) noexcept
            {
                using enum TextureFormat;
                switch (format)
                {
                // 8‑bit UNORM
                case R8:
                case R8_SNORM:
                case R8I:
                case R8UI:
                    return 1;
                // 16‑bit UNORM/SNORM/INT
                case R16:
                case R16_SNORM:
                case R16F:
                case R16I:
                case R16UI:
                case RG8:
                case RG8_SNORM:
                case RG8I:
                case RG8UI:
                    return 2;
                // 24‑bit (RGB8) or 32‑bit (RGBA8)
                case RGB8:
                case RGB8_SNORM:
                case RGB8I:
                case RGB8UI:
                case SRGB8:
                    return 3;
                case RGBA8:
                case RGBA8_SNORM:
                case RGBA8I:
                case RGBA8UI:
                case SRGB8_ALPHA8:
                    return 4;
                // 48‑bit (RGB16) or 64‑bit (RGBA16)
                case RGB16:
                case RGB16_SNORM:
                case RGB16F:
                case RGB16I:
                case RGB16UI:
                    return 6;
                case RGBA16:
                case RGBA16_SNORM:
                case RGBA16F:
                case RGBA16I:
                case RGBA16UI:
                    return 8;
                // 96‑bit (RGB32) or 128‑bit (RGBA32)
                case RGB32F:
                case RGB32I:
                case RGB32UI:
                    return 12;
                case RGBA32F:
                case RGBA32I:
                case RGBA32UI:
                    return 16;
                // Special formats
                case RGB10A2_UNORM:
                case RGB10A2_UINT:
                    return 4; // 10+10+10+2 bits packed into 32 bits
                case RGB9E5:
                case R11G11B10F:
                    return 4; // packed shared‑exponent formats
                // Depth/stencil
                case Depth16:
                    return 2;
                case Depth24:
                case StencilIndex8:
                    return 3;
                case Depth32F:
                    return 4;
                case Depth24Stencil8:
                    return 4; // 24 depth + 8 stencil
                case Depth32FStencil8:
                    return 5; // 32 depth + 8 stencil (usually 64‑bit aligned)
                // Compressed formats return 0 (use GetCompressedBlockBytes)
                default:
                    return 0;
                }
            }

            /**
             * @brief Get the block size (in pixels) of a compressed format.
             * @param format The compressed texture format.
             * @return Block width/height (both equal), or 0 for uncompressed/unknown formats.
             * @note BC/DXT formats use 4×4 pixel blocks.
             */
            [[nodiscard]] constexpr uint32_t GetCompressedBlockSize(TextureFormat format) noexcept
            {
                if (IsCompressedFormat(format))
                {
                    return 4u; // All BC/DXT formats use 4×4 blocks
                }
                return 0u;
            }

            /**
             * @brief Get the number of bytes per block for a compressed format.
             * @param format The compressed texture format.
             * @return Bytes per block (for a 4×4 pixel block), or 0 for uncompressed/unknown formats.
             */
            [[nodiscard]] constexpr uint32_t GetCompressedBlockBytes(TextureFormat format) noexcept
            {
                using enum TextureFormat;
                switch (format)
                {
                // BC1 (DXT1): 64 bits (8 bytes) per 4×4 block
                case BC1:
                case BC1_SRGB:
                    return 8u;
                // BC2 (DXT3), BC3 (DXT5), BC4, BC5, BC6, BC7: 128 bits (16 bytes) per 4×4 block
                case BC2:
                case BC2_SRGB:
                case BC3:
                case BC3_SRGB:
                case BC4_UNORM:
                case BC4_SNORM:
                case BC5_UNORM:
                case BC5_SNORM:
                case BC6H_UF16:
                case BC6H_SF16:
                case BC7_UNORM:
                case BC7_SRGB:
                    return 16u;
                default:
                    return 0u;
                }
            }

        } // namespace utils
    } // namespace rhi
} // namespace hud_3d