/**
 * @file spd_logger.h
 * @brief SPDLog-based logging backend implementation
 *
 * @details
 * Concrete implementation of the ILogger interface using the SPDLog library.
 * Provides high-performance, feature-rich logging with support for both
 * console and file outputs, asynchronous logging, and platform-specific
 * optimizations.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2025 3D HUD Project
 *
 * @see ILogger for the base interface definition
 * @see https://github.com/gabime/spdlog for SPDLog documentation
 */

#ifdef __3D_HUD_SPD_LOGGER__

#pragma once

#include "logger.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace hud_3d
{
    namespace utils
    {
        /**
         * @class SpdLogger
         * @brief SPDLog-based logging backend implementation
         *
         * @details
         * Implements the ILogger interface using the SPDLog library, providing
         * high-performance logging with features like asynchronous operations,
         * log rotation, and platform-specific optimizations. Designed for
         * production environments requiring robust logging capabilities.
         *
         * @note This implementation is thread-safe and exception-safe, with
         *       comprehensive error handling and resource management.
         */
        class SpdLogger final : public ILogger
        {
        public:
            /**
             * @brief Default constructor
             *
             * @details
             * Initializes the logger in an uninitialized state. The logger
             * must be explicitly initialized before use.
             */
            SpdLogger() noexcept;

            /**
             * @brief Virtual destructor
             *
             * @details
             * Ensures proper cleanup by automatically calling Uninitialize()
             * if the logger is still initialized.
             */
            virtual ~SpdLogger() noexcept override;

        public:
            /**
             * @brief Check if the logger is properly initialized
             * @return true if initialized and ready for logging operations
             * @return false if not initialized or in error state
             */
            virtual bool IsInitialized() const noexcept override;

            /**
             * @brief Initialize the logger with specified configuration
             * @param config Logging configuration parameters
             * @return true if initialization succeeded
             * @return false if initialization failed or configuration is invalid
             * @note This method is thread-safe and idempotent
             */
            virtual bool Initialize(const LogConfiguration &config) noexcept override;

            /**
             * @brief Uninitialize the logger and release resources
             * @note Safe to call multiple times, even if not initialized
             */
            virtual void Uninitialize() noexcept override;

            /**
             * @brief Set the minimum log level for output filtering
             * @param level Minimum log level to accept
             * @note Messages below this level will be silently ignored
             */
            virtual void SetMinimumLevel(const LogLevel level) noexcept override;

            /**
             * @brief Get the current minimum log level setting
             * @return Current minimum log level
             */
            virtual LogLevel GetMinimumLevel() const noexcept override;

            /**
             * @brief Write a log message to the configured output
             * @param file Source file name
             * @param line Source line number
             * @param func Function name
             * @param level Log severity level
             * @param text Log message content
             * @return true if message was successfully written
             * @return false if writing failed or logger is not initialized
             * @note Thread-safe and applies level filtering
             */
            virtual bool Write(std::string_view file, const int32_t line,
                               std::string_view func, const LogLevel level,
                               std::string_view text) noexcept override;

        private:
            LogLevel min_log_level_;                 ///< Minimum log level for filtering
            std::shared_ptr<spdlog::logger> logger_; ///< SPDLog logger instance
        };
    }
}

#endif // __3D_HUD_SPD_LOGGER__