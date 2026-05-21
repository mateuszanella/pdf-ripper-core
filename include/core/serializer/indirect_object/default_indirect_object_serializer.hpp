#pragma once

#include <cstddef>
#include <vector>

#include "core/document/object/indirect_object.hpp"
#include "core/serializer/indirect_object/indirect_object_serializer.hpp"

namespace ripper::pdf::core
{
    /// Default implementation for serializing a PDF `indirect_object` into raw bytes.
    class default_indirect_object_serializer : public indirect_object_serializer
    {
    public:
        ~default_indirect_object_serializer() override = default;

        /// Serialize `indirect_object` to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize(const indirect_object &obj) const override;

    private:
        /// Helper functions for serializing individual PDF object types to byte buffers.

        /// Serialize the `null` PDF object.
        ///
        /// Ex: `null` literal.
        [[nodiscard]] inline std::vector<std::byte> serialize_null() const
        {
            return {std::byte{'n'}, std::byte{'u'}, std::byte{'l'}, std::byte{'l'}};
        }

        /// Serialize a boolean PDF object.
        ///
        /// Ex: `true` or `false` literals.
        [[nodiscard]] std::vector<std::byte> serialize_bool(bool value) const;

        /// Serialize an integer PDF object.
        ///
        /// Ex: `42` or `-17`.
        [[nodiscard]] std::vector<std::byte> serialize_integer(std::int64_t value) const;

        /// Serialize a real PDF object.
        ///
        /// Ex: `3.14` or `-0.000001`.
        [[nodiscard]] std::vector<std::byte> serialize_real(double value) const;

        /// Serialize a string PDF object, escaping special characters as needed.
        ///
        /// Ex: `(Hello \(World\))` for the string `Hello (World)`.
        [[nodiscard]] std::vector<std::byte> serialize_string(const std::string &value) const;

        /// Serialize a name PDF object, escaping special characters as needed.
        ///
        /// Ex: `/Name#20and#28parens#29` for the name `Name and(parens)`.
        [[nodiscard]] std::vector<std::byte> serialize_name(const name &value) const;

        /// Serialize a direct reference PDF object.
        ///
        /// Ex: `42 0 obj` for a direct reference with object number 42 and generation 0.
        [[nodiscard]] std::vector<std::byte> serialize_direct_reference(const indirect_reference &value) const;

        /// Serialize an indirect reference PDF object.
        ///
        /// Ex: `42 0 R` for an indirect reference with object number 42 and generation 0.
        [[nodiscard]] std::vector<std::byte> serialize_indirect_reference(const indirect_reference &value) const;

        /// Serialize an array PDF object, recursively serializing contained objects.
        ///
        /// Ex: `[1 2 3]` for an array of three integers.
        [[nodiscard]] std::vector<std::byte> serialize_array(const array &value) const;

        /// Serialize a dictionary PDF object, recursively serializing contained key-value pairs.
        ///
        /// Ex: `<< /Key1 1 /Key2 2 >>` for a dictionary with two entries.
        [[nodiscard]] std::vector<std::byte> serialize_dictionary(const dictionary &value) const;

        /// Serialize a stream PDF object, including its dictionary and data.
        ///
        /// Ex: `<< /Length 4 >> stream data endstream` for a simple stream object.
        ///
        /// @throws not_implemented_exception not yet implemented.
        [[nodiscard]] std::vector<std::byte> serialize_stream_object(const object_stream &stream_obj) const;

        /// Serialize any PDF object by dispatching to the appropriate helper function based on the held variant type.
        [[nodiscard]] std::vector<std::byte> serialize_object_value(const object &obj) const;
    };
}
