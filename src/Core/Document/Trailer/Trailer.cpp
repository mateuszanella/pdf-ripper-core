#include "Core/Document/Trailer/Trailer.hpp"

namespace Ripper::Core
{
    void Trailer::SetSize(std::uint32_t size)
    {
        _size = size;
    }

    std::optional<std::uint32_t> Trailer::Size() const
    {
        return _size;
    }

    void Trailer::SetPrev(std::size_t prev)
    {
        _prev = prev;
    }

    std::optional<std::size_t> Trailer::Prev() const
    {
        return _prev;
    }

    void Trailer::SetRoot(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _rootObjectNumber = objectNumber;
        _rootGeneration = generation;
    }

    std::optional<std::uint32_t> Trailer::RootObjectNumber() const
    {
        return _rootObjectNumber;
    }

    std::optional<std::uint16_t> Trailer::RootGeneration() const
    {
        return _rootGeneration;
    }

    void Trailer::SetInfo(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _infoObjectNumber = objectNumber;
        _infoGeneration = generation;
    }

    std::optional<std::uint32_t> Trailer::InfoObjectNumber() const
    {
        return _infoObjectNumber;
    }

    std::optional<std::uint16_t> Trailer::InfoGeneration() const
    {
        return _infoGeneration;
    }

    void Trailer::SetEncrypt(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _encryptObjectNumber = objectNumber;
        _encryptGeneration = generation;
    }

    std::optional<std::uint32_t> Trailer::EncryptObjectNumber() const
    {
        return _encryptObjectNumber;
    }

    std::optional<std::uint16_t> Trailer::EncryptGeneration() const
    {
        return _encryptGeneration;
    }

    void Trailer::SetID(const std::pair<std::string, std::optional<std::string>> &ids)
    {
        _id = ids;
    }

    const std::optional<std::pair<std::string, std::optional<std::string>>> &Trailer::ID() const
    {
        return _id;
    }

    void Trailer::Merge(const Trailer &other)
    {
        if (other._size.has_value())
            _size = other._size;
        if (other._prev.has_value())
            _prev = other._prev;
        if (other._rootObjectNumber.has_value())
        {
            _rootObjectNumber = other._rootObjectNumber;
            _rootGeneration = other._rootGeneration;
        }
        if (other._infoObjectNumber.has_value())
        {
            _infoObjectNumber = other._infoObjectNumber;
            _infoGeneration = other._infoGeneration;
        }
        if (other._encryptObjectNumber.has_value())
        {
            _encryptObjectNumber = other._encryptObjectNumber;
            _encryptGeneration = other._encryptGeneration;
        }
        if (other._id.has_value())
            _id = other._id;
    }
}
