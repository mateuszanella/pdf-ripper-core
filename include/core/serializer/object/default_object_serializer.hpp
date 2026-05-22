#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "core/document/object/object.hpp"
#include "core/serializer/object/object_serializer.hpp"

namespace ripper::pdf::core
{
    class indirect_reference;

    /// Default implementation for serializing a single PDF object value into raw bytes.
    class default_object_serializer : public object_serializer
    {
    public:
        ~default_object_serializer() override = default;

        /// Serialize a PDF object value to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize(const object &obj) const override;

    private:
        [[nodiscard]] inline std::vector<std::byte> serialize_null() const
        {
            return {std::byte{'n'}, std::byte{'u'}, std::byte{'l'}, std::byte{'l'}};
        }

        [[nodiscard]] std::vector<std::byte> serialize_bool(bool value) const;
        [[nodiscard]] std::vector<std::byte> serialize_integer(std::int64_t value) const;
        [[nodiscard]] std::vector<std::byte> serialize_real(double value) const;
        [[nodiscard]] std::vector<std::byte> serialize_string(const std::string &value) const;
        [[nodiscard]] std::vector<std::byte> serialize_name(const name &value) const;
        [[nodiscard]] std::vector<std::byte> serialize_indirect_reference(const indirect_reference &value) const;
        [[nodiscard]] std::vector<std::byte> serialize_array(const array &value) const;
        [[nodiscard]] std::vector<std::byte> serialize_dictionary(const dictionary &value) const;
        [[nodiscard]] std::vector<std::byte> serialize_stream_object(const object_stream &stream_obj) const;
    };
}
