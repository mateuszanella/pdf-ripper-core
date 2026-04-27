#pragma once

#include <string>
#include <string_view>
#include <utility>

namespace ripper::core
{
    /// Represents the PDF file header, which contains the PDF version string.
    class header
    {
    public:
        /// Construct a header with the given version string.
        ///
        /// @param version A string representing the PDF version (e.g. "1.7").
        explicit header(std::string version);

        /// Returns the PDF version string (e.g. "1.7").
        [[nodiscard]] std::string_view version() const noexcept;

        /// Set a new PDF version string.
        ///
        /// @param version A string representing the new PDF version (e.g. "1.7").
        void set_version(std::string version);

    private:
        std::string version_;
    };
}
