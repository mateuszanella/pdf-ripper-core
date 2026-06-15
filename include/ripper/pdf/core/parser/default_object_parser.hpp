#pragma once

#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/object_parser.hpp"

#include <string_view>

namespace ripper::pdf::core
{
/// Default implementation of `object_parser`.
///
/// Parses a full PDF indirect object from raw bytes:
///   - Skips the `N G obj` header (identity is taken from `ref`).
///   - Parses the object dictionary.
///   - Detects and captures an optional content stream.
class default_object_parser : public object_parser
{
public:
    default_object_parser() = default;

    [[nodiscard]] indirect_object parse(document& doc, indirect_reference ref,
                                        std::string_view content,
                                        bool preload_stream = true) const override;
};
} // namespace ripper::pdf::core
