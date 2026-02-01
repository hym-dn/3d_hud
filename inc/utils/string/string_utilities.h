/**
 * @file string_utilities.h
 * @brief String utility functions for 3D HUD engine
 * 
 * @details
 * This header provides common string manipulation utilities used throughout
 * the 3D HUD engine. Functions are designed to be efficient, cross-platform,
 * and easy to use with modern C++ standards.
 * 
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace hud_3d
{
    namespace utils
    {
        namespace string
        {
            /**
             * @brief Extract filename from full file path
             * 
             * @details
             * Extracts the filename portion from a full file path, supporting
             * both Windows ('\\') and Unix ('/') path separators.
             * 
             * @param file_path Full file path
             * @return std::string_view containing only the filename portion
             * 
             * @example
             * ExtractFilename("d:/Work/3d_hud/src/utils/log/slog_logger.cpp")
             * returns "slog_logger.cpp"
             */
            std::string_view ExtractFilename(std::string_view file_path) noexcept;

            /**
             * @brief Extract file extension from filename
             * 
             * @param filename Filename or path
             * @return std::string_view containing the file extension (including dot)
             */
            std::string_view ExtractExtension(std::string_view filename) noexcept;

            /**
             * @brief Check if string starts with a given prefix
             * 
             * @param str String to check
             * @param prefix Prefix to look for
             * @return true if string starts with prefix
             */
            bool StartsWith(std::string_view str, std::string_view prefix) noexcept;

            /**
             * @brief Check if string ends with a given suffix
             * 
             * @param str String to check
             * @param suffix Suffix to look for
             * @return true if string ends with suffix
             */
            bool EndsWith(std::string_view str, std::string_view suffix) noexcept;

            /**
             * @brief Trim whitespace from the beginning of a string
             * 
             * @param str String to trim
             * @return std::string_view with leading whitespace removed
             */
            std::string_view TrimLeft(std::string_view str) noexcept;

            /**
             * @brief Trim whitespace from the end of a string
             * 
             * @param str String to trim
             * @return std::string_view with trailing whitespace removed
             */
            std::string_view TrimRight(std::string_view str) noexcept;

            /**
             * @brief Trim whitespace from both ends of a string
             * 
             * @param str String to trim
             * @return std::string_view with both leading and trailing whitespace removed
             */
            std::string_view Trim(std::string_view str) noexcept;

            /**
             * @brief Convert string to lowercase
             * 
             * @param str String to convert
             * @return std::string Lowercase version of input string
             */
            std::string ToLower(std::string_view str);

            /**
             * @brief Convert string to uppercase
             * 
             * @param str String to convert
             * @return std::string Uppercase version of input string
             */
            std::string ToUpper(std::string_view str);

            /**
             * @brief Split string by delimiter
             * 
             * @param str String to split
             * @param delimiter Delimiter character
             * @return std::vector<std::string_view> Vector of string parts
             */
            std::vector<std::string_view> Split(std::string_view str, char delimiter);

            /**
             * @brief Check if string contains a substring
             * 
             * @param str String to search in
             * @param substring Substring to search for
             * @return true if substring is found
             */
            bool Contains(std::string_view str, std::string_view substring) noexcept;
            
            /**
             * @brief Replace all occurrences of a substring
             * 
             * @param str Original string
             * @param from Substring to replace
             * @param to Replacement substring
             * @return std::string String with replacements
             */
            std::string ReplaceAll(std::string_view str, std::string_view from, std::string_view to);

        } // namespace string
    } // namespace utils
} // namespace hud_3d