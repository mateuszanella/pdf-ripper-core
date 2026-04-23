#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <functional>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
#include "core/parser/parser.hpp"
#include "core/reader/reader.hpp"
#include "core/serializer/serializer.hpp"
#include "core/writer/writer.hpp"

namespace ripper::core
{
    class parser;
    class catalog;
    class header;
    class cross_reference_table;
    class trailer;

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
        explicit document(std::unique_ptr<ripper::core::reader> reader, std::unique_ptr<ripper::core::writer> writer);

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

        /// Returns whether a reader backend is available.
        [[nodiscard]] bool has_reader() const noexcept;

        /// Returns whether a parser facade is available.
        [[nodiscard]] bool has_parser() const noexcept;

        /// Returns whether a writer backend is available.
        [[nodiscard]] bool has_writer() const noexcept;

        /// Returns whether a serializer facade is available.
        [[nodiscard]] bool has_serializer() const noexcept;

        /// Access the underlying reader backend.
        [[nodiscard]] std::expected<std::reference_wrapper<class reader>, error> reader() const noexcept;

        /// Access the parser facade.
        ///
        /// Returns `unexpected(error)` when no parser is configured.
        /// Individual parse operations also validate backend availability.
        [[nodiscard]] std::expected<std::reference_wrapper<class parser>, error> parser() const noexcept;

        /// Access the underlying writer backend.
        [[nodiscard]] std::expected<std::reference_wrapper<class writer>, error> writer() const noexcept;

        /// Access the serializer facade.
        [[nodiscard]] std::expected<std::reference_wrapper<class serializer>, error> serializer() const noexcept;

        /// Parse and return the PDF header (cached after first successful parse).
        [[nodiscard]] std::expected<header, error> header() const noexcept;

        /// Parse and return the compiled cross-reference table (cached).
        [[nodiscard]] std::expected<cross_reference_table, error> cross_reference_table() const noexcept;

        /// Parse and return the compiled trailer dictionary (cached).
        [[nodiscard]] std::expected<trailer, error> trailer() const noexcept;

        /// Parse and return the document catalog (cached).
        [[nodiscard]] std::expected<catalog, error> catalog() const noexcept;

    private:
        std::unique_ptr<class reader> reader_;
        std::unique_ptr<class parser> parser_;

        std::unique_ptr<class writer> writer_;
        std::unique_ptr<class serializer> serializer_;

        mutable std::optional<class header> header_;

        mutable std::optional<class cross_reference_table> xref_table_;
        mutable std::optional<std::vector<class cross_reference_table>> xref_history_;

        mutable std::optional<class trailer> trailer_;
        mutable std::optional<std::vector<class trailer>> trailer_history_;

        mutable std::optional<class catalog> catalog_;
    };
}
