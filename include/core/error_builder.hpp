#pragma once

#include <string>
#include <utility>

#include "core/error.hpp"
#include "core/document/indirect_reference.hpp"

namespace ripper::core
{
    /// Fluent helper for constructing `error` values.
    ///
    /// `error_builder` is useful when error payload assembly is conditional
    /// or incremental. It also exposes static convenience constructors for
    /// common, caller-actionable error cases.
    class error_builder
    {
    public:
        /// Create a builder with default code `internal_error` and empty message.
        constexpr error_builder() noexcept = default;

        /// Create a builder initialized with `code`.
        constexpr explicit error_builder(error_code code) noexcept
            : code_{code} {}

        /// Static factory method to create a builder with default code and message.
        [[nodiscard]] static constexpr error_builder create() noexcept
        {
            return error_builder{};
        }

        /// Set the error code to use when building.
        [[nodiscard]] constexpr error_builder &with_code(error_code code) noexcept
        {
            code_ = code;
            return *this;
        }

        /// Set the diagnostic message to use when building.
        [[nodiscard]] error_builder &with_message(std::string message)
        {
            message_ = std::move(message);

            return *this;
        }

        /// Build an `error` by copying current builder state.
        [[nodiscard]] error build() const
        {
            return error{code_, message_};
        }

        /// Build an `error` object by moving current builder state.
        [[nodiscard]] error build() &&
        {
            return error{code_, std::move(message_)};
        }

        /// One-shot convenience constructor.
        ///
        /// Builds an `error` directly from `code` and optional `message`.
        [[nodiscard]] static error build(error_code code, std::string message = {})
        {
            return error{code, std::move(message)};
        }

        /// Common convenience constructor for missing xref entry.
        ///
        /// Builds an `error` with code `xref_entry_not_found` and a message containing
        /// the object and generation numbers from `ref`.
        [[nodiscard]] static error xref_entry_not_found(indirect_reference ref)
        {
            std::string message = "Indirect object not found: " + std::to_string(ref.object_number()) + " " + std::to_string(ref.generation());

            return error{error_code::xref_entry_not_found, std::move(message)};
        }

        /// Common convenience constructor for a not in use xref entry.
        ///
        /// Builds an `error` with code `xref_entry_not_in_use` and a message containing
        /// the object and generation numbers from `ref`.
        [[nodiscard]] static error xref_entry_not_in_use(indirect_reference ref)
        {
            std::string message = "Indirect object not in use: " + std::to_string(ref.object_number()) + " " + std::to_string(ref.generation());

            return error{error_code::xref_entry_not_in_use, std::move(message)};
        }

    private:
        error_code code_{error_code::internal_error};
        std::string message_;
    };
}
