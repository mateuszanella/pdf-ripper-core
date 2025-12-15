#include "Core/Reader/FileReader.hpp"

#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace Ripper::Core
{
    FileReader::FileReader(const std::filesystem::path path)
        : m_path{std::move(path)},
          m_canonicalPath{std::filesystem::canonical(m_path).string()},
          m_handle{m_path, std::ios::binary}
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
        return std::filesystem::file_size(m_path);
    }

    std::string_view FileReader::GetPath() const noexcept
    {
        return m_canonicalPath;
    }
}
