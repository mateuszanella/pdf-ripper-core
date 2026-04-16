#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "core/errors/error_code.hpp"
#include "core/errors/error_component.hpp"
#include "core/errors/error_context.hpp"

namespace ripper::core
{
    /// Value-semantic error object.
    ///
    /// An `error` encapsulates an `error_code`, a diagnostic message,
    /// and optional structured context.
    /// The code is intended for programmatic branching, while the message
    /// provides human-readable diagnostic context.
    ///
    /// This type has no success state; every instance represents a failure.
    /// It is intended to be used as the error type in `std::expected`.
    class [[nodiscard]] error
    {
    public:
        /// Create an error with code `internal_error` and a default message.
        ///
        /// This represents an unspecified failure.
        error()
            : code_{error_code::internal_error},
              message_{error_code().default_message()} {}

        /// Create an error with the specified `code`.
        ///
        /// The message is initialized with a code-specific default.
        explicit error(error_code code)
            : code_{code}, message_{code.default_message()} {}

        /// Create an error with the specified `code` and `message`.
        ///
        /// If `message` is empty, a code-specific default is used.
        error(error_code code, std::string message)
            : code_{code}, message_{normalize_message(code, std::move(message))} {}

        /// Create an error with the specified `code` and structured context.
        ///
        /// The message is initialized with a code-specific default.
        error(error_code code, error_context context)
            : code_{code},
              message_{code.default_message()},
              context_{std::move(context)} {}

        /// Create an error with `code`, optional custom `message`, and structured context.
        ///
        /// If `message` is empty, a code-specific default is used.
        error(error_code code, std::string message, error_context context)
            : code_{code},
              message_{normalize_message(code, std::move(message))},
              context_{std::move(context)} {}

        /// Return the error code.
        ///
        /// This value should be used for programmatic branching.
        [[nodiscard]] constexpr error_code code() const noexcept { return code_; }

        /// Return the diagnostic message.
        ///
        /// The message is always non-empty and must not be used for control flow.
        [[nodiscard]] const std::string &message() const noexcept { return message_; }

        /// Return a fully formatted message including structured context when available.
        [[nodiscard]] std::string detailed_message() const
        {
            if (context_.empty())
            {
                return message_;
            }

            std::string result = message_;
            result += " [";

            bool first = true;
            const auto append = [&](std::string_view key, std::string value, bool &is_first)
            {
                if (!is_first)
                {
                    result += ", ";
                }

                result += std::string{key};
                result += "=";
                result += std::move(value);
                is_first = false;
            };

            if (context_.component != error_component::unknown)
            {
                append("component", std::string{context_.component.to_string()}, first);
            }

            if (context_.offset.has_value())
            {
                append("offset", std::to_string(*context_.offset), first);
            }

            if (context_.object_number.has_value())
            {
                append("object", std::to_string(*context_.object_number), first);
            }

            if (context_.generation.has_value())
            {
                append("generation", std::to_string(*context_.generation), first);
            }

            if (context_.field.has_value())
            {
                append("field", *context_.field, first);
            }

            if (context_.expected.has_value())
            {
                append("expected", *context_.expected, first);
            }

            if (context_.actual.has_value())
            {
                append("actual", *context_.actual, first);
            }

            result += "]";
            return result;
        }

        /// Return typed context attached to this error.
        [[nodiscard]] const error_context &context() const noexcept { return context_; }

        /// Return `true` if this error has a non-empty structured context payload.
        [[nodiscard]] bool has_context() const noexcept { return !context_.empty(); }

        /// Return `true` if this error originated from `component`.
        [[nodiscard]] bool in_component(error_component component) const noexcept
        {
            return context_.component == component;
        }

        /// Return `true` if this error has the specified `code`.
        ///
        /// Equivalent to `this->code() == code`.
        [[nodiscard]] constexpr bool is(error_code code) const noexcept { return code_ == code; }

    private:
        [[nodiscard]] static std::string normalize_message(error_code code, std::string message)
        {
            if (!message.empty())
            {
                return message;
            }

            return code.default_message();
        }

        error_code code_{error_code::internal_error};
        std::string message_;
        error_context context_{};
    };
}
