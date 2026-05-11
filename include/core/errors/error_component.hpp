#pragma once

#include <cstdint>
#include <string_view>

namespace ripper::pdf::core
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
            return lookup(value).name;
        }

    private:
        Value value = unknown;

        struct entry
        {
            Value component;
            std::string_view name;
        };

        static constexpr entry table[] = {
            {unknown, "unknown"},
            {document, "document"},
            {reader, "reader"},
            {writer, "writer"},
            {lexer, "lexer"},
            {parser, "parser"},
            {serializer, "serializer"},
            {cross_reference, "cross_reference"},
            {trailer, "trailer"},
            {catalog, "catalog"},
            {pages, "pages"},
            {compression, "compression"},
        };

        [[nodiscard]] static constexpr const entry &lookup(Value v) noexcept
        {
            for (const auto &e : table)
                if (e.component == v)
                    return e;
            return table[0];
        }
    };
}
