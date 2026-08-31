#include "ripper/pdf/core/filter/crypt_filter.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

namespace ripper::pdf::core
{

std::vector<std::byte> crypt_filter::decode(std::span<const std::byte>,
                                            const dictionary_object*) const
{
    throw not_implemented_exception{"Crypt is not implemented yet"};
}

std::vector<std::byte> crypt_filter::encode(std::span<const std::byte>,
                                            const dictionary_object*) const
{
    throw not_implemented_exception{"Crypt is not implemented yet"};
}

} // namespace ripper::pdf::core