/**
 * @file log_manager.h
 * @brief Log management interface definition for 3D HUD Engine
 * @author He Yameng
 * @version 0.2
 * @date 2025-11-28
 * @copyright Copyright (c) 2023+
 *
 * @details
 * This header defines the singleton interface for managing the logging system in the 3D HUD Engine.
 * The ILogManager provides a comprehensive, thread-safe logging infrastructure with the following features:
 * - Singleton pattern for global access
 * - Type-safe variadic template logging (eliminates format string vulnerabilities)
 * - Multi-level log severity (Trace, Debug, Info, Warn, Error, Critical, Off, Performance)
 * - Module-based log categorization for better organization
 * - Frequency-based throttling to prevent log flooding
 * - Runtime log level filtering for dynamic verbosity control
 * - Thread-safe operations for concurrent access
 * - Exception-safe design with noexcept guarantees
 *
 * @note The logging system must be initialized before use via Initialize() method.
 * @see LogConfig, LogLevel for related configuration types.
 */

#pragma once

#include "utils/utils_define.h"
#include "fmt/format.h"

namespace hud_3d
{
    namespace utils
    {
        /**
         * @class ILogManager
         * @brief Singleton interface for comprehensive log management
         *
         * @details
         * ILogManager implements the Singleton pattern to provide global access to logging functionality.
         * It supports advanced features including type-safe template-based logging, frequency throttling,
         * and module-based categorization. The interface is designed for high-performance, thread-safe
         * logging operations in a 3D graphics environment.
         *
         * @warning This class is non-copyable and non-movable to enforce singleton semantics.
         * @see Instance() for obtaining the singleton instance.
         */
        class ILogManager
        {
        public:
            /**
             * @brief Virtual destructor for proper polymorphic destruction
             *
             * @details
             * Ensures proper cleanup of derived classes. The destructor is virtual to support
             * polymorphic usage through the ILogManager interface.
             *
             * @note This destructor is noexcept by default as per C++11 standard.
             */
            virtual ~ILogManager() = default;

        public:
            /**
             * @brief Get the singleton instance of the log manager
             *
             * Provides thread-safe access to the global log manager instance.
             * The instance is created on first call and persists for the lifetime
             * of the application.
             *
             * @return Reference to the singleton log manager instance
             * @note This method is thread-safe and exception-safe
             */
            static ILogManager &Instance() noexcept;

        public:
            /**
             * @brief Check if the logging system is initialized
             *
             * @details
             * Verifies that the logging system has been properly initialized and is ready
             * for logging operations. This check is essential before attempting to write
             * log entries to avoid undefined behavior.
             *
             * @return bool true if initialized and ready, false otherwise
             * @note Thread-safe and noexcept operation
             * @pre None (can be called at any time)
             * @post No state changes
             */
            virtual bool IsInitialized() const noexcept = 0;

            /**
             * @brief Initialize the logging system with the specified configuration
             *
             * @details
             * Sets up the logging infrastructure with the provided configuration.
             * This includes configuring log backends, setting up thread pools,
             * and preparing the logging pipeline. The method is idempotent - calling
             * it multiple times will reinitialize the system with the new configuration.
             *
             * @param[in] config LogConfig object containing initialization parameters
             * @return bool true if initialization succeeded, false on failure
             * @note Thread-safe operation with noexcept guarantee
             * @pre config must be a valid LogConfig object
             * @post Logging system is ready for use if return value is true
             * @see LogConfig for configuration details
             */
            virtual bool Initialize(const LogConfiguration &config) noexcept = 0;

            /**
             * @brief Deinitialize the logging system and release resources
             *
             * Stops all logging operations, flushes pending log entries, and releases
             * allocated resources. After calling this method, the logging system should
             * not be used until reinitialized.
             *
             * @note This method is thread-safe and noexcept
             */
            virtual void Uninitialize() noexcept = 0;

            /**
             * @brief Set the minimum log level for runtime filtering
             *
             * @details
             * Dynamically adjusts the logging verbosity by filtering out log entries
             * with severity lower than the specified level. This allows runtime
             * control over logging output without requiring system reinitialization.
             *
             * @param[in] level hud_3d::utils::LogLevel enumeration value for minimum severity threshold
             * @note Thread-safe operation with noexcept guarantee
             * @pre None (can be called at any time)
             * @post Log entries below the specified level are discarded
             * @see GetMinimumLevel() for retrieving the current threshold
             */
            virtual void SetMinimumLevel(LogLevel level) noexcept = 0;

            /**
             * @brief Get the current minimum log level threshold
             *
             * @details
             * Retrieves the current minimum severity level used for log filtering.
             * Log entries with severity below this threshold are automatically discarded.
             *
             * @return hud_3d::utils::LogLevel Current minimum severity threshold for log filtering
             * @note Thread-safe and noexcept operation
             * @pre None (can be called at any time)
             * @post No state changes
             * @see SetMinimumLevel() for setting the threshold
             */
            virtual LogLevel GetMinimumLevel() const noexcept = 0;

