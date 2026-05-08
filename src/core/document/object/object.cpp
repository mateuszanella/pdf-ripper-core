#include "core/document/object/object.hpp"

#include "core/document/object/indirect_object.hpp"
#include "core/document/object/stream.hpp"
#include "core/document/object/value.hpp"

namespace ripper::core
{
    object::object(indirect_object identity, class value content) noexcept
        : identity_(std::move(identity)), content_(std::move(content)), stream_(std::nullopt)
    {
    }

    object::object(indirect_object identity, class value content, class stream stream) noexcept
        : identity_(std::move(identity)), content_(std::move(content)), stream_(std::move(stream))
    {
    }

    const indirect_object &object::identity() const noexcept
    {
        return identity_;
    }

    const value &object::content() const noexcept
    {
        return content_;
    }

    value &object::content() noexcept
    {
        return content_;
    }

    const dictionary *object::dictionary() const noexcept
    {
        return content_.as_dictionary();
    }

    dictionary *object::dictionary() noexcept
    {
        return content_.as_dictionary();
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

    void object::set_stream(class stream stream) noexcept
    {
        stream_ = std::move(stream);
    }
}
