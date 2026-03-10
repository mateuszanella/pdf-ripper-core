#include "core/document/trailer/trailer.hpp"

namespace ripper::core
{
    void trailer::set_size(std::uint32_t size)
    {
        _size = size;
    }

    std::optional<std::uint32_t> trailer::size() const
    {
        return _size;
    }

    void trailer::set_prev(std::size_t prev)
    {
        _prev = prev;
    }

    std::optional<std::size_t> trailer::prev() const
    {
        return _prev;
    }

    void trailer::set_root(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _rootObjectNumber = objectNumber;
        _rootGeneration = generation;
    }

    std::optional<std::uint32_t> trailer::root_object_number() const
    {
        return _rootObjectNumber;
    }

    std::optional<std::uint16_t> trailer::root_generation() const
    {
        return _rootGeneration;
    }

    void trailer::set_info(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _infoObjectNumber = objectNumber;
        _infoGeneration = generation;
    }

    std::optional<std::uint32_t> trailer::info_object_number() const
    {
        return _infoObjectNumber;
    }

    std::optional<std::uint16_t> trailer::info_generation() const
    {
        return _infoGeneration;
    }

    void trailer::set_encrypt(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _encryptObjectNumber = objectNumber;
        _encryptGeneration = generation;
    }

    std::optional<std::uint32_t> trailer::encrypt_object_number() const
    {
        return _encryptObjectNumber;
    }

    std::optional<std::uint16_t> trailer::encrypt_generation() const
    {
        return _encryptGeneration;
    }

    void trailer::set_id(const std::pair<std::string, std::optional<std::string>> &ids)
    {
        _id = ids;
    }

    const std::optional<std::pair<std::string, std::optional<std::string>>> &trailer::id() const
    {
        return _id;
    }

    void trailer::merge(const trailer &other)
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
