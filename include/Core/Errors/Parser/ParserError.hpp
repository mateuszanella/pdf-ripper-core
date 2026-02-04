#pragma once

namespace Ripper::Core
{
    enum class ParserError
    {
        None = 0,
        UnexpectedEOF,
        CorruptedHeader,
        MissingHeader,
        MissingCrossReferenceTable,
        CorruptedCrossReferenceTable,
        MissingTrailer,
        CorruptedTrailer,
        MissingCatalog,
        CorruptedCatalog,
    };
}
