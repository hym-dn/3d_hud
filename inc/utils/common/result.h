/**
 * @file result.h
 * @brief Type-safe, functional error handling for C++ inspired by Rust's Result type.
 *
 * This file provides the Result<T> template class and its void specialization,
 * offering a modern, type-safe approach to error handling without using exceptions
 * for control flow. It supports functional programming patterns like monadic bind
 * (AndThen), mapping (Map), error recovery (OrElse), and side effects (InspectError).
 *
 * @section Features
 *
 * - Type-safe error handling: Errors cannot be accidentally ignored
 * - Functional programming patterns: AndThen, Map, OrElse, InspectError
 * - Value semantics: Fully copyable and movable
 * - Two-state model: Success (with value) or Error/Warning (with message and code)
 * - Exception conversion: Use Expect() or Unwrap() when exceptions are needed
 * - Template specialization: Dedicated void specialization for operations without return values
 *
 * @section Design Philosophy
 *
 * Result<T> is designed around these core principles:
 *
 * 1. **Explicit error handling**: Errors must be explicitly handled, they cannot be ignored
 * 2. **Type safety**: The type system prevents accessing values from error states
 * 3. **Functional composition**: Operations can be chained without nested if-else blocks
 * 4. **Zero-cost abstraction**: No runtime overhead compared to traditional error handling
 * 5. **Modern C++**: Leverages C++11/14 features like move semantics, type traits, and perfect forwarding
 *
 * @section Comparison with Exceptions
 *
 * Traditional exception-based error handling:
 * @code
 * try {
 *     int value = MightThrow();
 *     Process(value);
 * } catch (const std::exception& e) {
 *     HandleError(e.what());
 * }
 * @endcode
 *
 * Result-based error handling:
 * @code
 * Result<int> result = MightFail();
 * if (!result) {
 *     HandleError(result.GetError());
 *     return;
 * }
 * Process(result.GetValue());
 * @endcode
 *
 * Functional chaining with Result:
 * @code
 * auto result = MightFail()
 *     .AndThen([](int value) { return Process(value); })
 *     .AndThen([](ProcessedData data) { return Save(data); });
 *
 * if (!result) {
 *     HandleError(result.GetError());
 * }
 * @endcode
 *
 * @section Best Practices
 *
 * 1. Use factory functions (Success(), Error(), Warning()) instead of constructors
 * 2. Check IsSuccess() or use operator bool() before accessing values
 * 3. Use AndThen() for chaining operations that might fail
 * 4. Use Map() for transforming values that cannot fail
 * 5. Use OrElse() for error recovery and fallback logic
 * 6. Use InspectError() for logging and side effects
 * 7. Convert to exceptions only at API boundaries using Expect() or Unwrap()
 *
 * @section Thread Safety
 *
 * Result<T> is not thread-safe by itself. Concurrent access to the same Result
 * object from multiple threads requires external synchronization. However, different
 * Result objects can be safely used in different threads.
 *
 * @section Performance
 *
 * - Move semantics minimize copies
 * - Small buffer optimization for stored values
 * - No heap allocations for error messages (use SSO)
 * - Inlined operations for better performance
 *
 * @authors Yameng.He
 * @date 2026-01-28
 * @copyright 3D HUD Project
 */

