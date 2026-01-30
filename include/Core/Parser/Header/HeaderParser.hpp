#pragma once

#include <expected>
#include <string>
#include <optional>
#include <vector>

#include "Core/Document/Header.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class HeaderParser
    {
    public:
        explicit HeaderParser(Reader &reader);

        [[nodiscard]] std::expected<Header, ParserError> Parse();

    private:
        Reader &_reader;
    };
}
