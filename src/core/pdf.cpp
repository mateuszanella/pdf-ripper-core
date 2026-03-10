#include "core/pdf.hpp"

#include <filesystem>
#include <memory>

#include "core/reader/file_reader.hpp"

namespace ripper::core
{
    pdf::pdf(std::unique_ptr<ripper::core::reader> reader)
        : _reader(std::move(reader)),
          _parser(std::make_unique<ripper::core::parser>(*_reader))
    {
    }

    pdf::pdf(const std::filesystem::path path)
        : pdf(std::make_unique<ripper::core::file_reader>(path))
    {
    }

    pdf pdf::open(const std::filesystem::path path)
    {
        pdf result(path);

        return result;
    }

    ripper::core::reader &pdf::reader() noexcept
    {
        return *_reader;
    }

    const ripper::core::reader &pdf::reader() const noexcept
    {
        return *_reader;
    }

    ripper::core::parser &pdf::parser() noexcept
    {
        return *_parser;
    }

    const ripper::core::parser &pdf::parser() const noexcept
    {
        return *_parser;
    }
}
