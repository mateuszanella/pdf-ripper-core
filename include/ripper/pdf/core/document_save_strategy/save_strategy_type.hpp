#pragma once

namespace ripper::pdf::core
{

/// Identifies the built-in document save strategies supported by the library.
///
/// Pass a value of this type to `document::save(save_strategy_type)` to select
/// a specific strategy, or inject a custom implementation via
/// `document::set_save_strategy()`.
enum class save_strategy_type
{
    /// Consolidate strategy (full rewrite).
    ///
    /// Squashes incremental update history, resolves all in-use entries from
    /// the document, then serialises every active object, cross-reference table,
    /// and trailer in a single pass. Produces a clean PDF.
    /// This is the default save strategy.
    consolidate,

    /// Raw resolve-and-dump strategy.
    ///
    /// Resolves all entries and writes the document to the writer backend.
    raw,

    /// Incremental update strategy.
    ///
    /// Copies the original file bytes (reader → writer), then appends new
    /// cross-reference sections and trailers for each in-memory revision
    /// that has not yet been written to disk. Preserves the existing file
    /// content and appends changes as a new revision.
    ///
    /// In order for changes to be saved, users MUST create a new revision
    /// of the document with a explicit call to `document::new_revision()`.
    ///
    /// Many revisions can be saved at once.
    ///
    /// Requires a reader backend. Throws if none is available.
    incremental
};

} // namespace ripper::pdf::core
