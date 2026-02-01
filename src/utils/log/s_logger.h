/**
 * @file s_logger.h
 * @brief SLOG logger header for QNX platform
 *
 * @details
 * This header defines the Slogger class which implements the ILogger
 * interface using QNX's SLOG2 (System LOG) system. SLOG2 is a lightweight
 * logging framework designed for embedded and real-time environments.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#ifdef __3D_HUD_S_LOGGER__

#pragma once

#include "logger.h"
#include <sys/slog2.h>
#include <mutex>
#include <list>
#include <string>
#include <thread>

namespace hud_3d
{
    namespace utils
    {
        /**
         * @brief SLOG logger implementation for QNX platform
         *
         * This class provides logging functionality using QNX's SLOG2 system.
         * It supports log level filtering, thread-safe operations, and
         * efficient logging suitable for embedded systems.
         */
        class Slogger : public ILogger
        {
        public:
            /**
             * @brief Default constructor
             */
            Slogger() noexcept;

            /**
             * @brief Virtual destructor
             */
            virtual ~Slogger() noexcept override;

        public:
            /**
             * @brief Check if logger is initialized
             * @return true if logger is ready for use
             */
            virtual bool IsInitialized() const noexcept override;

            /**
             * @brief Initialize logger with configuration
             * @param config Log configuration object
             * @return true if initialization succeeded
             */
            virtual bool Initialize(const LogConfiguration &config) noexcept override;

            /**
             * @brief Uninitialize and release resources
             */
            virtual void Uninitialize() noexcept override;

            /**
             * @brief Set minimum log level for filtering
             * @param level Minimum level to log
             */
            virtual void SetMinimumLevel(const LogLevel level) noexcept override;

            /**
             * @brief Get current minimum log level
             * @return Current minimum log level
             */
            virtual LogLevel GetMinimumLevel() const noexcept override;

            /**
             * @brief Write a log message
             * @param file Source file name
             * @param line Line number in source file
             * @param func Function name
             * @param level Log severity level
             * @param text Log message content
             * @return true if message was logged successfully
             */
            virtual bool Write(std::string_view file, const int32_t line,
                               std::string_view func, const LogLevel level,
                               std::string_view text) noexcept override;

        private:
            LogLevel min_log_level_; ///< Minimum log level for filtering
            slog2_buffer_t buffer_;  ///< SLOG2 buffer handle
        };
    }
}

#endif // __3D_HUD_S_LOGGER__