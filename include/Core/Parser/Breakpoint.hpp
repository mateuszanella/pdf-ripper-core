#pragma once

#include <expected>
#include <string>
#include <optional>

namespace Ripper::Core
{
    enum class BreakpointType
    {
        HeaderStart,
        HeaderEnd,
        XrefKeyword,
        XrefStart,
        XrefEnd,
        TrailerKeyword,
        TrailerStart,
        TrailerEnd,
        StartXrefKeyword,
        EOFMarker
    };

    class Breakpoint
    {
    public:
        explicit Breakpoint(std::size_t pos, BreakpointType type);

        /**
         * @brief Returns the byte offset position of this breakpoint in the PDF file.
         */
        [[nodiscard]] std::size_t Position() const;

        /**
         * @brief Returns the type of this breakpoint.
         */
        [[nodiscard]] BreakpointType Type() const;

        /**
         * @brief Checks if this breakpoint is of the given type.
         */
        [[nodiscard]] bool Is(BreakpointType type) const;

        [[nodiscard]] bool operator!=(const Breakpoint &other) const;
        [[nodiscard]] bool operator==(const Breakpoint &other) const;

    private:
        std::size_t _position;
        BreakpointType _type;
    };
}
