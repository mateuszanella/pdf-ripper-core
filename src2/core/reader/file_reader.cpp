#include "core/reader/file_reader.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace ripper::core
{
    file_reader::file_reader(const std::filesystem::path path)
        : _path{std::move(path)},
          _canonicalPath{std::filesystem::canonical(_path).string()},
          _handle{_path, std::ios::binary}
    {
        if (!_handle.is_open())
        {
            throw std::runtime_error{"Failed to open PDF file: " + _path.string()};
        }
    }

    bool file_reader::is_open() const noexcept
    {
        return _handle.is_open();
    }

    bool file_reader::eof() const noexcept
    {
        return _handle.eof();
    }

    std::uint64_t file_reader::size() const noexcept
    {
        return std::filesystem::file_size(_path);
    }

    std::string_view file_reader::get_path() const noexcept
    {
        return _canonicalPath;
    }

    std::size_t file_reader::tell() const noexcept
    {
        return _currentOffset;
    }

    std::byte file_reader::peek()
    {
        if (!is_open() || eof())
        {
            return std::byte{0};
        }

        const std::streampos currentPos = _handle.tellg();

        char ch = '\0';

        _handle.get(ch);
        _handle.seekg(currentPos);

        return std::byte{static_cast<unsigned char>(ch)};
    }

    std::size_t file_reader::read(std::span<std::byte> buffer)
    {
        if (!is_open())
        {
            return 0;
        }

        _handle.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(_handle.gcount());

        _currentOffset += bytesRead;

        return bytesRead;
    }

    std::size_t file_reader::read_at(std::span<std::byte> buffer, const std::uint64_t offset)
    {
        if (!is_open())
        {
            return 0;
        }

        _handle.clear();
        _handle.seekg(offset, std::ios::beg);
        _handle.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(_handle.gcount());

        _currentOffset = offset + bytesRead;

        return bytesRead;
    }

    std::size_t file_reader::read_line(std::span<std::byte> buffer)
    {
        if (!is_open())
        {
            return 0;
        }

        if (_handle.fail())
        {
            _handle.clear();
        }

        _handle.getline(reinterpret_cast<char *>(buffer.data()), buffer.size());

        std::size_t bytesRead = static_cast<std::size_t>(_handle.gcount());

        _currentOffset += bytesRead;

        return bytesRead;
    }

    void file_reader::seek(std::uint64_t offset)
    {
        if (!is_open())
        {
            return;
        }

        _handle.clear();
        _handle.seekg(offset, std::ios::beg);

        _currentOffset = offset;
    }

    void file_reader::skip(std::size_t n)
    {
        if (!is_open())
        {
            return;
        }

        _handle.clear();
        _handle.seekg(static_cast<std::streamoff>(n), std::ios::cur);

        _currentOffset += n;
    }
}
