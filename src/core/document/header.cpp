#include "ripper/pdf/core/document/header.hpp"

namespace ripper::pdf::core
{
header::header(std::string version) : version_{std::move(version)} {}

std::string_view header::version() const noexcept
{
    return version_;
}

void header::set_version(std::string version)
{
    version_ = std::move(version);
}
} // namespace ripper::pdf::core
