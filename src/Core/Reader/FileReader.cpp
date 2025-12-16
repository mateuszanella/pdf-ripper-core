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
        : _path{std::move(path)},
          _canonicalPath{std::filesystem::canonical(_path).string()},
          _handle{_path, std::ios::binary}
    {
        if (!_handle.is_open())
        {
            throw std::runtime_error{"Failed to open PDF file: " + path.string()};
        }
    }

    bool FileReader::IsOpen() const noexcept
    {
        return _handle.is_open();
    }

    bool FileReader::Eof() const noexcept
    {
        return _handle.eof();
    }

    std::uint64_t FileReader::Size() const noexcept
    {
        return std::filesystem::file_size(_path);
    }

    std::string_view FileReader::GetPath() const noexcept
    {
        return _canonicalPath;
    }

    std::size_t FileReader::Tell() const noexcept
    {
        return _currentOffset;
    }

    std::byte FileReader::Peek()
    {
        if (!IsOpen() || Eof())
        {
            return std::byte{0};
        }

        const std::streampos currentPos = _handle.tellg();

        char ch = '\0';

        _handle.get(ch);
        _handle.seekg(currentPos);

        return std::byte{static_cast<unsigned char>(ch)};
    }

    std::size_t FileReader::Read(std::span<std::byte> buffer)
    {
        if (!IsOpen())
        {
            return 0;
        }

        _handle.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(_handle.gcount());

        _currentOffset += bytesRead;

        return bytesRead;
    }

    std::size_t FileReader::ReadAt(std::span<std::byte> buffer, const std::uint64_t offset)
    {
        if (!IsOpen())
        {
            return 0;
        }

        _handle.seekg(offset, std::ios::beg);
        _handle.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(_handle.gcount());

        _currentOffset = offset + bytesRead;

        return bytesRead;
    }

    std::size_t FileReader::ReadLine(std::span<std::byte> buffer)
    {
        if (!IsOpen())
        {
            return 0;
        }

        _handle.getline(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(_handle.gcount());

        _currentOffset += bytesRead;

        return bytesRead;
    }

    void FileReader::Seek(std::uint64_t offset)
    {
        if (!IsOpen())
        {
            return;
        }

        _handle.seekg(offset, std::ios::beg);

        _currentOffset = offset;
    }

    void FileReader::Skip(std::size_t n)
    {
        if (!IsOpen())
        {
            return;
        }

        _handle.seekg(static_cast<std::streamoff>(n), std::ios::cur);

        _currentOffset += n;
    }
}
