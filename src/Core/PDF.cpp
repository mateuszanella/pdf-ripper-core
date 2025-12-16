#include "Core/PDF.hpp"

#include <filesystem>
#include <memory>

#include "Core/Reader/FileReader.hpp"

namespace Ripper::Core
{
    PDF::PDF(std::unique_ptr<Ripper::Core::Reader> reader)
        : _reader(std::move(reader)),
          _parser(std::make_unique<Ripper::Core::Parser>(*_reader))
    {
    }

    PDF::PDF(const std::filesystem::path path)
        : PDF(std::make_unique<Ripper::Core::FileReader>(path))
    {
    }

    PDF PDF::Open(const std::filesystem::path path)
    {
        PDF pdf(path);

        return pdf;
    }

    Ripper::Core::Reader &PDF::GetReader() noexcept
    {
        return *_reader;
    }

    const Ripper::Core::Reader &PDF::GetReader() const noexcept
    {
        return *_reader;
    }

    Ripper::Core::Parser &PDF::GetParser() noexcept
    {
        return *_parser;
    }

    const Ripper::Core::Parser &PDF::GetParser() const noexcept
    {
        return *_parser;
    }
}
