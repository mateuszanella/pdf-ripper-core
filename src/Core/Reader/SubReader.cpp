#include "Core/Reader/SubReader.hpp"

namespace Ripper::Core
{
    SubReader::SubReader(Reader &parent, std::size_t startOffset)
        : _parent{parent}, _startOffset{startOffset}
    {
    }

    bool SubReader::IsOpen() const noexcept
    {
        return _parent.IsOpen();
    }

    bool SubReader::Eof() const noexcept
    {
        return _startOffset + _currentOffset >= _parent.Size();
    }

    std::uint64_t SubReader::Size() const noexcept
    {
        const std::uint64_t parentSize = _parent.Size();
        return parentSize > _startOffset ? parentSize - _startOffset : 0;
    }

    std::size_t SubReader::Tell() const noexcept
    {
        return _currentOffset;
    }

    std::byte SubReader::Peek()
    {
        const std::size_t savedPos = _parent.Tell();
        _parent.Seek(_startOffset + _currentOffset);
        const std::byte result = _parent.Peek();
        _parent.Seek(savedPos);
        return result;
    }

    std::size_t SubReader::Read(std::span<std::byte> buffer)
    {
        _parent.Seek(_startOffset + _currentOffset);
        const std::size_t bytesRead = _parent.Read(buffer);
        _currentOffset += bytesRead;
        return bytesRead;
    }

    std::size_t SubReader::ReadAt(std::span<std::byte> buffer, std::uint64_t offset)
    {
        _parent.Seek(_startOffset + offset);
        const std::size_t bytesRead = _parent.Read(buffer);
        _currentOffset = offset + bytesRead;
        return bytesRead;
    }

    std::size_t SubReader::ReadLine(std::span<std::byte> buffer)
    {
        _parent.Seek(_startOffset + _currentOffset);
        const std::size_t bytesRead = _parent.ReadLine(buffer);
        _currentOffset += bytesRead;
        return bytesRead;
    }

    void SubReader::Seek(std::uint64_t offset)
    {
        _currentOffset = offset;
    }

    void SubReader::Skip(std::size_t n)
    {
        _currentOffset += n;
    }
}
