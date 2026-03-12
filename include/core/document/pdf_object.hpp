#pragma once

#include <cstdint>

#include "core/document/indirect_reference.hpp"

namespace ripper::core
{
    class document;

    /**
     * @brief Base class for any tangible PDF indirect object.
     *
     * Holds the indirect reference (obj number + generation),
     * the byte offset in the file, and a back-reference to the
     * owning document for lazy resolution of sub-objects.
     */
    class pdf_object
    {
    public:
        pdf_object(const document &doc, indirect_reference ref, std::uint64_t offset) noexcept
            : document_{doc}, reference_{ref}, offset_{offset}
        {
        }

        virtual ~pdf_object() = default;

        [[nodiscard]] const indirect_reference &reference() const noexcept
        {
            return reference_;
        }

        [[nodiscard]] std::uint64_t offset() const noexcept
        {
            return offset_;
        }

    protected:
        [[nodiscard]] const document &owner() const noexcept
        {
            return document_;
        }

    private:
        const document &document_;
        indirect_reference reference_;
        std::uint64_t offset_;
    };
}
