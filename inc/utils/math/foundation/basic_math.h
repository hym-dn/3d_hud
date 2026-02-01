/**
 * @file basic_math.h
 * @brief Basic mathematical utilities for 3D HUD engine
 *
 * @details
 * Provides fundamental mathematical functions including min/max operations,
 * clamping, interpolation, and other basic arithmetic utilities.
 * These functions are designed for performance and correctness in real-time applications.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-17
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <type_traits>

namespace hud_3d
{
    namespace utils
    {
        namespace math
        {
            // Mathematical constants
            template <typename T>
            constexpr T PI = static_cast<T>(3.14159265358979323846);
            template <typename T>
            constexpr T TWO_PI = static_cast<T>(6.28318530717958647692);
            template <typename T>
            constexpr T HALF_PI = static_cast<T>(1.57079632679489661923);
            template <typename T>
            constexpr T INV_PI = static_cast<T>(0.31830988618379067154);
            template <typename T>
            constexpr T DEG_TO_RAD = PI<T> / static_cast<T>(180);
            template <typename T>
            constexpr T RAD_TO_DEG = static_cast<T>(180) / PI<T>;
            template <typename T>
            constexpr T EPSILON = std::numeric_limits<T>::epsilon();

            /**
             * @brief Convert degrees to radians
             */
            template <typename T>
            constexpr T DegreesToRadians(T degrees) noexcept
            {
                return degrees * DEG_TO_RAD<T>;
            }

            /**
             * @brief Convert radians to degrees
             */
            template <typename T>
            constexpr T RadiansToDegrees(T radians) noexcept
            {
                return radians * RAD_TO_DEG<T>;
            }

            /**
             * @brief Clamp a value between min and max
             */
            template <typename T>
            constexpr T Clamp(T value, T min_val, T max_val) noexcept
            {
                return (value < min_val) ? min_val : (value > max_val) ? max_val
                                                                       : value;
            }

            /**
             * @brief Linear interpolation between two values
             */
            template <typename T, typename U>
            constexpr T Lerp(T a, T b, U t) noexcept
            {
                return a + (b - a) * t;
            }

            /**
             * @brief Check if two floating-point numbers are approximately equal
             */
            template <typename T>
            constexpr bool ApproximatelyEqual(T a, T b, T epsilon = EPSILON<T>) noexcept
            {
                return std::abs(a - b) <= epsilon;
            }

            /**
             * @brief Check if a floating-point number is approximately zero
             */
            template <typename T>
            constexpr bool ApproximatelyZero(T value, T epsilon = EPSILON<T>) noexcept
            {
                return std::abs(value) <= epsilon;
            }

            /**
             * @brief Calculate the square of a number
             */
            template <typename T>
            constexpr T Square(T x) noexcept
            {
                return x * x;
            }

            /**
             * @brief Calculate the cube of a number
             */
            template <typename T>
            constexpr T Cube(T x) noexcept
            {
                return x * x * x;
            }

            /**
             * @brief Calculate the sign of a number
             */
            template <typename T>
            constexpr int Sign(T x) noexcept
            {
                return (T(0) < x) - (x < T(0));
            }

            /**
             * @brief Calculate the fractional part of a floating-point number
             */
            template <typename T>
            constexpr T FractionalPart(T x) noexcept
            {
                return x - std::floor(x);
            }

            /**
             * @brief Smooth step function (cubic interpolation)
             */
            template <typename T>
            constexpr T SmoothStep(T edge0, T edge1, T x) noexcept
            {
                x = Clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
                return x * x * (T(3) - T(2) * x);
            }

            /**
             * @brief Smoother step function (quintic interpolation)
             */
            template <typename T>
            constexpr T SmootherStep(T edge0, T edge1, T x) noexcept
            {
                x = Clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
                return x * x * x * (x * (x * T(6) - T(15)) + T(10));
            }

            /**
             * @brief Wrap a value to the range [0, max)
             */
            template <typename T>
            constexpr T Wrap(T value, T max) noexcept
            {
                if (max == T(0))
                    return T(0);

                T result = std::fmod(value, max);
                if (result < T(0))
                    result += max;
                return result;
            }

            /**
             * @brief Calculate the modulo operation with proper handling of negative numbers
             */
            template <typename T>
            constexpr T Mod(T a, T b) noexcept
            {
                static_assert(std::is_integral_v<T>, "T must be an integral type");

                if (b == T(0))
                    return T(0);

                T result = a % b;
                if (result < T(0))
                    result += b;
                return result;
            }

            /**
             * @brief Calculate the greatest common divisor of two numbers
             */
            template <typename T>
            constexpr T Gcd(T a, T b) noexcept
            {
                static_assert(std::is_integral_v<T>, "T must be an integral type");

                while (b != T(0))
                {
                    T temp = b;
                    b = a % b;
                    a = temp;
                }
                return a;
            }

            /**
             * @brief Calculate the least common multiple of two numbers
             */
            template <typename T>
            constexpr T Lcm(T a, T b) noexcept
            {
                static_assert(std::is_integral_v<T>, "T must be an integral type");

                if (a == T(0) || b == T(0))
                    return T(0);

                return (a / gcd(a, b)) * b;
            }
        } // namespace math
    } // namespace utils
} // namespace hud_3d