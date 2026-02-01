/**
 * @file bit_ops.h
 * @brief Bit manipulation utilities for 3D HUD engine
 *
 * @details
 * Provides efficient bit-level operations including power-of-two checks,
 * alignment calculations, and other low-level bit manipulation utilities.
 * These functions are designed for maximum performance and are used extensively
 * in memory management, data structure alignment, and performance-critical code.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-17
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#include <cstddef>
#include <type_traits>
#include <limits>

namespace hud_3d
{
    namespace utils
    {
        namespace math
        {
            /**
             * @brief Check if a number is a power of two
             *
             * @param n The number to check
             * @return true if n is a power of two, false otherwise
             *
             * @details
             * Uses efficient bit manipulation: a power of two has exactly one bit set.
             * The expression `n & (n - 1)` clears the lowest set bit, so if the result
             * is zero and n is not zero, then n is a power of two.
             *
             * @note Zero is not considered a power of two.
             * @complexity O(1)
             */
            template <typename T>
            constexpr bool IsPowerOfTwo(T n) noexcept
            {
                static_assert(std::is_unsigned_v<T>, "T must be an unsigned integral type");
                return n && !(n & (n - 1));
            }

            /**
             * @brief Round up to the next power of two
             *
             * @param n The number to round up
             * @return The smallest power of two greater than or equal to n
             *
             * @details
             * For numbers that are already powers of two, returns the number itself.
             * For zero, returns 1 (smallest power of two).
             * Uses bit manipulation to efficiently find the next power of two.
             *
             * @complexity O(1) for built-in types
             */
            template <typename T>
            constexpr T NextPowerOfTwo(T n) noexcept
            {
                static_assert(std::is_unsigned_v<T>, "T must be an unsigned integral type");
                if (n == 0)
                    return 1;
                // For numbers that are already powers of two
                if (IsPowerOfTwo(n))
                    return n;
                // Decrement n to handle the case where n is already a power of two
                n--;
                // Propagate the highest set bit to all lower bits
                for (size_t i = 1; i < sizeof(T) * 8; i *= 2)
                {
                    n |= n >> i;
                }
                // The next power of two is one more than the result
                return n + 1;
            }

            /**
             * @brief Align a value up to the specified alignment
             *
             * @param value The value to align
             * @param alignment The alignment boundary (must be power of two)
             * @return The aligned value
             *
             * @details
             * Uses efficient bit manipulation: (value + alignment - 1) & ~(alignment - 1)
             * This rounds up to the next multiple of alignment.
             *
             * @pre alignment must be a power of two
             * @complexity O(1)
             */
            template <typename T>
            constexpr T AlignUp(T value, T alignment) noexcept
            {
                static_assert(std::is_unsigned_v<T>, "T must be an unsigned integral type");
                // Ensure alignment is a power of two for correct behavior
                if (!IsPowerOfTwo(alignment))
                {
                    alignment = NextPowerOfTwo(alignment);
                }
                return (value + alignment - 1) & ~(alignment - 1);
            }

            /**
             * @brief Align a value down to the specified alignment
             *
             * @param value The value to align
             * @param alignment The alignment boundary (must be power of two)
             * @return The aligned value
             *
             * @details
             * Uses efficient bit manipulation: value & ~(alignment - 1)
             * This rounds down to the previous multiple of alignment.
             *
             * @pre alignment must be a power of two
             * @complexity O(1)
             */
            template <typename T>
            constexpr T AlignDown(T value, T alignment) noexcept
            {
                static_assert(std::is_unsigned_v<T>, "T must be an unsigned integral type");
                // Ensure alignment is a power of two for correct behavior
                if (!IsPowerOfTwo(alignment))
                {
                    alignment = NextPowerOfTwo(alignment);
                }
                return value & ~(alignment - 1);
            }

            /**
             * @brief Check if a pointer is aligned to the specified boundary
             *
             * @param ptr The pointer to check
             * @param alignment The alignment boundary (must be power of two)
             * @return true if the pointer is aligned, false otherwise
             *
             * @complexity O(1)
             */
            template <typename T>
            constexpr bool IsAligned(const T *ptr, size_t alignment) noexcept
            {
                static_assert(std::is_unsigned_v<size_t>, "Alignment must be unsigned");
                // Ensure alignment is a power of two for correct behavior
                if (!IsPowerOfTwo(alignment))
                {
                    alignment = NextPowerOfTwo(alignment);
                }
                return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
            }

            /**
             * @brief Count the number of set bits (population count) in a number
             *
             * @param n The number to count bits in
             * @return The number of set bits
             *
             * @complexity O(1) for built-in types with hardware support
             */
            template <typename T>
            constexpr int32_t PopCount(T n) noexcept
            {
                static_assert(std::is_unsigned_v<T>, "T must be an unsigned integral type");
#if defined(__GNUC__) || defined(__clang__)
                if constexpr (sizeof(T) <= sizeof(unsigned int))
                {
                    return __builtin_popcount(static_cast<unsigned int>(n));
                }
                else if constexpr (sizeof(T) <= sizeof(unsigned long))
                {
                    return __builtin_popcountl(static_cast<unsigned long>(n));
                }
                else
                {
                    return __builtin_popcountll(static_cast<unsigned long long>(n));
                }

#elif defined(_MSC_VER)
                if constexpr (sizeof(T) == 1 || sizeof(T) == 2)
                {
                    return static_cast<int32_t>(__popcnt(static_cast<unsigned int>(n)));
                }
                else if constexpr (sizeof(T) == 4)
                {
                    return static_cast<int32_t>(__popcnt(static_cast<unsigned int>(n)));
                }
                else if constexpr (sizeof(T) == 8)
                {
#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64)
                    return static_cast<int32_t>(__popcnt64(static_cast<unsigned long long>(n)));
#else
                    uint32_t low = static_cast<uint32_t>(n & 0xFFFFFFFF);
                    uint32_t high = static_cast<uint32_t>(n >> 32);
                    return static_cast<int32_t>(__popcnt(low) + __popcnt(high));
#endif
                }
                else
                {
                    static_assert(sizeof(T) <= 8, "Unsupported type size");
                    return 0;
                }
#else
                int32_t count = 0;
                while (n)
                {
                    n &= (n - 1);
                    count++;
                }
                return count;
#endif
            }

            /**
             * @brief Find the position of the least significant set bit
             *
             * @param n The number to examine
             * @return The 0-based position of the least significant set bit, or -1 if no bits are set
             *
             * @complexity O(1)
             */
            template <typename T>
            constexpr int32_t FindLsb(T n) noexcept
            {
                static_assert(std::is_unsigned_v<T>, "T must be an unsigned integral type");
                if (n == 0)
                    return -1;
#if defined(__GNUC__) || defined(__clang__)
                if constexpr (sizeof(T) <= sizeof(unsigned int))
                {
                    return __builtin_ctz(static_cast<unsigned int>(n));
                }
                else if constexpr (sizeof(T) <= sizeof(unsigned long))
                {
                    return __builtin_ctzl(static_cast<unsigned long>(n));
                }
                else
                {
                    return __builtin_ctzll(static_cast<unsigned long long>(n));
                }

#elif defined(_MSC_VER)
                unsigned long index;
                if constexpr (sizeof(T) <= 4)
                {
                    if (_BitScanForward(&index, static_cast<unsigned long>(n)))
                    {
                        return static_cast<int32_t>(index);
                    }
                }
                else
                {
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64)
                    if (_BitScanForward64(&index, static_cast<unsigned __int64>(n)))
                    {
                        return static_cast<int32_t>(index);
                    }
#else
                    uint32_t low = static_cast<uint32_t>(n);
                    if (low != 0)
                    {
                        if (_BitScanForward(&index, low))
                        {
                            return static_cast<int32_t>(index);
                        }
                    }
                    uint32_t high = static_cast<uint32_t>(n >> 32);
                    if (_BitScanForward(&index, high))
                    {
                        return static_cast<int32_t>(index + 32);
                    }
#endif
                }

                return -1;