            /**
             * @brief Check if a log level is currently being throttled
             *
             * @details
             * Determines whether log messages at the specified severity level are
             * being rate-limited by the throttling mechanism. Throttling prevents
             * log flooding in high-frequency scenarios by suppressing duplicate
             * log entries within a configured time window.
             *
             * @param[in] level hud_3d::utils::LogLevel enumeration value to check for throttling
             * @return bool true if the log level is currently throttled, false otherwise
             * @note Thread-safe and noexcept operation
             * @pre Logging system must be initialized via Initialize()
             * @post No state changes
             * @see WriteWithThrottle() for throttled logging operations
             * @see SetThrottleConfig() for configuring throttling behavior
             */
            virtual bool IsThrottledLevel(LogLevel level) const noexcept = 0;

            /**
             * @brief Check if specific log content is currently being throttled
             *
             * @details
             * Performs fine-grained throttling checks based on log content location
             * and frequency. This method enables content-based rate limiting that
             * prevents identical log messages from the same source location from
             * flooding the log output within a specified time window.
             *
             * @param[in] freq Throttling frequency in milliseconds (<=0 disables throttling)
             * @param[in] file Source file name (typically __FILE__)
             * @param[in] line Source line number (typically __LINE__)
             * @param[in] func Function name (typically __FUNCTION__)
             * @return bool true if the content is currently throttled, false otherwise
             * @note Thread-safe and noexcept operation
             * @pre Logging system must be initialized via Initialize()
             * @post No state changes
             * @see WriteWithThrottle() for content-based throttled logging
             * @see IsThrottledLevel() for level-based throttling checks
             */
            virtual bool IsThrottledContent(int32_t freq, std::string_view file,
                                            int32_t line, std::string_view func) noexcept = 0;

            /**
             * @brief Write a log entry with optional frequency-based throttling
             *
             * @details
             * Core logging method that writes log entries with comprehensive metadata.
             * Supports optional frequency-based throttling to prevent log flooding
             * in high-frequency scenarios. When throttling is enabled, duplicate
             * log entries from the same source location are suppressed within the
             * specified time window.
             *
             * @param[in] file Source file name (typically __FILE__)
             * @param[in] line Source line number (typically __LINE__)
             * @param[in] func Function name (typically __FUNCTION__)
             * @param[in] level hud_3d::utils::LogLevel severity enumeration
             * @param[in] module_name Module identifier for categorization
             * @param[in] content Log message content
             * @note Thread-safe operation but may block during I/O operations
             * @pre Logging system must be initialized via Initialize()
             * @pre All string_view parameters must point to valid memory
             * @post Log entry is written to configured output destinations
             * @see WriteWithThrottle() for template-based throttled logging
             * @see IsThrottledContent() for content-based throttling checks
             */
            virtual bool Write(std::string_view file, int32_t line,
                               std::string_view func, LogLevel level,
                               std::string_view module_name,
                               std::string_view content) noexcept = 0;

        public:
            /**
             * @brief Write a log entry with type-safe variadic template parameters
             *
             * @details
             * Provides type-safe logging using C++ variadic templates instead of C-style
             * variadic arguments. This eliminates the risk of format string mismatches
             * and provides compile-time type checking. The method forwards the arguments
             * to the implementation using perfect forwarding.
             *
             * @tparam Args Template parameter pack for format arguments (automatically deduced)
             * @param[in] file Source file name (typically __FILE__)
             * @param[in] line Source line number (typically __LINE__)
             * @param[in] func Function name (typically __FUNCTION__)
             * @param[in] level hud_3d::utils::LogLevel severity enumeration
             * @param[in] module_name Module identifier string for categorization
             * @param[in] fmt Format string (printf-style syntax with type safety)
             * @param[in] args Variadic format arguments (forwarded to implementation)
             * @return bool true if log entry written successfully, false on failure
             * @note Thread-safe operation but may block during I/O operations
             * @pre Logging system must be initialized via Initialize()
             * @pre fmt must be compatible with provided argument types
             * @see WriteImpl() for the actual implementation
             */
            template <typename... Args>
            bool Write(std::string_view file, const int32_t line,
                       std::string_view func, const LogLevel level,
                       std::string_view module_name,
                       fmt::format_string<Args...> format,
                       Args &&...args) noexcept
            {
                return Write(-1, file, line, func, level, module_name,
                             format, std::forward<Args>(args)...);
            }

