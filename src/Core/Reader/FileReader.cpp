#include "Core/Reader/FileReader.hpp"

#include <stdexcept>
#include <utility>
#include <filesystem>

namespace Ripper::Core
{
    FileReader::FileReader(const std::filesystem::path &path)
        : m_handle{path, std::ios::binary}
    {
        if (!m_handle.is_open())
        {
            throw std::runtime_error{"Failed to open PDF file: " + path.string()};
        }
    }

    bool FileReader::IsOpen() const noexcept
    {
        return m_handle.is_open();
    }

    std::uint64_t FileReader::Size() const noexcept
    {
        return 0;
    }
}
