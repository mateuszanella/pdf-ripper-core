#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"

#include "ripper/pdf/core/document/object/object.hpp"

#include <utility>

namespace ripper::pdf::core
{
trailer_manager::trailer_manager(std::vector<trailer> trailers) noexcept
    : trailers_{std::move(trailers)}
{
}

std::vector<trailer>& trailer_manager::trailers() noexcept
{
    return trailers_;
}

const std::vector<trailer>& trailer_manager::trailers() const noexcept
{
    return trailers_;
}

trailer& trailer_manager::active_trailer()
{
    if (trailers_.empty())
        trailers_.emplace_back(dictionary{});

    return trailers_.back();
}

const trailer& trailer_manager::active_trailer() const noexcept
{
    return trailers_.back();
}

void trailer_manager::push(trailer t)
{
    trailers_.push_back(std::move(t));
}

trailer& trailer_manager::push_trailer()
{
    trailers_.emplace_back(dictionary{});
    return trailers_.back();
}

trailer trailer_manager::compiled() const
{
    dictionary merged{};

    for (const auto& t : trailers_)
    {
        for (const auto& [key, val] : t.dictionary().entries())
            merged.set(key, val);
    }

    return trailer{std::move(merged)};
}

std::size_t trailer_manager::size() const noexcept
{
    return trailers_.size();
}

void trailer_manager::squash()
{
    trailer merged = compiled();
    merged.dictionary().remove("Prev");
    trailers_.clear();
    trailers_.push_back(std::move(merged));
}
} // namespace ripper::pdf::core
