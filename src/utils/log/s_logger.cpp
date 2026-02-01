/**
 * @file s_logger.cpp
 * @brief Implementation of SLOG logger for QNX platform
 *
 * @details
 * This file implements the SLOG (System LOG) logger backend for QNX systems.
 * SLOG is a lightweight logging system designed for embedded and real-time
 * environments with minimal overhead and resource usage.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#ifdef __3D_HUD_S_LOGGER__

#include "s_logger.h"
#include "utils/string/string_utilities.h"
#include <process.h>
#include <sstream>

namespace hud_3d
{
    namespace utils
    {
        Slogger::Slogger() noexcept
            : ILogger(),
              min_log_level_(LogLevel::Invalid),
              buffer_(nullptr)
        {
        }

        Slogger::~Slogger() noexcept
        {
            Uninitialize();
        }

        bool Slogger::IsInitialized() const noexcept
        {
            return (LogLevel::Invalid != min_log_level_) &&
                   (nullptr != buffer_);
        }

        bool Slogger::Initialize(const LogConfiguration &config) noexcept
        {
            // Check if already initialized
            if (IsInitialized())
            {
                return false;
            }

            // Verify configuration type
            if (!config.Holds<SlogConfiguration>())
            {
                return false;
            }

            // Extract SLOG-specific configuration
            auto slog_config = config.Get<SlogConfiguration>();

            // Validate configuration parameters
            if (LogLevel::Invalid == slog_config.min_level ||
                slog_config.name.empty() ||
                slog_config.buffer_pages <= 0)
            {
                return false;
            }

            // Configure SLOG2 buffer
            slog2_buffer_set_config_t buffer_config;
            buffer_config.num_buffers = 1;
            buffer_config.buffer_set_name = slog_config.name.c_str();
            buffer_config.verbosity_level = SLOG2_DEBUG2;
            buffer_config.buffer_config[0].buffer_name = "3D_HUD";
            buffer_config.buffer_config[0].num_pages = slog_config.buffer_pages;
            buffer_config.max_retries = 0;

            // Register with SLOG2 system
            if (-1 == slog2_register(&buffer_config, &buffer_, 0))
            {
                return false;
            }

            min_log_level_ = slog_config.min_level;

            return true;
        }

        void Slogger::Uninitialize() noexcept
        {
            if (!IsInitialized())
            {
                return;
            }

            // Reset SLOG2 system
            slog2_reset();

            // Reset internal state
            buffer_ = nullptr;
            min_log_level_ = LogLevel::Invalid;
        }

        void Slogger::SetMinimumLevel(const LogLevel level) noexcept
        {
            // Update minimum log level for filtering
            min_log_level_ = level;
        }

        LogLevel Slogger::GetMinimumLevel() const noexcept
        {
            return min_log_level_;
        }

        bool Slogger::Write(std::string_view file, const int32_t line,
                            std::string_view func, const LogLevel level,
                            std::string_view text) noexcept
        {
            // Check if logger is properly initialized
            if (!IsInitialized())
            {
                return false;
            }

            // Map log level to SLOG2 severity and string representation
            uint8_t useverity = 0;
            std::string severity;
            switch (level)
            {
            case LogLevel::Trace:
            {
                severity = "[TRACE]";
                useverity = 7;
                break;
            }

            case LogLevel::Debug:
            {
                severity = "[DEBUG]";
                useverity = 6;
                break;
            }

            case LogLevel::Info:
            {
                severity = "[INFO]";
                useverity = 5;
                break;
            }

            case LogLevel::Warn:
            {
                severity = "[WARN]";
                useverity = 3;
                break;
            }

            case LogLevel::Error:
            {
                severity = "[ERR]";
                useverity = 2;
                break;
            }

            case LogLevel::Critical:
            {
                severity = "[CRITICAL]";
                useverity = 1;
                break;
            }

            case LogLevel::Off:
            {
                severity = "[OFF]";
                useverity = 0;
                break;
            }

            case LogLevel::Perf:
            {
                severity = "[PERF]";
                useverity = 5;
                break;
            }

            default:
                return false;
            };

            // Format log message with contextual information
            std::ostringstream oss;
            oss << "[" << hud_3d::utils::string::ExtractFilename(file)
                << ":" << line << "]"
                << "[" << func << "]"
                << "[" << gettid() << "]"
                << severity << text;
            std::string texts = oss.str();

            // Write to SLOG2 system
            slog2c(buffer_, 0, useverity, texts.c_str());

            return true;
        }
    }
}

#endif // __3D_HUD_S_LOGGER__