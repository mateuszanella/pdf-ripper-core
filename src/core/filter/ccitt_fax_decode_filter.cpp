#include "ripper/pdf/core/filter/ccitt_fax_decode_filter.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

namespace ripper::pdf::core
{

std::vector<std::byte> ccitt_fax_decode_filter::decode(std::span<const std::byte>,
                                                       const dictionary_object*) const
{
    throw not_implemented_exception{"CCITTFaxDecode is not implemented yet"};
}

std::vector<std::byte> ccitt_fax_decode_filter::encode(std::span<const std::byte>,
                                                       const dictionary_object*) const
{
    throw not_implemented_exception{"CCITTFaxDecode is not implemented yet"};
}

} // namespace ripper::pdf::core