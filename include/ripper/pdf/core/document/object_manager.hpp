#pragma once

#include "ripper/pdf/core/document/catalog/catalog.hpp"
#include "ripper/pdf/core/document/document_revision.hpp"
#include "ripper/pdf/core/document/header.hpp"

namespace ripper::pdf::core
{
class document;

/// Static utility for parsing and creating PDF document objects.
///
/// This type encapsulates all logic for parsing document objects from file data
/// and creating new objects to be written to file. It centralizes the coordination
/// of low-level operations (parser invocation, xref allocation, object commitment)
/// with domain-level concerns (object type validation, invariant enforcement).
///
/// All methods are static and take a `document&` parameter to access the parser,
/// xref, trailer, and serialization infrastructure.
///
/// ## Responsibilities
///
/// - **Parse operations**: Read an object from file via the parser and resolve it
///   into the xref, with type validation as needed.
///
/// - **Create operations**: Allocate a new indirect object, populate with defaults,
///   commit to xref, and wire cross-references as needed (e.g., set trailer /Root).
///
/// - **Structural initialization**: Build initial document structures (header, xref,
///   trailer) for new documents or parsed documents.
///
/// ## Usage
///
/// ```cpp
/// auto cat = object_manager::create_catalog(doc);
/// ```
class object_manager
{
public:
    object_manager() = delete;

    /// Parse the catalog from the file and resolve it into the xref.
    ///
    /// Looks up the /Root reference in the trailer, resolves the object via the parser,
    /// validates its /Type is /Catalog, and returns a typed view.
    ///
    /// @throws parse_exception if the trailer /Root is missing or the resolved object
    ///         is not a valid catalog dictionary.
    [[nodiscard]] static class catalog parse_catalog(document& doc);

    /// Allocate a new catalog into the xref and set the trailer /Root.
    ///
    /// Reserves an object reference, constructs a catalog dictionary with /Type /Catalog,
    /// commits it to the xref, and wires the trailer /Root to point to it.
    ///
    /// @throws logic_exception if the xref or trailer operations fail.
    [[nodiscard]] static class catalog create_catalog(document& doc);

    /// Parse the document structure (xref + trailer + histories) from file.
    ///
    /// Delegates to the parser to reconstruct the xref and trailer from the input
    /// and returns the assembled structure without caching.
    ///
    /// @throws parse_exception if the parser fails.
    [[nodiscard]] static class document_revision parse_revision(const document& doc);

    /// Generate a new document structure with default values.
    ///
    /// Constructs initial xref and trailer entries suitable for a new document,
    /// including the required xref entry 0 with object number 0 and generation 65535.
    [[nodiscard]] static class document_revision create_revision();

    /// Parse the PDF header from file without caching.
    ///
    /// Delegates to the parser to extract the header marker and version.
    ///
    /// @throws parse_exception if the parser fails or no parser is available.
    [[nodiscard]] static class header parse_header(const document& doc);

    /// Generate a new PDF header with default values.
    ///
    /// Returns a header object set to PDF 1.4 (suitable for most documents).
    [[nodiscard]] static class header create_header();
};

} // namespace ripper::pdf::core
