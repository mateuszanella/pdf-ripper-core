#pragma once

#include <cstdint>

#include "core/document/indirect_reference.hpp"

namespace ripper::core
{
    class document;

    /**
     * @brief Base class for any tangible PDF indirect object.
     */
    class pdf_object
    {
    public:
        pdf_object(const document &doc, indirect_reference ref) noexcept
            : document_{doc}, reference_{ref}
        {
        }

        virtual ~pdf_object() = default;

        [[nodiscard]] const indirect_reference &reference() const noexcept
        {
            return reference_;
        }

    protected:
        [[nodiscard]] const document &owner() const noexcept
        {
            return document_;
        }

    private:
        const document &document_;
        indirect_reference reference_;
    };
}
