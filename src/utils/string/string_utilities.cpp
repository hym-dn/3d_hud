/**
 * @file string_utilities.cpp
 * @brief Implementation of string utility functions for 3D HUD engine
 *
 * @details
 * This file contains the implementation of string manipulation utilities
 * declared in the corresponding header file.
 *
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#include "utils/string/string_utilities.h"
#include <algorithm>
#include <cctype>

namespace hud_3d
{
    namespace utils
    {
        namespace string
        {
            std::string_view ExtractFilename(std::string_view file_path) noexcept
            {
                // Find last directory separator (supports both / and \)
                size_t last_slash = file_path.find_last_of("/\\");
                if (last_slash != std::string_view::npos)
                {
                    return file_path.substr(last_slash + 1);
                }
                // If no separator found, return the original path
                return file_path;
            }

            std::string_view ExtractExtension(std::string_view filename) noexcept
            {
                size_t last_dot = filename.find_last_of('.');
                if (last_dot != std::string_view::npos && last_dot < filename.length() - 1)
                {
                    return filename.substr(last_dot);
                }
                return std::string_view{};
            }

            bool StartsWith(std::string_view str, std::string_view prefix) noexcept
            {
                return str.length() >= prefix.length() &&
                       str.compare(0, prefix.length(), prefix) == 0;
            }

            bool EndsWith(std::string_view str, std::string_view suffix) noexcept
            {
                return str.length() >= suffix.length() &&
                       str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
            }
            
            std::string_view TrimLeft(std::string_view str) noexcept
            {
                size_t start = str.find_first_not_of(" \t\n\r\f\v");
                return (start == std::string_view::npos) ? std::string_view{} : str.substr(start);
            }

            std::string_view TrimRight(std::string_view str) noexcept
            {
                size_t end = str.find_last_not_of(" \t\n\r\f\v");
                return (end == std::string_view::npos) ? std::string_view{} : str.substr(0, end + 1);
            }

            std::string_view Trim(std::string_view str) noexcept
            {
                return TrimLeft(TrimRight(str));
            }

            std::string ToLower(std::string_view str)
            {
                std::string result(str);
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c)
                               { return std::tolower(c); });
                return result;
            }

            std::string ToUpper(std::string_view str)
            {
                std::string result(str);
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c)
                               { return std::toupper(c); });
                return result;
            }

            std::vector<std::string_view> Split(std::string_view str, char delimiter)
            {
                std::vector<std::string_view> result;
                size_t start = 0;
                size_t end = str.find(delimiter);

                while (end != std::string_view::npos)
                {
                    result.emplace_back(str.substr(start, end - start));
                    start = end + 1;
                    end = str.find(delimiter, start);
                }

                // Add the last part
                if (start < str.length())
                {
                    result.emplace_back(str.substr(start));
                }

                return result;
            }

            bool Contains(std::string_view str, std::string_view substring) noexcept
            {
                return str.find(substring) != std::string_view::npos;
            }

            std::string ReplaceAll(std::string_view str, std::string_view from, std::string_view to)
            {
                std::string result(str);
                size_t start_pos = 0;
                while ((start_pos = result.find(from, start_pos)) != std::string::npos)
                {
                    result.replace(start_pos, from.length(), to);
                    start_pos += to.length();
                }
                return result;
            }

        } // namespace string
    } // namespace utils
} // namespace hud_3d