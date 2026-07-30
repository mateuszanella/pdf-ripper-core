#include "ripper/pdf/core/document/object/helpers/stream.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
namespace
{
std::vector<std::byte> b(std::string_view sv)
{
    std::vector<std::byte> v;
    v.reserve(sv.size());
    for (char c : sv)
        v.push_back(static_cast<std::byte>(c));
    return v;
}

std::vector<std::byte> z(std::size_t n)
{
    std::vector<std::byte> v(n);
    std::fill(v.begin(), v.end(), static_cast<std::byte>(0xAA));
    return v;
}
} // namespace

// ── construction ──────────────────────────────────────────────────────────────

TEST_CASE("stream construction", "[stream][construction]")
{
    SECTION("constructs with empty data")
    {
        stream s(b(""));
        REQUIRE(s.size() == 0);
        REQUIRE(s.data().empty());
    }

    SECTION("constructs with non-empty data")
    {
        stream s(b("hello"));
        REQUIRE(s.size() == 5);
        REQUIRE_FALSE(s.data().empty());
    }

    SECTION("constructed stream preserves byte content exactly")
    {
        auto input = b("ABC\x00\xFF");
        stream s(input);
        REQUIRE(s.data() == input);
    }

    SECTION("constructed stream preserves size")
    {
        auto input = b("1234567890");
        stream s(input);
        REQUIRE(s.size() == input.size());
        REQUIRE(s.size() == 10);
    }
}

// ── data() access ─────────────────────────────────────────────────────────────

TEST_CASE("stream data access", "[stream][data]")
{
    SECTION("const data returns reference to internal storage")
    {
        auto input = b("test");
        const stream s(input);
        REQUIRE(s.data().size() == 4);
        REQUIRE(s.data()[0] == static_cast<std::byte>('t'));
        REQUIRE(s.data()[3] == static_cast<std::byte>('t'));
    }

    SECTION("mutable data allows in-place editing")
    {
        stream s(b("ABCD"));
        s.data()[0] = static_cast<std::byte>('X');
        s.data()[3] = static_cast<std::byte>('Z');
        REQUIRE(s.data() == b("XBCZ"));
        REQUIRE(s.size() == 4);
    }

    SECTION("mutable data allows insertion")
    {
        stream s(b("AB"));
        s.data().push_back(static_cast<std::byte>('C'));
        REQUIRE(s.data() == b("ABC"));
    }

    SECTION("mutable data allows erasure")
    {
        stream s(b("XYZ"));
        s.data().clear();
        REQUIRE(s.data().empty());
        REQUIRE(s.size() == 0);
    }

    SECTION("data reflects writes")
    {
        stream s(b(""));
        auto data = b("a");
        s.write(data);
        REQUIRE(s.data() == b("a"));
    }

    SECTION("data reflects direct vector operations")
    {
        stream s(b(""));
        s.data().resize(3, static_cast<std::byte>(0x42));
        REQUIRE(s.size() == 3);
        REQUIRE(s.data()[0] == static_cast<std::byte>(0x42));
        REQUIRE(s.data()[2] == static_cast<std::byte>(0x42));
    }
}

// ── size() ────────────────────────────────────────────────────────────────────

TEST_CASE("stream size", "[stream][size]")
{
    SECTION("empty stream has size 0")
    {
        stream s(b(""));
        REQUIRE(s.size() == 0);
    }

    SECTION("size reflects initial constructor data")
    {
        stream s(b("hello world"));
        REQUIRE(s.size() == 11);
    }

    SECTION("size updates after write")
    {
        stream s(b(""));
        auto data = b("XY");
        s.write(data);
        REQUIRE(s.size() == 2);
    }

    SECTION("size updates after multiple writes")
    {
        stream s(b(""));
        auto a = b("abc");
        auto b_ = b("def");
        auto c = b("ghi");
        s.write(a);
        s.write(b_);
        s.write(c);
        REQUIRE(s.size() == 9);
    }

    SECTION("size reflects direct data mutation")
    {
        stream s(b(""));
        s.data().resize(100);
        REQUIRE(s.size() == 100);
    }

    SECTION("size is zero after clear via data()")
    {
        stream s(b("data"));
        s.data().clear();
        REQUIRE(s.size() == 0);
    }
}

// ── write() happy paths ───────────────────────────────────────────────────────

TEST_CASE("stream write happy path", "[stream][write]")
{
    SECTION("write single byte")
    {
        stream s(b(""));
        std::byte w = static_cast<std::byte>('A');
        s.write(std::span{&w, 1});
        REQUIRE(s.size() == 1);
        REQUIRE(s.data()[0] == static_cast<std::byte>('A'));
    }

    SECTION("write multiple bytes")
    {
        stream s(b(""));
        auto input = b("hello");
        s.write(input);
        REQUIRE(s.data() == input);
        REQUIRE(s.size() == 5);
    }

    SECTION("write empty span is a no-op")
    {
        stream s(b("data"));
        auto size_before = s.size();
        s.write(std::span<std::byte>{});
        REQUIRE(s.size() == size_before);
        REQUIRE(s.data() == b("data"));
    }

    SECTION("write appends to existing data")
    {
        stream s(b("start_"));
        auto middle = b("middle_");
        auto end = b("end");
        s.write(middle);
        s.write(end);
        REQUIRE(s.data() == b("start_middle_end"));
        REQUIRE(s.size() == 16);
    }

    SECTION("multiple writes preserve append order")
    {
        stream s(b(""));
        for (int i = 0; i < 10; ++i)
        {
            std::byte w = static_cast<std::byte>('0' + i);
            s.write(std::span{&w, 1});
        }
        REQUIRE(s.data() == b("0123456789"));
    }

    SECTION("write large amount of data")
    {
        stream s(b(""));
        auto chunk = z(1024 * 1024); // 1 MB
        s.write(chunk);
        REQUIRE(s.size() == chunk.size());
        REQUIRE(s.data() == chunk);
    }

    SECTION("write spanning a range from a sub-span")
    {
        stream s(b(""));
        auto full = b("ABCDEFGH");
        std::span<std::byte> sub{full.data() + 2, 4}; // "CDEF"
        s.write(sub);
        REQUIRE(s.data() == b("CDEF"));
        REQUIRE(s.size() == 4);
    }

    SECTION("write many small chunks")
    {
        stream s(b(""));
        std::size_t total = 0;
        for (std::size_t i = 1; i <= 100; ++i)
        {
            auto chunk = z(i);
            total += chunk.size();
            s.write(chunk);
        }
        REQUIRE(s.size() == total);
    }
}

