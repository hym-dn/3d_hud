/**
 * @file external_logger.cpp
 * @brief Implementation of external logging system integration
 *
 * This file implements the ExternalLogger class which provides integration
 * with external logging systems through configurable callback functions.
 * The implementation focuses on thread safety, error resilience, and
 * proper resource management.
 *
 * @version 1.0
 * @date 2025-11-30
 * @copyright Copyright (c) 2025 3D HUD Project
 */

#ifdef __3D_HUD_EXTERNAL_LOGGER__

#include "external_logger.h"
#include "utils/string/string_utilities.h"
#include <sstream>
#include <iostream>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace hud_3d
{
    namespace utils
    {
        ExternalLogger::ExternalLogger() noexcept
            : ILogger(),
              min_log_level_(LogLevel::Invalid),
              log_handler_()
        {
        }

        ExternalLogger::~ExternalLogger() noexcept
        {
            Uninitialize();
        }

        bool ExternalLogger::IsInitialized() const noexcept
        {
            // CRITICAL FIX: Original logic was inverted - should check if handler IS set
            return (LogLevel::Invalid != min_log_level_) &&
                   (static_cast<bool>(log_handler_));
        }

        bool ExternalLogger::Initialize(const LogConfiguration &config) noexcept
        {
            // Prevent re-initialization of already initialized logger
            if (IsInitialized())
            {
                return false;
            }

            // Verify configuration contains external logging settings
            if (!config.Holds<ExternalLogConfiguration>())
            {
                return false;
            }

            // Extract external logging configuration
            auto external_log_config = config.Get<ExternalLogConfiguration>();

            // Validate configuration parameters
            if (LogLevel::Invalid == external_log_config.min_level ||
                !static_cast<bool>(external_log_config.handler))
            {
                return false;
            }

            // Apply configuration to internal state
            min_log_level_ = external_log_config.min_level;
            log_handler_ = external_log_config.handler;

            return true;
        }

        void ExternalLogger::Uninitialize() noexcept
        {
            // Only perform cleanup if logger is actually initialized
            if (!IsInitialized())
            {
                return;
            }

            // Reset internal state to uninitialized values
            min_log_level_ = LogLevel::Invalid;
            log_handler_ = {};
        }

        void ExternalLogger::SetMinimumLevel(const LogLevel level) noexcept
        {
            min_log_level_ = level;
        }

        LogLevel ExternalLogger::GetMinimumLevel() const noexcept
        {
            return min_log_level_;
        }

        bool ExternalLogger::Write(std::string_view file, const int32_t line,
                                   std::string_view func, const LogLevel level,
                                   std::string_view text) noexcept
        {
            // Verify logger is properly initialized before processing
            if (!IsInitialized())
            {
                return false;
            }

            // Map log level to human-readable severity string
            std::string severity;
            switch (level)
            {
            case LogLevel::Trace:
                severity = "[TRACE]";
                break;
            case LogLevel::Debug:
                severity = "[DEBUG]";
                break;
            case LogLevel::Info:
                severity = "[INFO]";
                break;
            case LogLevel::Warn:
                severity = "[WARN]";
                break;
            case LogLevel::Error:
                severity = "[ERR]";
                break;
            case LogLevel::Critical:
                severity = "[CRITICAL]";
                break;
            case LogLevel::Off:
                severity = "[OFF]";
                break;
            case LogLevel::Perf:
                severity = "[PERF]";
                break;
            default:
                return false; // Invalid log level
            }

            // Format log message with comprehensive context information
            std::ostringstream oss;
            oss << "[" << hud_3d::utils::string::ExtractFilename(file)
                << ":" << line << "]"
                << "[" << func << "]"
#ifdef _MSC_VER
                << "[" << GetCurrentThreadId() << "]"
#else
                << "[" << pthread_self() << "]"
#endif // _MSC_VER
                << severity << text;

            std::string formatted_message = oss.str();

            // Forward formatted message to external log handler with exception safety
            try
            {
                // CRITICAL FIX: Original code had incorrect field name 'handler' instead of 'logger'
                log_handler_(static_cast<int32_t>(level), formatted_message);
                return true;
            }
            catch (const std::exception &e)
            {
                // Log the exception but don't propagate it (noexcept guarantee)
                return false;
            }
            catch (...)
            {
                // Catch any other exceptions to maintain noexcept guarantee
                return false;
            }
        }
    }
}

#endif // __3D_HUD_EXTERNAL_LOGGER__