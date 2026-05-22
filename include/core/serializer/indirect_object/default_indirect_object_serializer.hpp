#pragma once

#include <cstddef>
#include <vector>

#include "core/document/object/indirect_object.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/serializer/indirect_object/indirect_object_serializer.hpp"
#include "core/serializer/object/object_serializer.hpp"

namespace ripper::pdf::core
{
    /// Default implementation for serializing a PDF `indirect_object` into raw bytes.
    ///
    /// Handles the `N G obj ... endobj` envelope and delegates all object value
    /// serialization to the `object_serializer` reference supplied at construction.
    /// Ownership of the `object_serializer` lies with the caller (typically
    /// `serializer_manager`).
    class default_indirect_object_serializer : public indirect_object_serializer
    {
    public:
        explicit default_indirect_object_serializer(class object_serializer &object_serializer);
        ~default_indirect_object_serializer() override = default;

        /// Serialize `indirect_object` to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize(const indirect_object &obj) const override;

        /// Rebind the object value serializer.
        void set_object_serializer(class object_serializer &serializer) override;

        /// Propagates to the referenced `object_serializer` in addition to the base.
        void set_line_break_character(char c) override;

        /// Propagates to the referenced `object_serializer` in addition to the base.
        void set_object_break_character(char c) override;

    private:
        /// Format `N G obj` for the indirect-object header line.
        [[nodiscard]] std::vector<std::byte> serialize_direct_reference(const indirect_reference &ref) const;

        class object_serializer *object_serializer_;
    };
}
