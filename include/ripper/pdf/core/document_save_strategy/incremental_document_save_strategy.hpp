#pragma once

#include "ripper/pdf/core/document_save_strategy/document_save_strategy.hpp"

namespace ripper::pdf::core
{

/// Incremental update save strategy.
///
/// Copies the original file bytes verbatim (reader → writer), then appends
/// new cross-reference sections and trailers for every in-memory revision
/// that has not yet been written to disk.
///
/// This matches the PDF specification's incremental update model (§7.5.6):
/// the original file content is preserved, and a new revision containing
/// only the changed, added, or deleted entries is appended at the end.
///
/// ## Requirements
///
/// - A reader backend must be provided (the original file to copy from).
/// - Only sections whose `startxref_offset()` is `std::nullopt` are written
///   (all other sections are assumed to already exist in the copied bytes).
///
/// ## Caveats
///
/// - The `/Prev` chain in each new trailer is corrected during serialization
///   to point to the preceding section's xref offset in the output.
class incremental_document_save_strategy final : public document_save_strategy
{
public:
    void save(document& doc) override;
};

} // namespace ripper::pdf::core
