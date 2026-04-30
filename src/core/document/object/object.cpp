#include "core/document/object/object.hpp"

#include "core/document/object/dictionary.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/object/stream.hpp"

namespace ripper::core
{
    object::object(indirect_object identity, dictionary dictionary) noexcept
        : identity_(std::move(identity)), dictionary_(std::move(dictionary)), stream_(std::nullopt)
    {
    }

    object::object(indirect_object identity, dictionary dictionary, stream stream) noexcept
        : identity_(std::move(identity)), dictionary_(std::move(dictionary)), stream_(std::move(stream))
    {
    }

    const indirect_object &object::identity() const noexcept
    {
        return identity_;
    }

    const dictionary &object::dictionary() const noexcept
    {
        return dictionary_;
    }

    dictionary &object::dictionary() noexcept
    {
        return dictionary_;
    }

    bool object::has_stream() const noexcept
    {
        return stream_.has_value();
    }

    const stream *object::stream() const noexcept
    {
        return stream_ ? &stream_.value() : nullptr;
    }

    stream *object::stream() noexcept
    {
        return stream_ ? &stream_.value() : nullptr;
    }

    void object::set_stream(stream stream) noexcept
    {
        stream_ = std::move(stream);
    }
}
