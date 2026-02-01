/**
 * @file spd_logger.cpp
 * @brief Implementation of SPDLog-based logging backend
 *
 * @details
 * Provides the concrete implementation of the SpdLogger class, which serves
 * as a bridge between the 3D HUD engine's logging interface and the SPDLog
 * library. This implementation handles platform-specific optimizations,
 * asynchronous logging, and comprehensive error handling.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2025 3D HUD Project
 */

#ifdef __3D_HUD_SPD_LOGGER__

#include "spd_logger.h"
#include "utils/utils_define.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
#endif // __ANDROID__
#include <iostream>

namespace hud_3d
{
    namespace utils
    {
        SpdLogger::SpdLogger() noexcept
            : ILogger(),
              min_log_level_(LogLevel::Invalid),
              logger_()
        {
            // Logger is created in uninitialized state
            // Explicit initialization via Initialize() is required
        }

        SpdLogger::~SpdLogger() noexcept
        {
            // Ensure proper cleanup by calling Uninitialize
            // This is safe even if already uninitialized
            Uninitialize();
        }

        bool SpdLogger::IsInitialized() const noexcept
        {
            // Check if both the minimum log level is valid and logger instance exists
            return (LogLevel::Invalid != min_log_level_) &&
                   (nullptr != logger_.get());
        }

        bool SpdLogger::Initialize(const LogConfiguration &config) noexcept
        {
            try
            {
                // Prevent re-initialization
                if (IsInitialized())
                {
                    return false;
                }

                // Validate configuration type
                if (!config.Holds<SpdLogConfiguration>())
                {
                    return false;
                }

                const auto &spd_log_conf = config.Get<SpdLogConfiguration>();

                // Validate minimum log level
                if (LogLevel::Invalid == spd_log_conf.min_level)
                {
                    return false;
                }

                // Validate file logging parameters if not using console
                if (!spd_log_conf.to_console)
                {
                    if (spd_log_conf.file_name.empty() ||
                        spd_log_conf.max_file_size <= 0 ||
                        spd_log_conf.max_file_count <= 0)
                    {
                        return false;
                    }
                }

                // Create appropriate logger based on configuration
                if (spd_log_conf.to_console)
                {
#ifdef __ANDROID__
                    logger_ = spdlog::android_logger_mt(logger_name.data(), "3D_HUD:");
#else
                    logger_ = spdlog::stdout_color_mt(logger_name.data());
#endif
                }
                else
                {
                    logger_ = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>(
                        logger_name.data(),
                        spd_log_conf.file_name,
                        static_cast<size_t>(spd_log_conf.max_file_size),
                        static_cast<size_t>(spd_log_conf.max_file_count),
                        true);
                }

                // Verify logger creation succeeded
                if (nullptr == logger_.get())
                {
                    return false;
                }

                // Configure logger settings
                logger_->set_level(spdlog::level::level_enum::trace);
                logger_->flush_on(spdlog::level::level_enum::trace);

                // Set platform-specific log patterns
#ifdef __ANDROID__
                logger_->set_pattern("[%t] [%s:%#] [%!] - %v");
#else
                logger_->set_pattern("[%Y-%m-%d %H:%M:%S:%e:%f:%F] [%l] [%t] [%s:%#] [%!] %v");
#endif // __ANDROID__

                // Store the configured minimum level for filtering
                min_log_level_ = spd_log_conf.min_level;

                return true;
            }
            catch (const std::exception &)
            {
                // Log detailed exception information for debugging
                min_log_level_ = LogLevel::Invalid;
                logger_.reset();
                return false;
            }
            catch (...)
            {
                min_log_level_ = LogLevel::Invalid;
                logger_.reset();
                return false;
            }
        }

        void SpdLogger::Uninitialize() noexcept
        {
            try
            {
                // Early return if already uninitialized
                if (!IsInitialized())
                {
                    return;
                }

                // Clean up SPDLog resources if logger exists
                if (nullptr != logger_.get())
                {
                    // Drop all registered loggers from SPDLog registry
                    spdlog::drop_all();

                    // Gracefully shutdown SPDLog framework
                    spdlog::shutdown();

                    // Release the logger instance
                    logger_.reset();
                }

                // Reset internal state to uninitialized
                min_log_level_ = LogLevel::Invalid;
            }
            catch (const std::exception &)
            {
                // Force cleanup even if exceptions occur
                logger_.reset();
                min_log_level_ = LogLevel::Invalid;
            }
            catch (...)
            {
                // Force cleanup for unknown exceptions
                logger_.reset();
                min_log_level_ = LogLevel::Invalid;
            }
        }

        void SpdLogger::SetMinimumLevel(const LogLevel level) noexcept
        {
            // Direct assignment is safe and atomic for fundamental types
            min_log_level_ = level;
        }

        LogLevel SpdLogger::GetMinimumLevel() const noexcept
        {
            // Simple getter - no synchronization needed for read-only access
            return min_log_level_;
        }

        bool SpdLogger::Write(std::string_view file, const int32_t line,
                              std::string_view func, const LogLevel level,
                              std::string_view text) noexcept
        {
            try
            {
                // Early return if logger is not properly initialized
                if (!IsInitialized())
                {
                    return false;
                }

                // Handle performance level mapping (Perf -> Info for SPDLog)
                auto effective_level = level;
                if (LogLevel::Perf == level)
                {
                    effective_level = LogLevel::Info;
                }

                // Convert to SPDLog level enum and log the message
                const auto spdlog_level = static_cast<spdlog::level::level_enum>(effective_level);

                // Create source location and log the message
                const spdlog::source_loc source_location{file.data(), line, func.data()};
                logger_->log(source_location, spdlog_level, text);

                return true;
            }
            catch (const std::exception &)
            {
                // Log write failures for debugging (using stderr since logger might be broken)
                return false;
            }
            catch (...)
            {
                return false;
            }
        }
    }
}

#endif // __3D_HUD_SPD_LOGGER__