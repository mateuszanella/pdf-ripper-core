#pragma once

#include "core/document.hpp"
#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/exceptions/exception.hpp"
#include "core/serializer/serializer_manager.hpp"

namespace ripper::pdf::core
{
    /// High-level PDF serializer facade for a single `document`.
    ///
    /// This type orchestrates serialization by delegating to components managed by
    /// `serializer_manager`, and throws on failures.
    class serializer
    {
    public:
        /// Construct a serializer bound to `doc`.
        ///
        /// The serializer stores a reference and does not take ownership of the document.
        explicit serializer(const document &doc);

        /// Return the serializer manager used by this serializer.
        ///
        /// Can be used to replace serializer subcomponents.
        [[nodiscard]] class serializer_manager &manager();

        /// Serialize a PDF header to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize_header(const header &header);

        /// Serialize a PDF indirect object to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize_indirect_object(const indirect_object &obj);

        /// Serialize a PDF cross-reference table to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize_cross_reference_table(const cross_reference_manager &xref);

    private:
        const document &document_;

        std::unique_ptr<class serializer_manager> manager_;
    };
}
