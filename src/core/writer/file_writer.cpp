#include "core/writer/file_writer.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace ripper::core
{
    /// Construct a file-backed writer and open the target file.
    ///
    /// The target is opened in binary output mode with truncation.
    /// Throws if the file cannot be opened.
    file_writer::file_writer(std::filesystem::path path)
        : _path{std::move(path)},
          _canonicalPath{std::filesystem::absolute(_path).string()},
          _handle{_path, std::ios::binary | std::ios::out | std::ios::trunc}
    {
        if (!_handle.is_open())
        {
            throw std::runtime_error{"Failed to open output file: " + _path.string()};
        }
    }

    /// Return whether the underlying output stream is open.
    bool file_writer::is_open() const noexcept
    {
        return _handle.is_open();
    }

    /// Return the current logical write offset.
    std::size_t file_writer::tell() const noexcept
    {
        return static_cast<std::size_t>(_currentOffset);
    }

    /// Return the canonical absolute file path.
    std::string_view file_writer::get_path() const noexcept
    {
        return _canonicalPath;
    }

    /// Write bytes from `buffer` to the current stream position.
    ///
    /// Returns 0 if the stream is not open, the input buffer is empty,
    /// or a write failure occurs.
    std::size_t file_writer::write(std::span<const std::byte> buffer)
    {
        if (!is_open() || buffer.empty())
        {
            return 0;
        }

        _handle.write(reinterpret_cast<const char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));

        if (_handle.fail())
        {
            return 0;
        }

        _currentOffset += buffer.size();
        return buffer.size();
    }

    /// Set the write position to `offset` from stream start.
    void file_writer::seek(std::uint64_t offset)
    {
        if (!is_open())
        {
            return;
        }

        _handle.clear();
        _handle.seekp(static_cast<std::streamoff>(offset), std::ios::beg);

        if (_handle.fail())
        {
            return;
        }

        _currentOffset = offset;
    }

    /// Flush buffered bytes to backing storage.
    void file_writer::flush()
    {
        if (!is_open())
        {
            return;
        }

        _handle.flush();
    }

    /// Close the stream and release file resources.
    void file_writer::close()
    {
        if (!is_open())
        {
            return;
        }

        _handle.flush();
        _handle.close();
    }
}
