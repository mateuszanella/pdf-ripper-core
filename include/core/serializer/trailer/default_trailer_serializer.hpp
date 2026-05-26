#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/document/trailer/trailer.hpp"
#include "core/serializer/object/object_serializer.hpp"
#include "core/serializer/trailer/trailer_serializer.hpp"

namespace ripper::pdf::core
{
    /// Default implementation for serializing a PDF trailer block into raw bytes.
    ///
    /// Produces:
    ///   trailer\n<<dict>>\nstartxref\n<offset>\n%%EOF\n
    ///
    /// Fields that must not appear in a full-save trailer (e.g. `/Prev`) are
    /// stripped from the dictionary copy before serialization.
    class default_trailer_serializer : public trailer_serializer
    {
    public:
        explicit default_trailer_serializer(class object_serializer &object_serializer);
        ~default_trailer_serializer() override = default;

        /// Serialize the trailer block to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize(const trailer &t, std::uint64_t xref_offset) const override;

        /// Rebind the object value serializer.
        void set_object_serializer(class object_serializer &serializer) override;

    private:
        class object_serializer *object_serializer_;
    };
}
