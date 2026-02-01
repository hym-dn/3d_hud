/**
 * @file logger.h
 * @author He Yameng
 * @brief Cross-platform logging system interface definition
 *
 * This header defines the abstract interface for a thread-safe, extensible logging system
 * that supports multiple backend implementations (SPD log, Slog, External). The system
 * provides configurable log levels, file rotation, and robust error handling suitable
 * for production environments.
 *
 * @author Yameng.He
 * @version 0.1
 * @date 2025-11-28
 * @copyright Copyright (c) 2025+
 */

#pragma once

#include "utils/utils_define.h"
#include <cstdint>
#include <string_view>

namespace hud_3d
{
    namespace utils
    {
        /**
         * @brief Abstract interface for thread-safe, extensible logging system
         *
         * This interface defines a contract for logging implementations that must provide
         * robust, thread-safe logging capabilities across multiple output targets including
         * files, console, and network streams. Implementations are expected to handle
         * configuration validation, resource management, and graceful error recovery.
         *
         * Key design principles:
         * - Thread safety: All methods must be safe for concurrent access
         * - Exception safety: noexcept guarantees for predictable behavior
         * - Resource safety: Proper cleanup and leak prevention
         * - Performance: Minimal overhead in production configurations
         *
         * @warning This interface is non-copyable and non-movable to prevent
         *          accidental state sharing between logger instances.
         */
        class ILogger
        {
        public:
            /**
             * @brief Virtual destructor for polymorphic cleanup
             *
             * Ensures proper destruction of derived logger implementations and cleanup
             * of allocated resources. Derived classes should override this destructor
             * to perform implementation-specific cleanup.
             */
            virtual ~ILogger() = default;

        public:
            /**
             * @brief Query logger initialization state
             *
             * Determines whether the logger has been successfully initialized and is
             * ready to accept log entries. This method should be called before any
             * logging operations to ensure proper system state.
             *
             * @return true if logger is initialized and operational
             * @return false if logger requires initialization or has been shut down
             */
            virtual bool IsInitialized() const noexcept = 0;

            /**
             * @brief Initialize logging system with specified configuration
             *
             * Configures the logger backend according to the provided parameters.
             * This method performs validation of the configuration, allocates necessary
             * resources, and prepares the logging system for operation.
             *
             * @param config Complete logging configuration structure
             * @return true if initialization completed successfully
             * @return false if configuration is invalid or resources unavailable
             *
             * @throws No exceptions (noexcept guarantee)
             */
            virtual bool Initialize(const LogConfiguration &config) noexcept = 0;

            /**
             * @brief Clean up logging resources and shutdown logger
             *
             * Releases all allocated resources, closes log files, and prepares the
             * logger for destruction. This method is idempotent - multiple calls
             * should have the same effect as a single call.
             *
             * @note After calling Uninit(), the logger should return to an uninitialized
             *       state where IsInited() returns false.
             */
            virtual void Uninitialize() noexcept = 0;

            /**
             * @brief Configure minimum log level for entry filtering
             *
             * Sets the threshold severity level for log entry acceptance. Entries with
             * severity levels below the specified threshold will be silently discarded.
             * This allows dynamic adjustment of logging verbosity at runtime.
             *
             * @param level Minimum severity level to accept (inclusive)
             *
             * @pre level must be a valid LogLevel enumeration value
             */
            virtual void SetMinimumLevel(const LogLevel level) noexcept = 0;

            /**
             * @brief Retrieve current minimum log level configuration
             *
             * Returns the currently configured minimum severity level for log entry
             * filtering. This value can be used to optimize log generation by avoiding
             * the construction of log messages that would be filtered out.
             *
             * @return Current minimum log level for filtering
             */
            virtual LogLevel GetMinimumLevel() const noexcept = 0;

            /**
             * @brief Write log entry with comprehensive context information
             *
             * Records a log message with associated metadata including source location,
             * severity level, and timestamp. The implementation must handle concurrent
             * calls safely and provide appropriate buffering or synchronization.
             *
             * @param file Source file name (typically obtained via __FILE__ macro)
             * @param line Source line number (typically obtained via __LINE__ macro)
             * @param func Function or method name (typically obtained via __func__)
             * @param level Severity level of the log entry
             * @param text Log message content
             *
             * @return true if log entry was successfully recorded
             * @return false if logging failed due to invalid parameters or system error
             *
             * @pre file, func, and text must reference valid null-terminated strings
             * @pre line must be a positive integer value
             * @pre level must be a valid LogLevel enumeration value
             */
            virtual bool Write(std::string_view file, const int32_t line,
                               std::string_view func, const LogLevel level,
                               std::string_view text) noexcept = 0;

        protected:
            /**
             * @brief Default constructor
             *
             * Constructs an uninitialized logger instance. The logger must be explicitly
             * initialized using the Init() method before any logging operations can be
             * performed.
             */
            ILogger() = default;

        private:
            // Explicitly delete copy and move operations to enforce singleton-like behavior
            ILogger(const ILogger &) = delete; ///< Copy construction prohibited
            ILogger(ILogger &&) = delete;      ///< Move construction prohibited

        private:
            ILogger &operator=(const ILogger &) = delete; ///< Copy assignment prohibited
            ILogger &operator=(ILogger &&) = delete;      ///< Move assignment prohibited
        };
    }
}