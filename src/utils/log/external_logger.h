/**
 * @file external_logger.h
 * @brief External logging system integration implementation
 *
 * This header defines the ExternalLogger class which provides a bridge
 * between the 3D HUD logging system and external logging frameworks.
 * It allows integration with third-party logging systems through a
 * configurable callback interface.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-30
 * @copyright Copyright (c) 2025 3D HUD Project
 */

#ifdef __3D_HUD_EXTERNAL_LOGGER__

#pragma once

#include "logger.h"

namespace hud_3d
{
    namespace utils
    {
        /**
         * @brief External logging system integration implementation
         *
         * This class implements the ILogger interface by forwarding log messages
         * to an external logging system through a configurable callback function.
         * It provides seamless integration with third-party logging frameworks
         * while maintaining the standard 3D HUD logging interface.
         *
         * Key features:
         * - Configurable log level filtering
         * - Thread-safe callback execution
         * - Graceful error handling for external system failures
         * - Proper resource cleanup and state management
         *
         * @warning The external log handler must be thread-safe if used in
         *          multi-threaded environments. Callback exceptions are caught
         *          and logged internally to prevent system crashes.
         */
        class ExternalLogger : public ILogger
        {
        public:
            /**
             * @brief Construct an uninitialized external logger instance
             *
             * Creates a logger in uninitialized state. The logger must be
             * explicitly initialized with a valid configuration before use.
             */
            ExternalLogger() noexcept;

            /**
             * @brief Virtual destructor for proper cleanup
             *
             * Ensures proper resource cleanup by calling Uninitialize()
             * if the logger is still in initialized state.
             */
            virtual ~ExternalLogger() noexcept override;

        public:
            /**
             * @brief Check if logger is properly initialized and ready
             *
             * @return true if logger has valid configuration and handler
             * @return false if logger requires initialization
             */
            virtual bool IsInitialized() const noexcept override;

            /**
             * @brief Initialize logger with external system configuration
             *
             * @param config External logging configuration containing
             *               minimum log level and callback handler
             * @return true if initialization succeeded
             * @return false if configuration is invalid
             */
            virtual bool Initialize(const LogConfiguration &config) noexcept override;

            /**
             * @brief Clean up resources and reset logger to uninitialized state
             *
             * Releases all resources and resets internal state. This method
             * is idempotent and safe to call multiple times.
             */
            virtual void Uninitialize() noexcept override;

            /**
             * @brief Set minimum log level for message filtering
             *
             * @param level Minimum severity level to accept
             */
            virtual void SetMinimumLevel(const LogLevel level) noexcept override;

            /**
             * @brief Get current minimum log level configuration
             *
             * @return Current minimum log level for filtering
             */
            virtual LogLevel GetMinimumLevel() const noexcept override;

            /**
             * @brief Write log message to external logging system
             *
             * Formats the log message with context information and forwards
             * it to the configured external log handler. Messages below the
             * minimum log level are silently discarded.
             *
             * @param file Source file name
             * @param line Source line number
             * @param func Function/method name
             * @param level Log severity level
             * @param text Log message content
             * @return true if message was successfully forwarded
             * @return false if logging failed or message was filtered out
             */
            virtual bool Write(std::string_view file, const int32_t line,
                               std::string_view func, const LogLevel level,
                               std::string_view text) noexcept override;

        private:
            LogLevel min_log_level_; ///< Minimum log level for filtering
            LogHandler log_handler_; ///< External log message handler
        };
    }
}

#endif // __3D_HUD_EXTERNAL_LOGGER__