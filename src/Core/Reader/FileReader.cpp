#include "Core/Reader/FileReader.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
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

    std::size_t FileReader::Read(std::span<std::byte> buffer)
    {
        if (!IsOpen())
        {
            return 0;
        }

        m_handle.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(m_handle.gcount());

        m_currentOffset += bytesRead;

        return bytesRead;
    }
}
