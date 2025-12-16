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

    bool FileReader::Eof() const noexcept
    {
        return m_handle.eof();
    }

    std::uint64_t FileReader::Size() const noexcept
    {
        return std::filesystem::file_size(m_path);
    }

    std::string_view FileReader::GetPath() const noexcept
    {
        return m_canonicalPath;
    }

    std::size_t FileReader::Tell() const noexcept
    {
        return m_currentOffset;
    }

    std::byte FileReader::Peek()
    {
        if (!IsOpen() || Eof())
        {
            return std::byte{0};
        }

        const std::streampos currentPos = m_handle.tellg();

        char ch = '\0';

        m_handle.get(ch);
        m_handle.seekg(currentPos);

        return std::byte{static_cast<unsigned char>(ch)};
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

    std::size_t FileReader::ReadAt(std::span<std::byte> buffer, const std::uint64_t offset)
    {
        if (!IsOpen())
        {
            return 0;
        }

        m_handle.seekg(offset, std::ios::beg);
        m_handle.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(m_handle.gcount());

        m_currentOffset = offset + bytesRead;

        return bytesRead;
    }

    std::size_t FileReader::ReadLine(std::span<std::byte> buffer)
    {
        if (!IsOpen())
        {
            return 0;
        }

        m_handle.getline(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(m_handle.gcount());

        m_currentOffset += bytesRead;

        return bytesRead;
    }

    void FileReader::Seek(std::uint64_t offset)
    {
        if (!IsOpen())
        {
            return;
        }

        m_handle.seekg(offset, std::ios::beg);

        m_currentOffset = offset;
    }

    void FileReader::Skip(std::size_t n)
    {
        if (!IsOpen())
        {
            return;
        }

        m_handle.seekg(static_cast<std::streamoff>(n), std::ios::cur);

        m_currentOffset += n;
    }
}
