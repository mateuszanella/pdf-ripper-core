#pragma once

#include "core/document.hpp"
#include "core/error.hpp"
#include "core/serializer/header/header_serializer.hpp"

namespace ripper::core
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

        /// Replace the header parser implementation.
        void set_header_serializer(std::unique_ptr<class header_serializer> value) noexcept;

        /// Access the configured header parser.
        [[nodiscard]] class header_serializer &header_serializer();

    private:
        const document &document_;

        std::unique_ptr<class header_serializer> header_serializer_;
    };
}
