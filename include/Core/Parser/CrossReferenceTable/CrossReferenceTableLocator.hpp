#pragma once

#include <expected>
#include <vector>

#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class CrossReferenceTableLocator
    {
    public:
        explicit CrossReferenceTableLocator(Reader &reader);

        /**
         * @brief Finds all xref table offsets in the document by following the chain.
         * Returns offsets in order from newest to oldest.
         */
        [[nodiscard]] std::expected<std::vector<std::size_t>, ParserError> FindAllXrefOffsets();

    private:
        Reader &_reader;

        [[nodiscard]] std::expected<std::size_t, ParserError> FindStartXrefOffset();
        [[nodiscard]] std::expected<std::size_t, ParserError> FindPrevXrefOffset(std::size_t currentXrefOffset);
    };
}
