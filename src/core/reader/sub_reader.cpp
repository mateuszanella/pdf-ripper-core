#include "core/reader/sub_reader.hpp"

namespace ripper::core
{
    sub_reader::sub_reader(reader &parent, std::size_t startOffset)
        : _parent{parent}, _startOffset{startOffset}
    {
    }

    bool sub_reader::is_open() const noexcept
    {
        return _parent.is_open();
    }

    bool sub_reader::eof() const noexcept
    {
        return _startOffset + _currentOffset >= _parent.size();
    }

    std::uint64_t sub_reader::size() const noexcept
    {
        const std::uint64_t parentSize = _parent.size();
        return parentSize > _startOffset ? parentSize - _startOffset : 0;
    }

    std::size_t sub_reader::tell() const noexcept
    {
        return _currentOffset;
    }

    std::byte sub_reader::peek()
    {
        const std::size_t savedPos = _parent.tell();
        _parent.seek(_startOffset + _currentOffset);
        const std::byte result = _parent.peek();
        _parent.seek(savedPos);
        return result;
    }

    std::size_t sub_reader::read(std::span<std::byte> buffer)
    {
        _parent.seek(_startOffset + _currentOffset);
        const std::size_t bytesRead = _parent.read(buffer);
        _currentOffset += bytesRead;
        return bytesRead;
    }

    std::size_t sub_reader::read_at(std::span<std::byte> buffer, std::uint64_t offset)
    {
        _parent.seek(_startOffset + offset);
        const std::size_t bytesRead = _parent.read(buffer);
        _currentOffset = offset + bytesRead;
        return bytesRead;
    }

    std::size_t sub_reader::read_line(std::span<std::byte> buffer)
    {
        _parent.seek(_startOffset + _currentOffset);
        const std::size_t bytesRead = _parent.read_line(buffer);
        _currentOffset += bytesRead;
        return bytesRead;
    }

    void sub_reader::seek(std::uint64_t offset)
    {
        _currentOffset = offset;
    }

    void sub_reader::skip(std::size_t n)
    {
        _currentOffset += n;
    }
}