#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace hud_3d
{
    namespace utils
    {
        /**
         * @enum ResultType
         * @brief Enumeration of possible result states for the Result class.
         *
         * The ResultType enum defines three possible states:
         * - Success: Operation completed successfully
         * - Warning: Operation completed with warnings
         * - Error: Operation failed with an error
         */
        enum class ResultType
        {
            Success, ///< Operation completed successfully
            Warning, ///< Operation completed with warnings
            Error    ///< Operation failed with an error
        };

        /**
         * @class Result<T>
         * @brief A type-safe, functional error handling class similar to Rust's Result type.
         *
         * Result<T> is a template class that represents either a successful value of type T
         * or an error with associated message and error code. It provides a modern, functional
         * approach to error handling without using exceptions for control flow.
         *
         * @tparam T The type of the successful value. Can be any copyable/movable type.
         *
         * @section Usage Examples
         *
         * Basic usage:
         * @code
         * Result<int> Divide(int a, int b) {
         *     if (b == 0) {
         *         return Result<int>::Error("Division by zero", -1);
         *     }
         *     return Result<int>::Success(a / b);
         * }
         *
         * auto result = Divide(10, 2);
         * if (result.IsSuccess()) {
         *     std::cout << "Result: " << result.GetValue() << std::endl;
         * }
         * @endcode
         *
         * Functional chaining with AndThen:
         * @code
         * auto result = ReadFile("data.txt")
         *     .AndThen([](std::string content) { return ParseJSON(content); })
         *     .AndThen([](JsonValue json) { return ProcessData(json); });
         *
         * if (!result) {
         *     std::cerr << "Error: " << result.GetError() << std::endl;
         * }
         * @endcode
         *
         * @section Design Principles
         *
         * - Type safety: Errors cannot be ignored accidentally
         * - Value semantics: Copyable and movable
         * - Functional style: Chain operations with AndThen, Map, OrElse
         * - No exceptions for control flow: Use Expect or Unwrap when exceptions are desired
         * - RAII compliant: Proper resource management
         */
        template <typename T>
        class Result final
        {
        public:
            /**
             * @brief Default constructor. Creates an error Result with "Uninitialized Result" message.
             *
             * The default constructor initializes the Result to an error state. This encourages
             * explicit initialization and prevents accidental use of uninitialized results.
             */
            Result() = default;

            /**
             * @brief Move constructor.
             * @param other The Result to move from
             */
            Result(Result &&) = default;

            /**
             * @brief Copy constructor.
             * @param other The Result to copy from
             */
            Result(const Result &) = default;

            /**
             * @brief Destructor.
             */
            ~Result() = default;

            /**
             * @brief Move assignment operator.
             * @param other The Result to move from
             * @return Reference to this Result
             */
            Result &operator=(Result &&) = default;

            /**
             * @brief Copy assignment operator.
             * @param other The Result to copy from
             * @return Reference to this Result
             */
            Result &operator=(const Result &) = default;

            /**
             * @brief Creates a successful Result with the given value.
             * @param value The successful value to store (will be moved)
             * @return A Result in success state containing the value
             *
             * @note The value is moved into the Result, not copied.
             * @note This function is noexcept because it cannot fail.
             *
             * @code
             * Result<std::string> result = Result<std::string>::Success("Hello");
             * @endcode
             */
            static Result Success(T &&value) noexcept
            {
                return Result(std::move(value));
            }

            /**
             * @brief Creates an error Result with the given message and error code.
             * @param message Error message describing what went wrong
             * @param code Error code (default: -1)
             * @return A Result in error state
             *
             * @code
             * Result<int> result = Result<int>::Error("File not found", ENOENT);
             * @endcode
             */
            static Result Error(std::string_view message, int32_t code = -1) noexcept
            {
                return Result(message, code, ResultType::Error);
            }

            /**
             * @brief Creates a warning Result with the given message and warning code.
             * @param message Warning message
             * @param code Warning code (default: -1)
             * @return A Result in warning state
             *
             * @note Warning results are still considered successful for boolean evaluation.
             * Use IsWarning() to check specifically for warnings.
             *
             * @code
             * Result<int> result = Result<int>::Warning("Deprecated API used", 100);
             * if (result.IsWarning()) {
             *     LogWarning(result.GetError());
             * }
             * @endcode
             */
            static Result Warning(std::string_view message, int32_t code = -1) noexcept
            {
                return Result(message, code, ResultType::Warning);
            }

            /**
             * @brief Checks if the Result is in success state.
             * @return true if success, false otherwise
             *
             * @code
             * if (result.IsSuccess()) {
             *     UseValue(result.GetValue());
             * }
             * @endcode
             */
            bool IsSuccess() const noexcept
            {
                return type_ == ResultType::Success;
            }

            /**
             * @brief Checks if the Result is in error state.
             * @return true if error, false otherwise
             *
             * @code
             * if (result.IsError()) {
             *     HandleError(result.GetError(), result.GetErrorCode());
             * }
             * @endcode
             */
            bool IsError() const noexcept
            {
                return type_ == ResultType::Error;
            }

            /**
             * @brief Checks if the Result is in warning state.
             * @return true if warning, false otherwise
             *
             * @note Warning results are still considered successful for boolean evaluation
             * and operator bool().
             *
             * @code
             * if (result.IsWarning()) {
             *     LogWarning(result.GetError());
             * }
             * @endcode
             */
            bool IsWarning() const noexcept
            {
                return type_ == ResultType::Warning;
            }

            /**
             * @brief Boolean conversion operator. Returns true if the Result is successful.
             * @return true if success, false if error
             *
             * @note This allows using Result in boolean contexts like if statements.
             * Warning results are considered successful (return true).
             *
             * @code
             * Result<int> result = SomeOperation();
             * if (result) {
             *     // Success path
             * }
             * @endcode
             */
            explicit operator bool() const noexcept
            {
                return IsSuccess();
            }

            /**
             * @brief Gets the stored value (lvalue reference).
             * @return Reference to the stored value
             *
             * @pre IsSuccess() must be true
             * @note Does not check if the Result is in success state. Caller must verify
             * before calling this function.
             *
             * @code
             * Result<int> result = Result<int>::Success(42);
             * int& value = result.GetValue();  // value is 42
             * @endcode
             */
            T &GetValue() & noexcept
            {
                return value_;
            }

            /**
             * @brief Gets the stored value (rvalue reference, move semantics).
             * @return Rvalue reference to the stored value
             *
             * @pre IsSuccess() must be true
             * @note The value will be moved from the Result, leaving it in a valid but
             * unspecified state.
             *
             * @code
             * Result<std::string> result = Result<std::string>::Success("Hello");
             * std::string value = std::move(result).GetValue();  // Moves the string
             * @endcode
             */
            T &&GetValue() && noexcept
            {
                return std::move(value_);
            }

            /**
             * @brief Gets the stored value (const reference).
             * @return Const reference to the stored value
             *
             * @pre IsSuccess() must be true
             * @note Does not modify the Result. Safe to call on const objects.
             *
             * @code
             * const Result<int> result = Result<int>::Success(42);
             * const int& value = result.GetValue();  // value is 42
             * @endcode
             */
            const T &GetValue() const & noexcept
            {
                return value_;
            }

            /**
             * @brief Gets the error message.
             * @return String view of the error message
             *
             * @note Safe to call regardless of the Result state. Returns empty string
             * for success results.
             *
             * @code
             * Result<int> result = Result<int>::Error("File not found");
             * std::cout << result.GetError() << std::endl;  // "File not found"
             * @endcode
             */
            std::string_view GetError() const noexcept
            {
                return error_message_;
            }

            /**
             * @brief Gets the error code.
             * @return The error code
             *
             * @note Safe to call regardless of the Result state. Returns -1 for success
             * results unless explicitly set otherwise.
             *
             * @code
             * Result<int> result = Result<int>::Error("Error", ENOENT);
             * if (result.GetErrorCode() == ENOENT) {
             *     HandleNotFound();
             * }
             * @endcode
             */
            int32_t GetErrorCode() const noexcept
            {
                return error_code_;
            }

            /**
             * @brief Gets the Result type (Success, Warning, or Error).
             * @return The ResultType enum value
             *
             * @code
             * Result<int> result = SomeOperation();
             * switch (result.GetType()) {
             *     case ResultType::Success:
             *         // Handle success
             *         break;
             *     case ResultType::Warning:
             *         // Handle warning
             *         break;
             *     case ResultType::Error:
             *         // Handle error
             *         break;
             * }
             * @endcode
             */
            ResultType GetType() const noexcept
            {
                return type_;
            }

            /**
             * @brief Throws an exception if the Result is not successful.
             * @param message Custom message to include in the exception
             *
             * @throw std::runtime_error If the Result is not in success state
             *
             * @note This function is useful when you want to convert a Result-based
             * error into an exception. It's often used at API boundaries or when
             * interfacing with code that uses exceptions.
             *
             * @code
             * Result<int> result = SomeOperation();
             * result.Expect("Operation failed");  // Throws if result is error
             * int value = result.GetValue();      // Safe to call after Expect
             * @endcode
             */
            void Expect(std::string_view message) const
            {
                if (!IsSuccess())
                {
                    throw std::runtime_error(std::string(message) + ": " + std::string(GetError()));
                }
            }

            /**
             * @brief Chains a function to be executed if the Result is successful (monadic bind).
             * @param f Function to call with the stored value if successful
             * @return Result of the function call, or an error Result if this Result is not successful
             *
             * @tparam F Type of the function (must be callable with T and return a Result)
             *
             * @note This is the monadic bind operation. If this Result is successful, the function
             * is called with the stored value and its Result is returned. If this Result is an error,
             * the error is propagated without calling the function.
             *
             * @code
             * Result<int> result = ReadFile("config.txt")
             *     .AndThen([](std::string content) { return ParseConfig(content); })
             *     .AndThen([](Config config) { return ValidateConfig(config); });
             * @endcode
             */
            template <typename F>
            auto AndThen(F &&f) -> std::invoke_result_t<F, T>
            {
                using ReturnType = std::invoke_result_t<F, T>;
                if (IsSuccess())
                {
                    return std::forward<F>(f)(std::move(value_));
                }
                return ReturnType::Error(error_message_, error_code_);
            }

            /**
             * @brief Provides error recovery by calling a function if the Result is not successful.
             * @param f Function to call with error information if this Result is not successful
             * @return The Result returned by the function, or this Result if successful
             *
             * @tparam F Type of the function (must be callable with (std::string_view, int32_t) and return Result)
             *
             * @note This is useful for error recovery or fallback logic. If this Result is successful,
             * it's returned unchanged. If it's an error, the recovery function is called.
             *
             * @code
             * Result<Data> result = LoadFromCache()
             *     .OrElse([](auto msg, auto code) { return LoadFromDatabase(); })
             *     .OrElse([](auto msg, auto code) { return LoadDefault(); });
             * @endcode
             */
            template <typename F>
            Result OrElse(F &&f) const
            {
                if (!IsSuccess())
                {
                    return std::forward<F>(f)(error_message_, error_code_);
                }
                return *this;
            }

            /**
             * @brief Transforms the successful value using a mapping function.
             * @param f Function to transform the stored value
             * @return Result containing the transformed value, or an error Result if this Result is not successful
             *
             * @tparam F Type of the transformation function
             *
             * @note Similar to AndThen but for functions that return a plain value, not a Result.
             * If this Result is successful, the function is applied to the value and wrapped in a new Result.
             * If this Result is an error, the error is propagated.
             *
             * @code
             * Result<int> result = Result<int>::Success(21);
             * Result<int> doubled = result.Map([](int x) { return x * 2; });  // Success(42)
             * @endcode
             */
            template <typename F>
            auto Map(F &&f) const -> Result<std::invoke_result_t<F, const T &>>
            {
                using U = std::invoke_result_t<F, const T &>;
                if (IsSuccess())
                {
                    return Result<U>::Success(std::forward<F>(f)(value_));
                }
                return Result<U>::Error(error_message_, error_code_);
            }

            /**
             * @brief Executes a side-effect function if the Result is not successful (non-const version).
             * @param f Function to call with error information for side effects (e.g., logging)
             * @return Reference to this Result (for chaining)
             *
             * @note This is useful for debugging, logging, or other side effects without
             * changing the Result state. The function is called only if the Result is not successful.
             *
             * @code
             * Result<int> result = SomeOperation()
             *     .InspectError([](auto msg, auto code) {
             *         LogError("Operation failed: {} (code: {})", msg, code);
             *     });
             * @endcode
             */
            Result &InspectError(std::function<void(std::string_view, int32_t)> f) &
            {
                if (!IsSuccess())
                {
                    f(error_message_, error_code_);
                }
                return *this;
            }

            /**
             * @brief Executes a side-effect function if the Result is not successful (const version).
             * @param f Function to call with error information for side effects (e.g., logging)
             * @return Const reference to this Result (for chaining)
             *
             * @note Const version of InspectError. Safe to call on const objects.
             */
            const Result &InspectError(std::function<void(std::string_view, int32_t)> f) const &
            {
                if (!IsSuccess())
                {
                    f(error_message_, error_code_);
                }
                return *this;
            }

            /**
             * @brief Gets the stored value or throws an exception if the Result is an error.
             * @return The stored value
             *
             * @throw std::runtime_error If the Result is not in success state
             *
             * @note This is a convenience function for when you want to convert a Result-based
             * error into an exception. It's similar to Expect() but provides a default error message.
             *
             * @code
             * Result<int> result = Result<int>::Success(42);
             * int value = result.Unwrap();  // Returns 42
             *
             * Result<int> error = Result<int>::Error("Something went wrong");
             * int value = error.Unwrap();   // Throws std::runtime_error
             * @endcode
             */
            T Unwrap() const
            {
                if (!IsSuccess())
                {
                    throw std::runtime_error(std::string("Called Unwrap on an error Result: ") + std::string(error_message_));
                }
                return value_;
            }

            /**
             * @brief Gets the stored value or returns a default value if the Result is an error.
             * @param default_value Value to return if this Result is not successful
             * @return The stored value if successful, otherwise the default value
             *
             * @note This function does not throw. It's useful for providing fallback values.
             *
             * @code
             * Result<int> result = Result<int>::Success(42);
             * int value = result.UnwrapOr(0);  // Returns 42
             *
             * Result<int> error = Result<int>::Error("Something went wrong");
             * int value = error.UnwrapOr(0);   // Returns 0 (the default)
             * @endcode
             */
            T UnwrapOr(T &&default_value) const noexcept
            {
                if (IsSuccess())
                {
                    return value_;
                }
                return std::forward<T>(default_value);
            }

        private:
            /**
             * @brief Private constructor for creating success Results.
             * @param value The successful value to store
             *
             * @note This constructor is private and only accessible via the static Success() factory.
             */
            explicit Result(T &&value) noexcept
                : type_(ResultType::Success),
                  value_(std::move(value)),
                  error_message_{},
                  error_code_(0)
            {
            }

            /**
             * @brief Private constructor for creating error/warning Results.
             * @param msg Error/warning message
             * @param code Error/warning code
             * @param type Result type (Error or Warning)
             *
             * @note This constructor is private and only accessible via the static Error() and Warning() factories.
             */
            explicit Result(std::string_view msg, int32_t code, ResultType type)
                : type_(type),
                  value_{},
                  error_message_(msg),
                  error_code_(code)
            {
            }

        private:
            ResultType type_ = ResultType::Error; ///< Current state of the Result
            T value_{};                           ///< Stored value (only valid if type_ is Success)
            std::string error_message_{};         ///< Error/warning message
            int32_t error_code_ = -1;             ///< Error/warning code
        };

        /**
         * @class Result<void>
         * @brief Specialization of Result for void type (operations that succeed without a value).
         *
         * This specialization handles operations that don't produce a value on success,
         * such as writing to a file or sending a network packet. It provides the same
         * functional interface as the primary template but without value-related operations.
         *
         * @section Usage Examples
         *
         * Basic void operation:
         * @code
         * Result<void> SaveData(const std::string& filename) {
         *     if (!OpenFile(filename)) {
         *         return Result<void>::Error("Cannot open file", errno);
         *     }
         *     if (!WriteData()) {
         *         return Result<void>::Error("Write failed", errno);
         *     }
         *     return Result<void>::Success();
         * }
         *
         * auto result = SaveData("data.txt");
         * if (!result) {
         *     std::cerr << "Save failed: " << result.GetError() << std::endl;
         * }
         * @endcode
         *
         * Chaining void operations:
         * @code
         * Result<void> SetupSystem() {
         *     return InitializeLogging()
         *         .AndThen([]() { return LoadConfiguration(); })
         *         .AndThen([]() { return ConnectToDatabase(); })
         *         .AndThen([]() { return StartServices(); });
         * }
         * @endcode
         */
        template <>
        class Result<void> final
        {
        public:
            /**
             * @brief Default constructor. Creates an error Result with "Uninitialized Result" message.
             */
            Result() = default;

            /**
             * @brief Move constructor.
             * @param other The Result to move from
             */
            Result(Result &&) = default;

            /**
             * @brief Copy constructor.
             * @param other The Result to copy from
             */
            Result(const Result &) = default;

            /**
             * @brief Destructor.
             */
            ~Result() = default;

            /**
             * @brief Move assignment operator.
             * @param other The Result to move from
             * @return Reference to this Result
             */
            Result &operator=(Result &&) = default;

            /**
             * @brief Copy assignment operator.
             * @param other The Result to copy from
             * @return Reference to this Result
             */
            Result &operator=(const Result &) = default;

            /**
             * @brief Creates a successful void Result.
             * @return A Result in success state
             *
             * @code
             * Result<void> result = Result<void>::Success();
             * if (result.IsSuccess()) {
             *     std::cout << "Operation completed successfully" << std::endl;
             * }
             * @endcode
             */
            static Result Success() noexcept
            {
                return Result(ResultType::Success);
            }

            /**
             * @brief Creates an error Result with the given message and error code.
             * @param message Error message describing what went wrong
             * @param code Error code (default: -1)
             * @return A Result in error state
             *
             * @code
             * Result<void> result = Result<void>::Error("Network timeout", ETIMEDOUT);
             * @endcode
             */
            static Result Error(std::string_view message, int32_t code = -1) noexcept
            {
                return Result(message, code, ResultType::Error);
            }

            /**
             * @brief Creates a warning Result with the given message and warning code.
             * @param message Warning message
             * @param code Warning code (default: -1)
             * @return A Result in warning state
             *
             * @code
             * Result<void> result = Result<void>::Warning("Operation completed with non-fatal issues", 100);
             * @endcode
             */
            static Result Warning(std::string_view message, int32_t code = -1) noexcept
            {
                return Result(message, code, ResultType::Warning);
            }

            /**
             * @brief Checks if the Result is in success state.
             * @return true if success, false otherwise
             */
            bool IsSuccess() const noexcept
            {
                return type_ == ResultType::Success;
            }

            /**
             * @brief Checks if the Result is in error state.
             * @return true if error, false otherwise
             */
            bool IsError() const noexcept
            {
                return type_ == ResultType::Error;
            }

            /**
             * @brief Checks if the Result is in warning state.
             * @return true if warning, false otherwise
             */
            bool IsWarning() const noexcept
            {
                return type_ == ResultType::Warning;
            }

            /**
             * @brief Boolean conversion operator. Returns true if the Result is successful.
             * @return true if success, false if error
             *
             * @note Warning results are considered successful (return true).
             */
            explicit operator bool() const noexcept
            {
                return IsSuccess();
            }

            /**
             * @brief Gets the error message.
             * @return String view of the error message
             */
            std::string_view GetError() const noexcept
            {
                return error_message_;
            }

            /**
             * @brief Gets the error code.
             * @return The error code
             */
            int32_t GetErrorCode() const noexcept
            {
                return error_code_;
            }

            /**
             * @brief Gets the Result type (Success, Warning, or Error).
             * @return The ResultType enum value
             */
            ResultType GetType() const noexcept
            {
                return type_;
            }

            /**
             * @brief Throws an exception if the Result is not successful.
             * @param message Custom message to include in the exception
             *
             * @throw std::runtime_error If the Result is not in success state
             */
            void Expect(std::string_view message) const
            {
                if (!IsSuccess())
                {
                    throw std::runtime_error(std::string(message) + ": " + std::string(error_message_));
                }
            }

            /**
             * @brief Chains a function to be executed if the Result is successful (monadic bind).
             * @param f Function to call if successful (must take no parameters and return a Result)
             * @return Result of the function call, or an error Result if this Result is not successful
             *
             * @tparam F Type of the function (must be callable with no args and return a Result)
             *
             * @note Unlike the primary template, this version calls the function without arguments
             * since there's no value to pass for void Results.
             *
             * @code
             * Result<void> result = Initialize()
             *     .AndThen([]() { return LoadConfig(); })
             *     .AndThen([]() { return StartServices(); });
             * @endcode
             */
            template <typename F>
            auto AndThen(F &&f) -> std::invoke_result_t<F>
            {
                using ReturnType = std::invoke_result_t<F>;
                if (IsSuccess())
                {
                    return std::forward<F>(f)();
                }
                return ReturnType::Error(error_message_, error_code_);
            }

            /**
             * @brief Provides error recovery by calling a function if the Result is not successful.
             * @param f Function to call with error information if this Result is not successful
             * @return The Result returned by the function, or this Result if successful
             *
             * @tparam F Type of the function (must be callable with (std::string_view, int32_t) and return Result)
             */
            template <typename F>
            Result OrElse(F &&f) const
            {
                if (!IsSuccess())
                {
                    return std::forward<F>(f)(error_message_, error_code_);
                }
                return *this;
            }

            /**
             * @brief Transforms the void success into a value using a mapping function.
             * @param f Function that produces a value if this Result is successful
             * @return Result containing the produced value, or an error Result if this Result is not successful
             *
             * @tparam F Type of the function
             *
             * @note This converts a void Result into a Result containing a value.
             * If this Result is successful, the function is called and its return value is wrapped in a Result.
             * If this Result is an error, the error is propagated.
             *
             * @code
             * Result<void> setup = SetupSystem();
             * Result<Config> config = setup.Map([]() { return LoadConfiguration(); });
             * @endcode
             */
            template <typename F>
            auto Map(F &&f) const -> Result<std::invoke_result_t<F>>
            {
                using U = std::invoke_result_t<F>;
                if (IsSuccess())
                {
                    return Result<U>::Success(std::forward<F>(f)());
                }
                return Result<U>::Error(error_message_, error_code_);
            }

            /**
             * @brief Executes a side-effect function if the Result is not successful (non-const version).
             * @param f Function to call with error information for side effects (e.g., logging)
             * @return Reference to this Result (for chaining)
             */
            Result &InspectError(std::function<void(std::string_view, int32_t)> f) &
            {
                if (!IsSuccess())
                {
                    f(error_message_, error_code_);
                }
                return *this;
            }

            /**
             * @brief Executes a side-effect function if the Result is not successful (const version).
             * @param f Function to call with error information for side effects (e.g., logging)
             * @return Const reference to this Result (for chaining)
             */
            const Result &InspectError(std::function<void(std::string_view, int32_t)> f) const &
            {
                if (!IsSuccess())
                {
                    f(error_message_, error_code_);
                }
                return *this;
            }

            /**
             * @brief Gets the stored value or throws an exception if the Result is an error.
             * @return The stored value
             *
             * @throw std::runtime_error If the Result is not in success state
             *
             * @note This is a convenience function for when you want to convert a Result-based
             * error into an exception. It's similar to Expect() but provides a default error message.
             *
             * @code
             * Result<int> result = Result<int>::Success(42);
             * int value = result.Unwrap();  // Returns 42
             *
             * Result<int> error = Result<int>::Error("Something went wrong");
             * int value = error.Unwrap();   // Throws std::runtime_error
             * @endcode
             */
            T Unwrap() const
            {
                if (!IsSuccess())
                {
                    throw std::runtime_error(std::string("Called Unwrap on an error Result: ") + std::string(error_message_));
                }
                return value_;
            }

            /**
             * @brief Gets the stored value or returns a default value if the Result is an error.
             * @param default_value Value to return if this Result is not successful
             * @return The stored value if successful, otherwise the default value
             *
             * @note This function does not throw. It's useful for providing fallback values.
             *
             * @code
             * Result<int> result = Result<int>::Success(42);
             * int value = result.UnwrapOr(0);  // Returns 42
             *
             * Result<int> error = Result<int>::Error("Something went wrong");
             * int value = error.UnwrapOr(0);   // Returns 0 (the default)
             * @endcode
             */
            T UnwrapOr(T &&default_value) const noexcept
            {
                if (IsSuccess())
                {
                    return value_;
                }
                return std::forward<T>(default_value);
            }

        private:
            /**
             * @brief Private constructor for creating success Results.
             * @param success_type Must be ResultType::Success
             */
            explicit Result(ResultType success_type) noexcept
                : type_(success_type),
                  error_message_(""),
                  error_code_(-1)
            {
            }

            /**
             * @brief Private constructor for creating error/warning Results.
             * @param msg Error/warning message
             * @param code Error/warning code
             * @param type Result type (Error or Warning)
             */
            Result(std::string_view msg, int32_t code, ResultType type) noexcept
                : type_(type),
                  error_message_(msg),
                  error_code_(code)
            {
            }

        private:
            ResultType type_ = ResultType::Error;                ///< Current state of the Result
            std::string error_message_ = "Uninitialized Result"; ///< Error/warning message
            int32_t error_code_ = -1;                            ///< Error/warning code
        };
    } // namespace utils
} // namespace hud_3d
