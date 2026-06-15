#pragma once

#include "core/document/object/indirect_object.hpp"
#include "core/document/object/object.hpp"
#include "core/document/object/object_view.hpp"

#include <optional>

namespace ripper::pdf::core
{
class pages;

class page : public object_view
{
public:
    explicit page(indirect_object& obj) noexcept;

    /// Returns the parent /Pages node of this page, or std::nullopt if /Parent is absent
    /// or cannot be resolved.
    ///
    /// Per the PDF spec (§7.7.3.3) the /Parent of every /Page leaf is always a /Pages node,
    /// so the returned view exposes the full pages API (count, each, add_page, delete_page…).
    [[nodiscard]] std::optional<class pages> parent();
};
} // namespace ripper::pdf::core
