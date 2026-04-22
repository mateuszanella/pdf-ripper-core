#pragma once

#include "core/document.hpp"
#include "core/error.hpp"
#include "core/serializer/serializer_manager.hpp"

namespace ripper::core
{
    /// High-level PDF serializer facade for a single `document`.
    ///
    /// This type orchestrates serialization by delegating to components managed by
    /// `serializer_manager`, and returns failures through `std::expected<..., error>`.
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
        [[nodiscard]] serializer_manager &manager();

        /// Serialize a PDF header to a byte buffer.
        [[nodiscard]] std::expected<std::vector<std::byte>, error> serialize_header(const header &value);

    private:
        const document &document_;

        std::unique_ptr<class serializer_manager> manager_;
    };
}
