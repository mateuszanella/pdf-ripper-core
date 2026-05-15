#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <functional>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/document_structure.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/parser.hpp"
#include "core/reader/reader.hpp"
#include "core/serializer/serializer.hpp"
#include "core/writer/writer.hpp"

namespace ripper::pdf::core
{
    /// High-level PDF document facade and primary library entrypoint.
    ///
    /// This type represents a PDF document and centralizes all interactions:
    /// opening/creating files, parsing document structures, and exposing
    /// materialized document components through a lazy API.
    class document
    {
    public:
        /// Construct a document from optional reader/writer backends.
        ///
        /// The document takes ownership of the provided backends.
        ///
        /// When a reader is present, accessing document objects (xref, catalog, pages, etc.)
        /// will lazily parse the metadata of the document structure reading bytes from `reader`.
        /// These components are cached after first access, so subsequent accesses do not require
        /// additional parsing.
        ///
        /// When a writer is present, the document can be saved through the `save` method and
        /// written to the backing storage through `writer`. All changes made in memory to objects
        /// returned by the document API are reflected in the output of `save`.
        ///
        /// If a document contains a `writer`, but has no `reader`, all objects are created from
        /// scratch and are not based on any existing file content. This can be used to create new
        /// PDF documents.
        ///
        /// If a document contains a `reader`, but has no `writer`, all objects are read from the
        /// input file and cannot be modified or saved. This can be used to parse and inspect
        /// existing PDF documents.
        explicit document(std::unique_ptr<class reader> reader, std::unique_ptr<class writer> writer);

        /// Open an existing document at `path` in read mode.
        ///
        /// Returns a `document` configured with a file reader.
        ///
        /// @throws std::runtime_error if the file cannot be opened for reading.
        static document open(const std::filesystem::path &path);

        /// Create a document at `path` in write mode.
        ///
        /// Returns a `document` configured with a file writer.
        ///
        /// @throws std::runtime_error if the file cannot be opened for writing.
        static document create(const std::filesystem::path &path);

        /// Save the document to the configured writer backend.
        ///
        /// Returns `true` on success, or an `false` on failure.
        [[nodiscard]] bool save();

        /// Returns whether a reader backend is available.
        [[nodiscard]] bool has_reader() const;

        /// Returns whether a parser facade is available.
        [[nodiscard]] bool has_parser() const;

        /// Returns whether a writer backend is available.
        [[nodiscard]] bool has_writer() const;

        /// Returns whether a serializer facade is available.
        [[nodiscard]] bool has_serializer() const;

        /// Access the underlying reader backend.
        ///
        /// Returns `nullptr` when no reader is configured.
        [[nodiscard]] class reader *reader() const;

        /// Access the parser facade.
        ///
        /// Returns `nullptr` when no parser is configured.
        [[nodiscard]] class parser *parser() const;

        /// Access the underlying writer backend.
        ///
        /// Returns `nullptr` when no writer is configured.
        [[nodiscard]] class writer *writer() const;

        /// Access the serializer facade.
        ///
        /// Returns `nullptr` when no serializer is configured.
        [[nodiscard]] class serializer *serializer() const;

        /// Return the PDF header object.
        ///
        /// If `reader` is set, the header is parsed from the input upon first access and cached
        /// for subsequent accesses. Otherwise, a new header with default values is created and cached.
        /// Changes made to the returned header are reflected in the document on save.
        [[nodiscard]] class header &header();

        /// Parse and return the compiled cross-reference table (cached).
        /// Changes made to the returned table are reflected in the document on save.
        [[nodiscard]] class cross_reference_table &cross_reference_table();

        /// Parse and return the compiled trailer dictionary (cached).
        /// Changes made to the returned trailer are reflected in the document on save.
        [[nodiscard]] class trailer &trailer();

        /// Return a view over the document catalog.
        ///
        /// Resolves the catalog from the xref on first access (lazy). The returned `catalog`
        /// is a lightweight non-owning view. Ownership remains with the xref entry.
        [[nodiscard]] class catalog catalog();

        /// Resolve any indirect object by reference, lazy-loading from file if needed.
        ///
        /// Returns a raw non-owning pointer into the xref entry. The pointer remains valid
        /// as long as the document (and its xref) is alive.
        [[nodiscard]] class indirect_object *resolve_object(indirect_reference ref);

    private:
        /// Parse and return the assembled document structure (xref + trailer + histories) (cached).
        [[nodiscard]] class document_structure &structure();

        /// Parse and return the PDF header without caching.
        [[nodiscard]] class header parse_header() const;

        /// Generate a new PDF header with default values.
        [[nodiscard]] class header create_header() const;

        /// Parse and return the document structure without caching.
        [[nodiscard]] class document_structure parse_structure() const;

        /// Generate a new document structure with default values.
        [[nodiscard]] class document_structure create_structure() const;

        /// Parse the catalog from the file and commit it into the xref.
        [[nodiscard]] class catalog parse_catalog();

        /// Allocate a new catalog into the xref and set the trailer /Root.
        [[nodiscard]] class catalog create_catalog();

        std::unique_ptr<class reader> reader_;
        std::unique_ptr<class parser> parser_;

        std::unique_ptr<class writer> writer_;
        std::unique_ptr<class serializer> serializer_;

        std::optional<class header> header_;

        std::optional<class document_structure> structure_;
    };
}
