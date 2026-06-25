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
    /// Linearize strategy (full rewrite).
    ///
    /// Squashes incremental update history, resolves all in-use entries from
    /// the file, then serialises every active object, cross-reference table,
    /// and trailer in a single pass.  Produces a clean, linearised PDF.
    lienarize
};

} // namespace ripper::pdf::core
