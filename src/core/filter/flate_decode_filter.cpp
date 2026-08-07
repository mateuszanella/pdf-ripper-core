#include "ripper/pdf/core/filter/flate_decode_filter.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

#include <zlib.h>

namespace ripper::pdf::core
{
namespace
{

[[nodiscard]] Bytef* as_zlib_buffer(std::byte* p) noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Bytef*>(p);
}

[[nodiscard]] const Bytef* as_zlib_buffer(const std::byte* p) noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const Bytef*>(p);
}

} // namespace

std::vector<std::byte> flate_decode_filter::decode(std::span<const std::byte> input,
                                                   const dictionary_object* /*params*/) const
{
    if (input.empty())
    {
        return {};
    }

    std::size_t outputSize = std::max<std::size_t>(input.size() * 10, 4096);

    for (;;)
    {
        std::vector<std::byte> output(outputSize);
        uLongf decompressedSize = static_cast<uLongf>(outputSize);

        const int result =
            uncompress(as_zlib_buffer(output.data()), &decompressedSize,
                       as_zlib_buffer(input.data()), static_cast<uLong>(input.size()));

        if (result == Z_OK)
        {
            output.resize(decompressedSize);
            return output;
        }

        if (result == Z_BUF_ERROR)
        {
            outputSize *= 2;
            continue;
        }

        switch (result)
        {
            case Z_MEM_ERROR:
                throw io_exception{"Decompression failed due to memory error"};
            case Z_DATA_ERROR:
                throw parse_exception{"Compressed stream is corrupted"};
            default:
                throw io_exception{"Decompression failed"};
        }
    }
}

std::vector<std::byte> flate_decode_filter::encode(std::span<const std::byte> input,
                                                   const dictionary_object* /*params*/) const
{
    if (input.empty())
    {
        return {};
    }

    const uLongf maxSize = compressBound(static_cast<uLong>(input.size()));
    std::vector<std::byte> output(static_cast<std::size_t>(maxSize));

    uLongf compressedSize = maxSize;
    const int result = ::compress(as_zlib_buffer(output.data()), &compressedSize,
                                  as_zlib_buffer(input.data()), static_cast<uLong>(input.size()));

    if (result != Z_OK)
    {
        switch (result)
        {
            case Z_MEM_ERROR:
                throw io_exception{"Compression failed due to memory error"};
            case Z_BUF_ERROR:
                throw io_exception{"Compression output buffer too small"};
            default:
                throw io_exception{"Compression failed"};
        }
    }

    output.resize(static_cast<std::size_t>(compressedSize));
    return output;
}

} // namespace ripper::pdf::core
