#pragma once

#include <expected>
#include <filesystem>
#include <memory>

#include "core/errors/parser/parser_error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class parser;
    class catalog;
    class header;
    class cross_reference_table;
    class trailer;

    class document
    {
    public:
        explicit document(std::unique_ptr<ripper::core::reader> reader);
        explicit document(const std::filesystem::path &path);

        static document open(const std::filesystem::path &path);

        [[nodiscard]] ripper::core::reader &reader() const noexcept;
        [[nodiscard]] ripper::core::parser &parser() const noexcept;

        [[nodiscard]] std::expected<const header &, parser_error> header() const;
        [[nodiscard]] std::expected<const cross_reference_table &, parser_error> cross_reference_table() const;
        [[nodiscard]] std::expected<const trailer &, parser_error> trailer() const;
        [[nodiscard]] std::expected<const catalog &, parser_error> catalog() const;

    private:
        std::unique_ptr<ripper::core::reader> reader_;
        std::unique_ptr<ripper::core::parser> parser_;
    };
}
