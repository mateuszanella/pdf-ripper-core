#pragma once

#include <expected>
#include <string>
#include <optional>
#include <vector>

#include "Core/Document/Header.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class Parser
    {
    public:
        explicit Parser(Reader &reader);

        /**
         * @brief Parses the PDF header from the underlying reader.
         *
         * @return The parsed Header on success, or a ParserError on failure.
         */
        [[nodiscard]] std::expected<Header, ParserError> ParseHeader();

        /**
         * @brief Returns the last parsed header, if any.
         */
        [[nodiscard]] const std::optional<Header> &Header() const;

        /**
         * @brief Returns the last computed breakpoints for this parser instance.
         */
        [[nodiscard]] const std::vector<Breakpoint> &Breakpoints() const;

    private:
        Reader &_reader;
        std::vector<Breakpoint> _breakpoints{};

        std::optional<class Header> _header;
    };
}
