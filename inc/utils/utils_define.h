/**
 * @file utils_define.h
 * @brief Core utility module type definitions and common structures
 *
 * @details
 * This header file defines fundamental types and structures used throughout
 * the 3D HUD utility module. It serves as a central repository for common
 * enumerations, configuration structures, and utility types that are shared
 * across different utility components.
 *
 * Currently contains logging-related definitions, but designed to be extensible
 * for future utility components. The design emphasizes type safety through
 * modern C++ features like std::variant and std::function.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 *
 * @see utils namespace for other utility components
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <functional>
#include <variant>

namespace hud_3d
{
    namespace utils
    {
        /**
         * @brief Callback type for external log message handling
         *
         * @details
         * Defines the signature for custom log processing functions that can be
         * registered with the logging system. This allows integration with
         * third-party logging frameworks or custom logging implementations.
         *
         * @param logLevel The severity level of the log message (mapped to LogLevel enum)
         * @param message The log message content as a string view
         *
         * @note The handler must be thread-safe if used in multi-threaded environments.
         * @see ExternalLogConfiguration for usage example
         */
        using LogHandler = std::function<bool(const int32_t logLevel, const std::string_view message)>;

        /**
         * @enum LogLevel
         * @brief Enumeration of log severity levels
         *
         * @details
         * Defines the severity levels for log messages, ordered from most verbose (Trace)
         * to completely disabled (Off). Each level represents a different category of
         * log messages with specific use cases and filtering behavior.
         *
         * @note The enum uses int8_t as underlying type for efficient storage and serialization.
         *       Values are ordered by increasing severity for easy comparison.
         */
        enum class LogLevel : int8_t
        {
            Invalid = -1, ///< Invalid or uninitialized log level
            Trace,        ///< Detailed tracing information for debugging
            Debug,        ///< Debug information useful for development
            Info,         ///< General information about system operations
            Warn,         ///< Warning messages indicating potential issues
            Error,        ///< Error conditions that affect functionality
            Critical,     ///< Critical errors requiring immediate attention
            Perf,         ///< Performance metrics and timing information
            Off,          ///< Completely disables logging
        };

        /**
         * @struct SpdLogConfiguration
         * @brief Configuration parameters for SPDLOG backend
         *
         * @details
         * Contains specific configuration options for the SPDLOG logging backend.
         * This structure defines the core settings for log output destinations,
         * file management, and severity filtering.
         *
         * @warning Using C-style arrays for file_name may be less safe than std::string.
         *          Consider using std::string or std::array for better type safety.
         */
        struct SpdLogConfiguration
        {
            LogLevel min_level = LogLevel::Invalid; ///< Minimum log level to output (inclusive)
            bool to_console = false;                ///< Enable console output
            std::string file_name = "";             ///< Log file name (C-string, max 259 chars + null terminator)
            int32_t max_file_size = 0;              ///< Maximum log file size in bytes
            int32_t max_file_count = 0;             ///< Maximum number of log files to keep
        };

        /**
         * @struct SlogConfiguration
         * @brief Configuration parameters for SLOG backend
         *
         * @details
         * Contains specific configuration options for the SLOG (Simple LOG) backend.
         * SLOG is designed as a lightweight, simplified logging implementation
         * with minimal dependencies and overhead.
         *
         * @note The "num_page" parameter suggests a paged memory buffer design,
         *       which is typical for embedded or resource-constrained environments.
         */
        struct SlogConfiguration
        {
            LogLevel min_level = LogLevel::Invalid; ///< Minimum log level to output (inclusive)
            std::string name = "";                  ///< Logger instance name or identifier
            int32_t buffer_pages = 0;               ///< Number of memory pages for log buffer
        };

        /**
         * @struct ExternalLogConfiguration
         * @brief Configuration for external logging system integration
         *
         * @details
         * Provides a bridge to integrate external logging systems by allowing
         * custom log handling functions. This enables seamless integration with
         * third-party logging frameworks or custom logging implementations.
         *
         * @warning The log_func callback must be thread-safe if the logging system
         *          is used in multi-threaded environments.
         */
        struct ExternalLogConfiguration
        {
            LogLevel min_level = LogLevel::Invalid; ///< Minimum log level to forward to external system
            LogHandler handler = {};                ///< Custom log handling function
        };

        /**
         * @struct LogConfiguration
         * @brief Unified configuration container for all logging backends
         *
         * @details
         * Provides a type-safe, unified interface for configuring different logging
         * backends using std::variant. This design allows runtime selection of the
         * desired logging implementation while maintaining compile-time type safety.
         *
         * @note The variant-based approach eliminates the need for manual type checking
         *       and provides better error handling compared to union-based designs.
         * @see std::variant for type-safe discriminated union implementation
         */
        struct LogConfiguration
        {
            std::variant<SpdLogConfiguration, SlogConfiguration, ExternalLogConfiguration> configs; ///< Type-safe configuration container

            /**
             * @brief Default constructor
             * Constructs a LogConfiguration with default-initialized variant
             */
            LogConfiguration() = default;

            /**
             * @brief Template constructor for direct configuration assignment
             * @tparam T Configuration type (SpdLogConfiguration, SlogConfiguration, or ExternalLogConfiguration)
             * @param config Configuration object to initialize with
             */
            template <typename T>
            LogConfiguration(T &&config) : configs(std::forward<T>(config)) {}

            /**
             * @brief Default destructor for LogConfiguration
             *
             * @details
             * Ensures proper cleanup of the variant-based configuration container.
             * Automatically handles destruction of the active configuration object.
             */
            ~LogConfiguration() = default;

            /**
             * @brief Check if the configuration holds a specific backend type
             * @tparam T Configuration type to check for
             * @return true if the variant holds the specified type
             */
            template <typename T>
            bool Holds() const { return std::holds_alternative<T>(configs); }

            /**
             * @brief Get configuration for a specific backend type
             * @tparam T Configuration type to retrieve
             * @return Reference to the configuration object
             * @throws std::bad_variant_access if the variant doesn't hold the requested type
             */
            template <typename T>
            const T &Get() const { return std::get<T>(configs); }
            template <typename T>
            T &Get() { return std::get<T>(configs); }
        };

        /**
         * @brief Primary logger identifier for the 3D HUD engine.
         *
         * @details
         * This name is used when creating the main logger instance and appears
         * in log outputs. It should be unique within the application namespace
         * to prevent conflicts with other logging components.
         *
         * @value "3D_HUD"
         */
        inline constexpr std::string_view logger_name = "3D_HUD";
    }
}

// String concatenation macros for unique variable names
#ifndef HUD_3D_UTILS_CONCAT_IMPL
#define HUD_3D_UTILS_CONCAT_IMPL(x, y) x##y
#define HUD_3D_UTILS_CONCAT(x, y) HUD_3D_UTILS_CONCAT_IMPL(x, y)
#endif