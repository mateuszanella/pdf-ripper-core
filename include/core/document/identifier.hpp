#pragma once

#include <optional>
#include <string>

namespace ripper::core
{
    /**
     * @brief Represents a PDF document identifier.
     */
    class identifier
    {
        public:
            explicit identifier(std::string original) noexcept : original_{std::move(original)}, current_{std::nullopt} {}
            explicit identifier(std::string original, std::string current) noexcept : original_{std::move(original)}, current_{std::move(current)} {}

            [[nodiscard]] const std::string &original() const noexcept { return original_; }
            [[nodiscard]] const std::optional<std::string> &current() const noexcept { return current_; }
        private:
            std::string original_;
            std::optional<std::string> current_;
    };
}
