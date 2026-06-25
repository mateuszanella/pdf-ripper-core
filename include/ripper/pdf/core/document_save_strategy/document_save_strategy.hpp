#pragma once

namespace ripper::pdf::core
{

class document;

/// Pluggable save strategy for a PDF `document`.
///
/// Implement this interface to customise how a `document` is serialised
/// to its writer backend.  The library ships one built-in
/// implementation (`linearize_document_save_strategy`) which performs a full
/// rewrite of all objects, the cross-reference table, and the trailer.
///
/// # Usage
///
/// ```cpp
/// class my_strategy final : public document_save_strategy
/// {
/// public:
///     void save(document& doc) override
///     {
///         // custom save logic using doc's public API
///     }
/// };
///
/// doc.set_save_strategy(std::make_unique<my_strategy>());
/// doc.save();
/// ```
class document_save_strategy
{
public:
    virtual ~document_save_strategy() = default;

    /// Save `document` to its configured writer backend.
    ///
    /// @throws logic_exception if no writer or serializer is available.
    virtual void save(document& doc) = 0;
};

} // namespace ripper::pdf::core
