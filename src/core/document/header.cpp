#include "core/document/header.hpp"

namespace ripper::core
{
    header::header(std::string version)
        : version_{std::move(version)}
    {
    }

    std::string_view header::version() const noexcept
    {
        return version_;
    }

    void header::set_version(std::string version)
    {
        version_ = std::move(version);
    }
}
