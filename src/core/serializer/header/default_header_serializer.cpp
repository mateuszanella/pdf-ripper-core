#include "core/serializer/header/default_header_serializer.hpp"

#include <string_view>
#include <vector>

#include "core/error.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::core
{
    std::expected<std::vector<std::byte>, error> default_header_serializer::serialize(const header &value) const
    {
        const std::string_view version = value.version();

        if (version.empty())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::invalid_argument)
                                       .with_component(error_component::serializer)
                                       .with_field("header.version")
                                       .with_message("Header version cannot be empty")
                                       .build());
        }

        const std::string header_line = "%PDF-" + std::string{version} + "\n";

        std::vector<std::byte> buffer(header_line.size());
        for (std::size_t i = 0; i < header_line.size(); ++i)
        {
            buffer[i] = static_cast<std::byte>(header_line[i]);
        }

        return buffer;
    }
}
