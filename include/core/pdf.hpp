#pragma once

#include <filesystem>
#include <memory>

#include "core/parser/parser.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class pdf
    {
    public:
        explicit pdf(std::unique_ptr<ripper::core::reader> reader);
        explicit pdf(const std::filesystem::path path);

        static pdf open(const std::filesystem::path path);

        [[nodiscard]] ripper::core::reader &reader() noexcept;
        [[nodiscard]] const ripper::core::reader &reader() const noexcept;

        [[nodiscard]] ripper::core::parser &parser() noexcept;
        [[nodiscard]] const ripper::core::parser &parser() const noexcept;

    private:
        std::unique_ptr<ripper::core::reader> _reader;
        std::unique_ptr<ripper::core::parser> _parser;
    };
}
