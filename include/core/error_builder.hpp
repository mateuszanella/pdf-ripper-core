#pragma once

#include <cstddef>
#include <cstdint>
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
        /// Create a builder with default code `internal_error` and default context.
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

        /// Set the error code to use when building from an enum value.
        [[nodiscard]] constexpr error_builder &with_code(error_code::Value code) noexcept
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

        /// Set a full structured context payload to use when building.
        [[nodiscard]] error_builder &with_context(error_context context)
        {
            context_ = std::move(context);

            return *this;
        }

        /// Set the component in the structured context payload.
        [[nodiscard]] error_builder &with_component(error_component component) noexcept
        {
            context_.component = component;

            return *this;
        }

        /// Set the component in the structured context payload from an enum value.
        [[nodiscard]] error_builder &with_component(error_component::Value component) noexcept
        {
            context_.component = component;

            return *this;
        }

        /// Set the byte offset in the structured context payload.
        [[nodiscard]] error_builder &with_offset(std::size_t offset) noexcept
        {
            context_.offset = offset;

            return *this;
        }

        /// Set object number and generation in the structured context payload.
        [[nodiscard]] error_builder &with_object_reference(std::uint32_t object_number, std::uint16_t generation) noexcept
        {
            context_.object_number = object_number;
            context_.generation = generation;

            return *this;
        }

        /// Set object number and generation from an indirect reference.
        [[nodiscard]] error_builder &with_reference(indirect_reference ref) noexcept
        {
            return with_object_reference(ref.object_number(), ref.generation());
        }

        /// Set `field` in the structured context payload.
        [[nodiscard]] error_builder &with_field(std::string field)
        {
            context_.field = std::move(field);

            return *this;
        }

        /// Set `expected` in the structured context payload.
        [[nodiscard]] error_builder &with_expected(std::string expected)
        {
            context_.expected = std::move(expected);

            return *this;
        }

        /// Set `actual` in the structured context payload.
        [[nodiscard]] error_builder &with_actual(std::string actual)
        {
            context_.actual = std::move(actual);

            return *this;
        }

        /// Build an `error` by copying current builder state.
        [[nodiscard]] error build() const
        {
            return error{code_, message_, context_};
        }

        /// Build an `error` object by moving current builder state.
        [[nodiscard]] error build() &&
        {
            return error{code_, std::move(message_), std::move(context_)};
        }

        /// One-shot convenience constructor.
        [[nodiscard]] static error build(error_code code, std::string message = {}, error_context context = {})
        {
            return error{code, std::move(message), std::move(context)};
        }

        /// Convenience constructor for unexpected EOF conditions.
        [[nodiscard]] static error unexpected_eof(std::string message = {}, error_context context = {})
        {
            return error{error_code(error_code::unexpected_eof), std::move(message), std::move(context)};
        }

        /// Convenience constructor for tokenization failures.
        [[nodiscard]] static error tokenization_error(std::string message = {}, error_context context = {})
        {
            return error{error_code(error_code::tokenization_error), std::move(message), std::move(context)};
        }

        /// Common convenience constructor for missing xref entry.
        [[nodiscard]] static error xref_entry_not_found(indirect_reference ref)
        {
            std::string message = "Indirect object not found: " + std::to_string(ref.object_number()) + " " + std::to_string(ref.generation());

            error_context context{};
            context.component = error_component::cross_reference;
            context.object_number = ref.object_number();
            context.generation = ref.generation();

            return error{error_code(error_code::xref_entry_not_found), std::move(message), std::move(context)};
        }

        /// Common convenience constructor for a not in use xref entry.
        [[nodiscard]] static error xref_entry_not_in_use(indirect_reference ref)
        {
            std::string message = "Indirect object not in use: " + std::to_string(ref.object_number()) + " " + std::to_string(ref.generation());

            error_context context{};
            context.component = error_component::cross_reference;
            context.object_number = ref.object_number();
            context.generation = ref.generation();

            return error{error_code(error_code::xref_entry_not_in_use), std::move(message), std::move(context)};
        }

    private:
        error_code code_{error_code::internal_error};
        std::string message_;
        error_context context_{};
    };
}
