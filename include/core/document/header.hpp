#pragma once

#include <string>
#include <string_view>

namespace ripper::core
{
    class header
    {
    public:
        explicit header(std::string version)
            : _version{std::move(version)}
        {
        }

        [[nodiscard]] std::string_view version() const
        {
            return _version;
        }
    private:
        std::string _version;
    };
}