#else
                int32_t pos = 0;
                constexpr T one = static_cast<T>(1);
                while ((n & one) == 0)
                {
                    n >>= 1;
                    pos++;
                    if (pos >= static_cast<int32_t>(sizeof(T) * 8))
                    {
                        break;
                    }
                }
                return pos;
#endif
            }

            /**
             * @brief Find the position of the most significant set bit
             *
             * @param n The number to examine
             * @return The 0-based position of the most significant set bit, or -1 if no bits are set
             *
             * @complexity O(1)
             */
            template <typename T>
            constexpr int32_t FindMsb(T n) noexcept
            {
                static_assert(std::is_unsigned_v<T>, "T must be an unsigned integral type");

                if (n == 0)
                    return -1;

#if defined(__GNUC__) || defined(__clang__)
                if constexpr (sizeof(T) <= sizeof(unsigned int))
                {
                    constexpr int total_bits = sizeof(T) * 8;
                    return total_bits - 1 - __builtin_clz(static_cast<unsigned int>(n));
                }
                else if constexpr (sizeof(T) <= sizeof(unsigned long))
                {
                    constexpr int total_bits = sizeof(T) * 8;
                    return total_bits - 1 - __builtin_clzl(static_cast<unsigned long>(n));
                }
                else
                {
                    constexpr int total_bits = sizeof(T) * 8;
                    return total_bits - 1 - __builtin_clzll(static_cast<unsigned long long>(n));
                }

#elif defined(_MSC_VER)
                unsigned long index;
                if constexpr (sizeof(T) <= 4)
                {
                    if (_BitScanReverse(&index, static_cast<unsigned long>(n)))
                    {
                        return static_cast<int32_t>(index);
                    }
                }
                else
                {
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64)
                    if (_BitScanReverse64(&index, static_cast<unsigned __int64>(n)))
                    {
                        return static_cast<int32_t>(index);
                    }
#else
                    uint32_t high = static_cast<uint32_t>(n >> 32);
                    if (high != 0)
                    {
                        if (_BitScanReverse(&index, high))
                        {
                            return static_cast<int32_t>(index + 32);
                        }
                    }
                    uint32_t low = static_cast<uint32_t>(n);
                    if (_BitScanReverse(&index, low))
                    {
                        return static_cast<int32_t>(index);
                    }
#endif
                }
                return -1;
#else
                if (n == 0)
                    return -1;
                int32_t pos = 0;
                T temp = n;
                while (temp > 1)
                {
                    temp >>= 1;
                    pos++;
                    if (pos >= static_cast<int32_t>(sizeof(T) * 8) - 1)
                    {
                        break;
                    }
                }
                return pos;
#endif
            }

        } // namespace math
    } // namespace utils
} // namespace hud_3d