            /**
             * @brief Write a log entry with frequency-based throttling
             *
             * @details
             * Provides rate-limited logging to prevent log flooding in high-frequency scenarios.
             * Log entries are throttled based on the specified frequency parameter.
             * If throttling is active, duplicate log entries within the frequency window
             * are suppressed to reduce I/O overhead and log file size.
             *
             * @tparam Args Template parameter pack for format arguments (automatically deduced)
             * @param[in] freq Throttling frequency in milliseconds (<=0 disables throttling)
             * @param[in] file Source file name (typically __FILE__)
             * @param[in] line Source line number (typically __LINE__)
             * @param[in] func Function name (typically __FUNCTION__)
             * @param[in] level hud_3d::utils::LogLevel severity enumeration
             * @param[in] module_name Module identifier string for categorization
             * @param[in] fmt Format string (printf-style syntax with type safety)
             * @param[in] args Variadic format arguments (forwarded to implementation)
             * @return bool true if log entry written or throttled, false on error
             * @note Provides rate limiting to prevent excessive logging in tight loops
             * @pre Logging system must be initialized via Initialize()
             * @pre fmt must be compatible with provided argument types
             * @see WriteWithThrottleImpl() for the actual implementation
             */
            template <typename... Args>
            bool Write(const int32_t freq,
                       std::string_view file, const int32_t line,
                       std::string_view func, const LogLevel level,
                       std::string_view module_name,
                       fmt::format_string<Args...> format,
                       Args &&...args) noexcept
            {
                if (IsThrottledLevel(level) ||
                    IsThrottledContent(freq, file, line, func))
                {
                    return false;
                }

                std::string content;
                try
                {
                    content = fmt::format(format, std::forward<Args>(args)...);
                }
                catch (...)
                {
                    content = "Format error occurred.";
                }

                return Write(file, line, func, level, module_name, content);
            }

        protected:
            /**
             * @brief Default constructor (protected for Singleton pattern)
             *
             * @details
             * Protected constructor to enforce Singleton pattern semantics.
             * Only accessible to derived classes and the Instance() method.
             * This prevents direct instantiation and ensures proper Singleton usage.
             *
             * @note Defaulted to allow compiler-generated implementation
             * @warning Do not attempt to create instances directly
             * @see Instance() for proper Singleton access
             */
            ILogManager() = default;

        private: // Copying is prohibited
            ILogManager(ILogManager &&) = delete;
            ILogManager(const ILogManager &) = delete;

        private: // Assignment is prohibited
            ILogManager &operator=(ILogManager &&) = delete;
            ILogManager &operator=(const ILogManager &) = delete;
        };
    }
}

// Standard logging macros for SDK module using type-safe templates

#define MODULE_NAME_FOR_3D_HUD "3d_hud"

/**
 * @def LOG_3D_HUD_TRACE(fmt, ...)
 * @brief Log trace-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details Detailed tracing information for debugging SDK internals
 */
#define LOG_3D_HUD_TRACE(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Trace, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/**
 * @def LOG_3D_HUD_DEBUG(fmt, ...)
 * @brief Log debug-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details Debug information useful for SDK development and troubleshooting
 */
#define LOG_3D_HUD_DEBUG(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Debug, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/**
 * @def LOG_3D_HUD_INFO(fmt, ...)
 * @brief Log info-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details General information about SDK operations and state
 */
#define LOG_3D_HUD_INFO(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Info, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/**
 * @def LOG_3D_HUD_WARN(fmt, ...)
 * @brief Log warning-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details Warning messages indicating potential issues in SDK operations
 */
#define LOG_3D_HUD_WARN(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Warn, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/**
 * @def LOG_3D_HUD_ERROR(fmt, ...)
 * @brief Log error-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details Error conditions that affect SDK functionality but are recoverable
 */
#define LOG_3D_HUD_ERROR(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Error, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/**
 * @def LOG_3D_HUD_CRITICAL(fmt, ...)
 * @brief Log critical-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details Critical errors that may cause SDK failure or require immediate attention
 */
#define LOG_3D_HUD_CRITICAL(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Critical, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/**
 * @def LOG_3D_HUD_OFF(fmt, ...)
 * @brief Log off-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details Disables logging for the SDK module (log entries are discarded)
 */
#define LOG_3D_HUD_OFF(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Off, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/**
 * @def LOG_3D_HUD_PERF(fmt, ...)
 * @brief Log performance-level message for SDK module
 * @param[in] fmt Format string (printf-style syntax)
 * @param[in] ... Variable arguments for format string
 * @details Performance-related metrics and timing information for SDK operations
 */
#define LOG_3D_HUD_PERF(fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(__FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Trace, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

// Frequency-throttled logging macros for SDK module

/** @brief Log trace-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_TRACE(freq, fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Trace, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/** @brief Log debug-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_DEBUG(freq, fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Debug, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/** @brief Log info-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_INFO(freq, fmt, ...) \
    ILogManager::Instance().Write(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Info, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/** @brief Log warning-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_WARN(freq, fmt, ...) \
    hud_3d::utils::ILogManager::Instance().WriteWithThrottle(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Warn, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/** @brief Log error-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_ERROR(freq, fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Error, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/** @brief Log critical-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_CRITICAL(freq, fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Critical, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/** @brief Log off-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_OFF(freq, fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Off, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)

/** @brief Log performance-level message with frequency throttling for SDK module */
#define LOG_3D_HUD_FREQ_PERF(freq, fmt, ...) \
    hud_3d::utils::ILogManager::Instance().Write(freq, __FILE__, __LINE__, __FUNCTION__, hud_3d::utils::LogLevel::Trace, MODULE_NAME_FOR_3D_HUD, fmt, ##__VA_ARGS__)