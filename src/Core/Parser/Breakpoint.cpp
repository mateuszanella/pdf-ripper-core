#include "Core/Parser/Breakpoint.hpp"

namespace Ripper::Core
{
    Breakpoint::Breakpoint(std::size_t pos, BreakpointType type)
        : _position{pos}, _type{type}
    {
    }

    std::size_t Breakpoint::GetPosition() const
    {
        return _position;
    }

    BreakpointType Breakpoint::GetType() const
    {
        return _type;
    }

    bool Breakpoint::Is(BreakpointType type) const
    {
        return _type == type;
    }

    bool Breakpoint::operator!=(const Breakpoint &other) const
    {
        return !(*this == other);
    }

    bool Breakpoint::operator==(const Breakpoint &other) const
    {
        return _position == other._position && _type == other._type;
    }
}
