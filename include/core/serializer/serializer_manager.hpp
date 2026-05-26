#pragma once

#include <memory>

#include "core/document.hpp"
#include "core/serializer/cross_reference_table/cross_reference_table_serializer.hpp"
#include "core/serializer/header/header_serializer.hpp"
#include "core/serializer/indirect_object/indirect_object_serializer.hpp"
#include "core/serializer/object/object_serializer.hpp"
#include "core/serializer/trailer/trailer_serializer.hpp"

namespace ripper::pdf::core
{
    /// Owns and exposes the serializer subcomponents used to process a `document`.
    ///
    /// This type centralizes serializer dependencies and enables runtime injection
    /// of concrete serializer implementations (useful for composition and testing).
    /// All injected components are owned via `std::unique_ptr`.
    class serializer_manager
    {
    public:
        /// Construct a serializer manager bound to `doc`.
        ///
        /// The serializer manager stores a reference and does not take ownership of the document.
        explicit serializer_manager(const document &doc);

        /// Replace the header serializer implementation.
        void set_header_serializer(std::unique_ptr<class header_serializer> object);

        /// Replace the object value serializer.
        ///
        /// Also propagates to the indirect_object_serializer so both stay in sync.
        void set_object_serializer(std::unique_ptr<class object_serializer> object);

        /// Replace the indirect-object serializer implementation.
        void set_indirect_object_serializer(std::unique_ptr<class indirect_object_serializer> object);

        /// Replace the cross-reference table serializer implementation.
        void set_cross_reference_table_serializer(std::unique_ptr<class cross_reference_table_serializer> object);

        /// Replace the trailer serializer implementation.
        void set_trailer_serializer(std::unique_ptr<class trailer_serializer> object);

        /// Access the configured header serializer.
        [[nodiscard]] class header_serializer &header_serializer();

        /// Access the configured object serializer.
        [[nodiscard]] class object_serializer &object_serializer();

        /// Access the configured indirect-object serializer.
        [[nodiscard]] class indirect_object_serializer &indirect_object_serializer();

        /// Access the configured cross-reference table serializer.
        [[nodiscard]] class cross_reference_table_serializer &cross_reference_table_serializer();

        /// Access the configured trailer serializer.
        [[nodiscard]] class trailer_serializer &trailer_serializer();

    private:
        const document &document_;

        std::unique_ptr<class header_serializer> header_serializer_;
        std::unique_ptr<class object_serializer> object_serializer_;
        std::unique_ptr<class indirect_object_serializer> indirect_object_serializer_;
        std::unique_ptr<class cross_reference_table_serializer> cross_reference_table_serializer_;
        std::unique_ptr<class trailer_serializer> trailer_serializer_;
    };
}
