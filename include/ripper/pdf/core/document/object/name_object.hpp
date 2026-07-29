#pragma once

#include <string>

namespace ripper::pdf::core
{

/// Represents a PDF name object (e.g. `/Type`, `/Pages`).
///
/// Distinct from `string_object` to preserve the semantic difference between
/// PDF name objects and PDF string objects at the type level.
///
/// Keys are stored without the leading `/` (e.g. `"Type"`, not `"/Type"`).
struct name_object
{
    std::string value;
};

} // namespace ripper::pdf::core
