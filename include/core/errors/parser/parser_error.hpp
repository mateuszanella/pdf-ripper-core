#pragma once

namespace ripper::core
{
    enum class parser_error
    {
        none = 0,
        unexpected_eof,
        corrupted_header,
        missing_header,
        missing_cross_reference_table,
        corrupted_cross_reference_table,
        missing_trailer,
        corrupted_trailer,
        missing_catalog,
        corrupted_catalog,
    };
}
