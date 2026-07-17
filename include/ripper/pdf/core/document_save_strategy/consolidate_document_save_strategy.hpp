#pragma once

#include "ripper/pdf/core/document_save_strategy/document_save_strategy.hpp"

namespace ripper::pdf::core
{

/// Consolidate (full-rewrite) document save strategy.
///
/// This is the default strategy used when no custom strategy has been injected
/// via `document::set_save_strategy()`.  It:
///
///   1. Squashes incremental update history into a single section.
///   2. Resolves all in-use entries that still reside on disk.
///   3. Serialises every active object, the cross-reference table, and the
///      trailer to the document's writer backend.
class consolidate_document_save_strategy final : public document_save_strategy
{
public:
    void save(document& doc) override;
};

} // namespace ripper::pdf::core
