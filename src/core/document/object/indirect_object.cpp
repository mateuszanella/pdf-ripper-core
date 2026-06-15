#include "core/document/object/indirect_object.hpp"

#include "core/document/object/object.hpp"
#include "core/document/object/stream.hpp"
namespace ripper::pdf::core
{
indirect_object::indirect_object(object_identity identity, class object content) noexcept
    : identity_(std::move(identity)), content_(std::move(content))
{
}

const object_identity& indirect_object::identity() const noexcept
{
    return identity_;
}

object_identity& indirect_object::identity() noexcept
{
    return identity_;
}

const object& indirect_object::content() const noexcept
{
    return content_;
}

object& indirect_object::content() noexcept
{
    return content_;
}

const dictionary* indirect_object::dictionary() const noexcept
{
    return content_.as_dictionary();
}

dictionary* indirect_object::dictionary() noexcept
{
    return content_.as_dictionary();
}
} // namespace ripper::pdf::core
