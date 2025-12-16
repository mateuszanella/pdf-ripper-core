#pragma once

#include <expected>
#include <string>

#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class Parser
    {
    public:
        explicit Parser(Reader& reader);

        [[nodiscard]] std::expected<std::string, ParserError> ReadHeader();

    private:
        Reader& m_reader;
    };
}
