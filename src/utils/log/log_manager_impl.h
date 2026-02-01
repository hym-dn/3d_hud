/**
 * @file log_manager_impl.h
 * @brief Concrete implementation of the logging manager interface
 *
 * @details
 * Provides the actual implementation of the ILogManager interface with
 * thread-safe logging operations, throttling mechanisms, and multiple
 * backend support through the ILogger interface.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2025 3D HUD Project
 *
 * @see ILogManager for the interface definition
 * @see ILogger for backend implementations
 */

#pragma once

#include "logger.h"
#include "utils/log/log_manager.h"
#include <mutex>
#include <memory>
#include <unordered_map>
#include <cstdint>

namespace hud_3d
{
    namespace utils
    {
        /**
         * @class LogManager
         * @brief Concrete logging manager implementation
         *
         * @details
         * Implements the ILogManager interface with thread-safe operations,
         * content throttling, and configurable logging backends. Uses RAII
         * for resource management and provides noexcept guarantees for
         * critical operations.
         */
        class LogManager final : public ILogManager
        {
        public:
            /// Smart pointer type for logger backend management
            using LoggerPointer = std::unique_ptr<ILogger>;

        public:
            /**
             * @brief Default constructor
             * @note Initializes with default configuration and null logger
             */
            LogManager() noexcept;

            /**
             * @brief Virtual destructor for proper polymorphic destruction
             */
            virtual ~LogManager() noexcept override;

        public:
            /**
             * @brief Check if the logging system is initialized
             * @return true if initialized and ready for logging
             * @return false if not initialized or in error state
             */
            virtual bool IsInitialized() const noexcept override;

            /**
             * @brief Initialize the logging system with specified configuration
             * @param config Logging configuration parameters
             * @return true if initialization succeeded
             * @return false if initialization failed
             * @note This method is thread-safe and idempotent
             */
            virtual bool Initialize(const LogConfiguration &config) noexcept override;

            /**
             * @brief Uninitialize the logging system and release resources
             * @note Safe to call multiple times, even if not initialized
             */
            virtual void Uninitialize() noexcept override;

            /**
             * @brief Set the minimum log level for output filtering
             * @param level Minimum log level to accept
             * @note Messages below this level will be silently ignored
             */
            virtual void SetMinimumLevel(LogLevel level) noexcept override;

            /**
             * @brief Get the current minimum log level
             * @return Current minimum log level setting
             */
            virtual LogLevel GetMinimumLevel() const noexcept override;

            /**
             * @brief Check if a log level should be throttled
             * @param level Log level to check
             * @return true if level is below minimum threshold
             * @return false if level should be processed
             */
            virtual bool IsThrottledLevel(LogLevel level) const noexcept override;

            /**
             * @brief Check if specific log content should be throttled
             * @param freq Throttle frequency in milliseconds
             * @param file Source file name
             * @param line Source line number
             * @param func Function name
             * @return true if content should be throttled
             * @return false if content should be logged
             * @note Uses content-based hashing for throttling decisions
             */
            virtual bool IsThrottledContent(int32_t freq, std::string_view file,
                                            int32_t line, std::string_view func) noexcept override;

            /**
             * @brief Write a log message to the configured backend
             * @param file Source file name
             * @param line Source line number
             * @param func Function name
             * @param level Log severity level
             * @param module_name Module or category name
             * @param content Log message content
             * @return true if message was successfully written
             * @return false if writing failed or message was filtered
             * @note Thread-safe and applies all filtering/throttling rules
             */
            virtual bool Write(std::string_view file, int32_t line,
                               std::string_view func, LogLevel level,
                               std::string_view module_name,
                               std::string_view content) noexcept override;

        private:
            LoggerPointer logger_;
            mutable std::mutex logger_keys_mutex_;
            std::unordered_map<std::string, int64_t> logger_keys_;
        };
    }
}