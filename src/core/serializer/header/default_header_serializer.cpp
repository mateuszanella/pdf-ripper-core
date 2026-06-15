#include "ripper/pdf/core/serializer/header/default_header_serializer.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
std::vector<std::byte> default_header_serializer::serialize(const header& object) const
{
    const std::string_view version = object.version();

    if (version.empty())
    {
        throw logic_exception{"Header version cannot be empty"};
    }

    const std::string header_line = "%PDF-" + std::string{version} + line_break_character_;

    std::vector<std::byte> buffer(header_line.size());
    for (std::size_t i = 0; i < header_line.size(); ++i)
    {
        buffer[i] = static_cast<std::byte>(header_line[i]);
    }

    return buffer;
}
} // namespace ripper::pdf::core
