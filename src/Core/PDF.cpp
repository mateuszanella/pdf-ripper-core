#include "Core/PDF.hpp"

#include <filesystem>
#include <memory>

#include "Core/Reader/FileReader.hpp"

namespace Ripper::Core
{
    PDF::PDF(std::unique_ptr<Ripper::Core::Reader> reader)
        : _reader(std::move(reader))
    {
    }

    PDF::PDF(const std::filesystem::path path)
        : PDF(std::make_unique<Ripper::Core::FileReader>(path))
    {
    }

    Ripper::Core::Reader &PDF::GetReader() noexcept
    {
        return *_reader;
    }

    const Ripper::Core::Reader &PDF::GetReader() const noexcept
    {
        return *_reader;
    }
}
