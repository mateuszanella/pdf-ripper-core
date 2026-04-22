#pragma once

#include <cstdint>
#include <string_view>

namespace ripper::core
{
    /// Type-safe wrapper around error component enum with member methods.
    ///
    /// Wraps an underlying enum to provide encapsulated string conversion
    /// while supporting direct enum semantics and switch statements.
    class error_component
    {
    public:
        /// Underlying enum values representing error origin components.
        enum Value : std::uint8_t
        {
            unknown = 0,
            document,
            reader,
            writer,
            lexer,
            parser,
            serializer,
            cross_reference,
            trailer,
            catalog,
            pages,
            compression,
        };

        /// Default constructor initializes to `unknown`.
        constexpr error_component() = default;

        /// Construct from an underlying enum value.
        explicit constexpr error_component(Value component) noexcept : value(component) {}

        /// Assign from an underlying enum value.
        constexpr error_component &operator=(Value component) noexcept
        {
            value = component;
            return *this;
        }

        /// Allow use in switch statements and direct enum comparison.
        constexpr operator Value() const noexcept { return value; }

        /// Prevent implicit boolean conversion.
        explicit operator bool() const = delete;

        /// Allow equality comparison with other error_component instances.
        constexpr bool operator==(error_component other) const noexcept { return value == other.value; }

        /// Allow inequality comparison with other error_component instances.
        constexpr bool operator!=(error_component other) const noexcept { return value != other.value; }

        // Allow direct comparison with underlying enum values for convenience.
        constexpr bool operator==(Value other) const noexcept { return value == other; }

        // Allow direct inequality comparison with underlying enum values for convenience.
        constexpr bool operator!=(Value other) const noexcept { return value != other; }

        /// Return a string representation of this component.
        [[nodiscard]] constexpr std::string_view to_string() const noexcept
        {
            static constexpr std::string_view table[] = {
                "unknown",
                "reader",
                "lexer",
                "parser",
                "cross_reference",
                "trailer",
                "catalog",
                "pages",
                "compression",
                "document",
                "serializer",
                "writer",
            };
            if (value < std::size(table))
                return table[value];
            return "unknown";
        }

    private:
        Value value = unknown;
    };
}
