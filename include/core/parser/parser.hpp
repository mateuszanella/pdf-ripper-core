#pragma once

#include <memory>
#include <vector>

#include "core/document/header.hpp"
#include "core/document/document_structure.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    class document;
    class parser_manager;
    class indirect_reference;

    /// High-level PDF parser facade for a single `document`.
    ///
    /// This type orchestrates parsing by delegating to components managed by
    /// `parser_manager`, and throws on failures.
    class parser
    {
    public:
        /// Construct a parser bound to `doc`.
        ///
        /// The parser stores a reference and does not take ownership of the document.
        /// Backend availability is validated by parse operations.
        explicit parser(document &doc);

        /// Destroy the parser and its internal manager.
        ~parser();

        /// Return the parser manager used by this parser.
        ///
        /// Can be used to replace parser subcomponents.
        [[nodiscard]] parser_manager &manager();

        /// Parse and return the document header.
        [[nodiscard]] header header();

        /// Parse and return compiled document structure and traversal history.
        [[nodiscard]] document_structure structure();

        /// Parse any indirect object by reference and return the fully resolved `indirect_object`.
        ///
        /// The result can be cast to a typed subclass (catalog, pages, etc.) when the
        /// caller knows the `/Type` of the object.
        [[nodiscard]] indirect_object parse_object(indirect_reference ref);

    private:
        document &document_;
        std::unique_ptr<parser_manager> manager_;
    };
}
