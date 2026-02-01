/**
 * @file log_manager_impl.cpp
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
 * @copyright Copyright (c) 2024 3D HUD Project
 *
 * @see ILogManager for the interface definition
 * @see ILogger for backend implementations
 */

#include "log_manager_impl.h"
#include "spd_logger.h"
#include "s_logger.h"
#include "external_logger.h"
#include <string>
#include <chrono>

namespace hud_3d
{
    namespace utils
    {
        ILogManager &ILogManager::Instance() noexcept
        {
            static LogManager instance;
            return instance;
        }

        LogManager::LogManager() noexcept
            : ILogManager(),
              logger_(),
              logger_keys_mutex_(),
              logger_keys_()
        {
        }

        LogManager::~LogManager() noexcept
        {
            Uninitialize();
        }

        bool LogManager::IsInitialized() const noexcept
        {
            return nullptr != logger_.get();
        }

        bool LogManager::Initialize(const LogConfiguration &config) noexcept
        {
            if (true == IsInitialized())
            {
                return false;
            }

            if (config.Holds<SpdLogConfiguration>())
            {
#ifdef __3D_HUD_SPD_LOGGER__
                logger_ = std::make_unique<SpdLogger>();
#else
                logger_.reset();
#endif // __3D_HUD_SPD_LOGGER__
            }
            else if (config.Holds<SlogConfiguration>())
            {
#ifdef __3D_HUD_S_LOGGER__
                logger_ = std::make_unique<SLogger>();
#else
                logger_.reset();
#endif // __3D_HUD_S_LOGGER__
            }
            else if (config.Holds<ExternalLogConfiguration>())
            {
#ifdef __3D_HUD_EXTERNAL_LOGGER__
                logger_ = std::make_unique<ExternalLogger>();
#else
                logger_.reset();
#endif // __3D_HUD_EXTERNAL_LOGGER__
            }
            if (nullptr == logger_.get())
            {
                return false;
            }

            if (false == logger_->Initialize(config))
            {
                logger_.reset();
                return false;
            }

            return true;
        }

        void LogManager::Uninitialize() noexcept
        {
            if (!IsInitialized())
            {
                return;
            }
            logger_->Uninitialize();
            logger_.reset();
        }

        void LogManager::SetMinimumLevel(const LogLevel level) noexcept
        {
            if (nullptr != logger_.get())
                logger_->SetMinimumLevel(level);
        }

        LogLevel LogManager::GetMinimumLevel() const noexcept
        {
            if (nullptr != logger_.get())
                return logger_->GetMinimumLevel();
            else
                return LogLevel::Invalid;
        }

        bool LogManager::IsThrottledLevel(LogLevel level) const noexcept
        {
            // Check if logger is available and level exceeds minimum threshold
            if (nullptr != logger_.get())
                return level < logger_->GetMinimumLevel();
            else
                return false;
        }

        bool LogManager::IsThrottledContent(int32_t freq, std::string_view file,
                                            int32_t line, std::string_view func) noexcept
        {
            // Validate input parameters and logger state
            if (file.empty() || line <= 0 ||
                func.empty() || nullptr == logger_.get())
            {
                return true; // Invalid parameters or uninitialized logger
            }

            // If frequency is non-positive, disable throttling
            if (freq <= 0)
            {
                return false;
            }

            // Generate unique key for this log location and acquire thread safety
            std::lock_guard<std::mutex> locker(logger_keys_mutex_);

            // CRITICAL BUG: Variable name inconsistency - logger_keys_ vs logger_keys_
            // Original code uses logger_keys_ for find but logger_keys_ for access
            const std::string key = std::string(file) + "_" + std::to_string(line) + "_" + std::string(func);
            const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::steady_clock::now().time_since_epoch())
                                    .count();

            auto it = logger_keys_.find(key);
            if (it == logger_keys_.end())
            {
                // First occurrence of this log location - record timestamp
                logger_keys_[key] = now;
                return false; // Allow logging
            }
            else
            {
                // Check time interval since last log at this location
                const int64_t interval = now - it->second;
                if (interval < freq)
                {
                    return true; // Throttle - too frequent
                }
                else
                {
                    // Update timestamp and allow logging
                    it->second = now;
                    return false;
                }
            }
        }

        bool LogManager::Write(std::string_view file, int32_t line,
                               std::string_view func, LogLevel level,
                               std::string_view module_name,
                               std::string_view content) noexcept
        {
            // Verify logger is properly initialized
            if (!IsInitialized())
            {
                return false;
            }

            // Validate all input parameters for correctness
            if (file.empty() || line <= 0 || func.empty() ||
                LogLevel::Invalid == level || module_name.empty() ||
                content.empty())
            {
                return false;
            }

            // Format log message with module name prefix
            std::string text;
            text += "[";
            text += module_name;
            text += "] - ";
            text += content;

            // Forward formatted message to underlying logger implementation
            return logger_->Write(file, line, func, level, text.c_str());
        }
    }
}