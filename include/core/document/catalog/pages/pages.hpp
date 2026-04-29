#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include "core/document/object/indirect_object.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class pages : public indirect_object
    {
    public:
        pages(const document &doc, indirect_reference ref) noexcept
            : indirect_object{doc, ref}
        {
        }

        pages(const document &doc, indirect_reference ref, std::optional<std::uint32_t> count) noexcept
            : indirect_object{doc, ref}, count_{count}
        {
        }

        std::expected<std::uint32_t, error> count() const;

    private:
        std::optional<std::uint32_t> count_;
    };
}
