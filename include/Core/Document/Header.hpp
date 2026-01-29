#pragma once

#include <string>
#include <string_view>

namespace Ripper::Core
{
    class Header
    {
    public:
        explicit Header(std::string version)
            : _version{std::move(version)}
        {
        }

        [[nodiscard]] const std::string_view &Version() const
        {
            return _version;
        }
    private:
        std::string _version;
    };
}