// ── write() edge / corrupt-style cases ────────────────────────────────────────

TEST_CASE("stream write edge cases", "[stream][write][edge]")
{
    SECTION("write zero byte (null byte) is preserved")
    {
        stream s(b(""));
        std::byte zero{0};
        s.write(std::span{&zero, 1});
        REQUIRE(s.size() == 1);
        REQUIRE(s.data()[0] == std::byte{0});
    }

    SECTION("write 0xFF byte is preserved")
    {
        stream s(b(""));
        std::byte ff{0xFF};
        s.write(std::span{&ff, 1});
        REQUIRE(s.data()[0] == std::byte{0xFF});
    }

    SECTION("write all byte values from 0x00 to 0xFF")
    {
        stream s(b(""));
        std::vector<std::byte> all;
        for (int i = 0; i < 256; ++i)
            all.push_back(static_cast<std::byte>(i));
        s.write(all);
        REQUIRE(s.size() == 256);
        REQUIRE(s.data() == all);
    }

    SECTION("write after direct data clear")
    {
        stream s(b("original"));
        s.data().clear();
        REQUIRE(s.size() == 0);
        auto new_data = b("new");
        s.write(new_data);
        REQUIRE(s.data() == b("new"));
        REQUIRE(s.size() == 3);
    }

    SECTION("write after direct resize and fill")
    {
        stream s(b("AB"));
        s.data().resize(10, static_cast<std::byte>('X'));
        auto tail = b("YZ");
        s.write(tail);
        REQUIRE(s.size() == 12);
        REQUIRE(s.data().front() == static_cast<std::byte>('A'));
        REQUIRE(s.data()[9] == static_cast<std::byte>('X'));
        REQUIRE(s.data()[10] == static_cast<std::byte>('Y'));
        REQUIRE(s.data()[11] == static_cast<std::byte>('Z'));
    }

    SECTION("write after data shrinkage then expansion")
    {
        stream s(b("ABCDEFGH"));
        s.data().resize(2); // "AB"
        auto add = b("CD");
        s.write(add); // "ABCD"
        REQUIRE(s.data() == b("ABCD"));
    }

    SECTION("write interleaved with direct data mutation")
    {
        stream s(b("init_"));
        auto w1 = b("w1_");
        s.write(w1);
        s.data().push_back(static_cast<std::byte>('['));
        s.data().push_back(static_cast<std::byte>('m'));
        s.data().push_back(static_cast<std::byte>(']'));
        auto w2 = b("_w2");
        s.write(w2);
        REQUIRE(s.data() == b("init_w1_[m]_w2"));
    }
}

// ── write() large / stress ────────────────────────────────────────────────────

TEST_CASE("stream write stress", "[stream][write][stress]")
{
    SECTION("write individual bytes in tight loop")
    {
        stream s(b(""));
        const std::size_t count = 10000;
        for (std::size_t i = 0; i < count; ++i)
        {
            std::byte w = static_cast<std::byte>(i % 256);
            s.write(std::span{&w, 1});
        }
        REQUIRE(s.size() == count);
    }

    SECTION("write alternately sized chunks")
    {
        stream s(b(""));
        std::size_t total = 0;
        for (std::size_t i = 0; i < 500; ++i)
        {
            auto chunk = z((i % 7) + 1);
            total += chunk.size();
            s.write(chunk);
        }
        REQUIRE(s.size() == total);
    }

    SECTION("write chunks with varying spans")
    {
        stream s(b(""));
        auto base = z(100);
        for (std::size_t off = 0; off < 50; ++off)
        {
            std::span<std::byte> sub{base.data(), off + 1};
            s.write(sub);
        }
        REQUIRE(s.size() == 1275); // sum of 1..50
    }

    SECTION("write 2 MB of data")
    {
        stream s(b(""));
        auto chunk = z(2 * 1024 * 1024);
        s.write(chunk);
        REQUIRE(s.size() == chunk.size());
        REQUIRE(s.data() == chunk);
    }

    SECTION("write after constructing with non-empty data")
    {
        auto prefix = b("PREFIX_");
        auto suffix = z(1024);
        stream s(std::move(prefix));
        s.write(suffix);
        REQUIRE(s.size() == 7 + 1024);
        auto expected_prefix = b("PREFIX_");
        REQUIRE(std::equal(s.data().begin(), s.data().begin() + 7, expected_prefix.begin()));
    }
}
} // namespace ripper::pdf::core
