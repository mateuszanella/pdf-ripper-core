#pragma once

#include <string>
#include <string_view>
#include <utility>

namespace ripper::core
{
    /**
     * @brief Represents the PDF file header version (e.g. "1.7").
     * Immutable value type.
     */
    class header
    {
    public:
        explicit header(std::string version)
            : version_{std::move(version)}
        {
        }

        [[nodiscard]] std::string_view version() const noexcept { return version_; }

    private:
        std::string version_;
    };
}
