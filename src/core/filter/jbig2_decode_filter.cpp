#include "ripper/pdf/core/filter/jbig2_decode_filter.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

namespace ripper::pdf::core
{

std::vector<std::byte> jbig2_decode_filter::decode(std::span<const std::byte>,
                                                   const dictionary_object*) const
{
    throw not_implemented_exception{"JBIG2Decode is not implemented yet"};
}

std::vector<std::byte> jbig2_decode_filter::encode(std::span<const std::byte>,
                                                   const dictionary_object*) const
{
    throw not_implemented_exception{"JBIG2Decode is not implemented yet"};
}

} // namespace ripper::pdf::core