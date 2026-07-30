#include "ripper/pdf/core/document/object/object.hpp"

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

dictionary_object make_dict()
{
    return dictionary_object{};
}

stream_object make_stream()
{
    return stream_object{make_dict(), stream{b("")}};
}
} // namespace

// ── construction ──────────────────────────────────────────────────────────────

TEST_CASE("stream_object construction", "[stream_object][construction]")
{
    SECTION("constructs with explicit dictionary_object and stream")
    {
        auto dict = make_dict();
        dict.set("Type", object{name_object{"Example"}});
        auto payload = b("payload");
        auto s = stream_object{std::move(dict), stream{payload}};

        REQUIRE(s.stream().size() == 7);
        REQUIRE(s.stream().data() == payload);

        const auto* type = s.dictionary().get_name("Type");
        REQUIRE(type != nullptr);
        REQUIRE(type->value == "Example");
    }

    SECTION("constructed stream has no implicit Length entry")
    {
        auto data = b("data");
        auto s = stream_object{make_dict(), stream{data}};
        REQUIRE(s.dictionary().get_number("Length") == nullptr);
    }

    SECTION("constructed stream preserves existing Length from dictionary")
    {
        auto dict = make_dict();
        dict.set("Length", object{std::int64_t{42}});
        auto data = b("data");
        auto s = stream_object{std::move(dict), stream{data}};

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 42);
        REQUIRE(s.stream().size() == 4); // actual data length differs
    }

    SECTION("constructed stream with empty data")
    {
        auto s = stream_object{make_dict(), stream{b("")}};
        REQUIRE(s.stream().size() == 0);
        REQUIRE(s.stream().data().empty());
    }

    SECTION("constructed stream preserves exact byte content")
    {
        auto input = b("AB\x00\xFF");
        auto s = stream_object{make_dict(), stream{input}};
        REQUIRE(s.stream().data() == input);
    }
}

// ── dictionary() / stream() access ────────────────────────────────────────────

TEST_CASE("stream_object dictionary_object access", "[stream_object][access]")
{
    SECTION("const dictionary_object access works")
    {
        auto dict = make_dict();
        dict.set("A", object{std::int64_t{1}});
        const auto s = stream_object{std::move(dict), stream{b("")}};

        REQUIRE(s.dictionary().size() == 1);
        const auto* a = s.dictionary().get_number("A");
        REQUIRE(a != nullptr);
        REQUIRE(a->as_integer() == 1);
    }

    SECTION("mutable dictionary_object access allows modification")
    {
        auto s = make_stream();
        s.dictionary().set("NewKey", object{name_object{"NewValue"}});

        const auto* nv = s.dictionary().get_name("NewKey");
        REQUIRE(nv != nullptr);
        REQUIRE(nv->value == "NewValue");
    }

    SECTION("mutable dictionary_object access allows removing entries")
    {
        auto dict = make_dict();
        dict.set("RemoveMe", object{std::int64_t{99}});
        auto s = stream_object{std::move(dict), stream{b("")}};

        REQUIRE(s.dictionary().contains("RemoveMe"));
        s.dictionary().remove("RemoveMe");
        REQUIRE_FALSE(s.dictionary().contains("RemoveMe"));
    }
}

TEST_CASE("stream_object stream access", "[stream_object][access]")
{
    SECTION("const stream access works")
    {
        auto data = b("const_data");
        const auto s = stream_object{make_dict(), stream{data}};

        REQUIRE(s.stream().size() == 10);
        REQUIRE(s.stream().data() == data);
    }

    SECTION("mutable stream access allows direct modification")
    {
        auto s = make_stream();
        s.stream().data().push_back(static_cast<std::byte>('X'));
        REQUIRE(s.stream().size() == 1);
        REQUIRE(s.stream().data()[0] == static_cast<std::byte>('X'));
    }
}

// ── set_length() ──────────────────────────────────────────────────────────────

TEST_CASE("stream_object set_length", "[stream_object][length]")
{
    SECTION("set length to zero")
    {
        auto s = make_stream();
        s.set_length(0);

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 0);
    }

    SECTION("set length to positive value")
    {
        auto s = make_stream();
        s.set_length(500);

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 500);
    }

    SECTION("set length to large value")
    {
        auto s = make_stream();
        s.set_length(123456789);

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 123456789);
    }

    SECTION("set length overwrites existing value")
    {
        auto s = make_stream();
        s.set_length(10);
        s.set_length(20);
        s.set_length(5);

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 5);
    }

    SECTION("set length does not change stream data")
    {
        auto s = make_stream();
        s.set_length(999);
        REQUIRE(s.stream().size() == 0);
        REQUIRE(s.stream().data().empty());
    }

    SECTION("set_length on stream with existing Length entry")
    {
        auto dict = make_dict();
        dict.set("Length", object{std::int64_t{42}});
        auto data = b("data");
        auto s = stream_object{std::move(dict), stream{data}};

        s.set_length(100);
        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 100);
    }
}

// ── sync_length() ─────────────────────────────────────────────────────────────

TEST_CASE("stream_object sync_length", "[stream_object][length]")
{
    SECTION("sync length sets Length from actual stream size")
    {
        auto data = b("hello");
        auto s = stream_object{make_dict(), stream{data}};
        s.sync_length();

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 5);
    }

    SECTION("sync length on empty stream")
    {
        auto s = make_stream();
        s.sync_length();

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 0);
    }

    SECTION("sync length overwrites previous manual value")
    {
        auto data = b("seven!!");
        auto s = stream_object{make_dict(), stream{data}};
        s.set_length(999);
        s.sync_length();

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 7);
    }

    SECTION("sync length after direct stream mutation via mutable access")
    {
        auto s = make_stream();
        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0);

        s.stream().data().push_back(static_cast<std::byte>('A'));
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0); // not synced

        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 1);
    }

    SECTION("multiple syncs are idempotent")
    {
        auto data = b("test");
        auto s = stream_object{make_dict(), stream{data}};
        s.sync_length();
        s.sync_length();
        s.sync_length();

        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 4);
    }

    SECTION("sync adds Length entry when not present")
    {
        auto dict = make_dict();
        dict.set("Type", object{name_object{"ObjStm"}});
        auto s = stream_object{std::move(dict), stream{b("abc")}};

        REQUIRE_FALSE(s.dictionary().contains("Length"));
        s.sync_length();
        REQUIRE(s.dictionary().contains("Length"));
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 3);
        REQUIRE(s.dictionary().contains("Type")); // other keys preserved
    }

    SECTION("sync length after clearing stream data")
    {
        auto s = stream_object{make_dict(), stream{b("content")}};
        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 7);

        s.stream().data().clear();
        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0);
    }
}

// ── write() happy paths ───────────────────────────────────────────────────────

TEST_CASE("stream_object write happy path", "[stream_object][write]")
{
    SECTION("write auto-syncs length")
    {
        auto s = make_stream();
        REQUIRE(s.dictionary().get_number("Length") == nullptr);

        auto data = b("hello");
        s.write(data);

        REQUIRE(s.stream().size() == 5);
        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 5);
    }

    SECTION("write single byte")
    {
        auto s = make_stream();
        std::byte w = static_cast<std::byte>('Z');
        s.write(std::span{&w, 1});

        REQUIRE(s.stream().size() == 1);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 1);
        REQUIRE(s.stream().data()[0] == static_cast<std::byte>('Z'));
    }

    SECTION("write multiple bytes")
    {
        auto s = make_stream();
        auto data = b("payload");
        s.write(data);

        REQUIRE(s.stream().data() == data);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 7);
    }

    SECTION("write empty span leaves length unchanged if synced before")
    {
        auto s = make_stream();
        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0);

        s.write(std::span<std::byte>{});
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0);
    }

    SECTION("write empty span sets Length to 0 if not previously set")
    {
        auto s = make_stream();
        s.write(std::span<std::byte>{});
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0);
    }

    SECTION("multiple writes accumulate and stay synced")
    {
        auto s = make_stream();
        auto a = b("111");
        auto b2 = b("222");
        auto c = b("333");

        s.write(a);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 3);

        s.write(b2);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 6);

        s.write(c);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 9);
        REQUIRE(s.stream().data() == b("111222333"));
    }

    SECTION("write after construction preserves append order")
    {
        auto s = make_stream();
        for (int i = 0; i < 5; ++i)
        {
            std::byte w = static_cast<std::byte>('A' + i);
            s.write(std::span{&w, 1});
        }
        REQUIRE(s.stream().data() == b("ABCDE"));
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 5);
    }
}

// ── write() edge / corrupt-style cases ────────────────────────────────────────

TEST_CASE("stream_object write edge cases", "[stream_object][write][edge]")
{
    SECTION("write after manual length tampering gets overwritten")
    {
        auto s = make_stream();
        s.set_length(555);

        auto data = b("real");
        s.write(data);

        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 4);
    }

    SECTION("write does not trigger sync on direct stream().write()")
    {
        auto s = make_stream();
        s.sync_length();
        s.set_length(99);

        auto data = b("data");
        s.stream().write(data);

        REQUIRE(s.stream().size() == 4);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 99); // unchanged
    }

    SECTION("write through stream_object does sync after direct stream write")
    {
        auto s = make_stream();

        auto data1 = b("before_");
        s.stream().write(data1);
        REQUIRE(s.dictionary().get_number("Length") == nullptr);

        auto data2 = b("after");
        s.write(data2);

        REQUIRE(s.stream().size() == 12); // 7 + 5
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 12);
    }

    SECTION("write preserves all byte values including nulls and 0xFF")
    {
        auto s = make_stream();
        std::vector<std::byte> all;
        for (int i = 0; i < 256; ++i)
            all.push_back(static_cast<std::byte>(i));
        s.write(all);

        REQUIRE(s.stream().size() == 256);
        REQUIRE(s.stream().data() == all);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 256);
    }

    SECTION("write interleaved with set_length retains user intent on last call")
    {
        auto s = make_stream();
        auto h = b("hello");
        s.write(h);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 5);

        s.set_length(100);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 100);

        auto w = b("world");
        s.write(w);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 10); // write() re-syncs
    }

    SECTION("write when dictionary_object already holds non-integer Length key")
    {
        auto dict = make_dict();
        dict.set("Length", object{name_object{"NotAnInt"}});

        // Before write, Length is a name, not an integer
        REQUIRE(dict.get_number("Length") == nullptr);
        const auto* name_val = dict.get_name("Length");
        REQUIRE(name_val != nullptr);
        REQUIRE(name_val->value == "NotAnInt");

        auto initial_data = b("abc");
        auto s = stream_object{std::move(dict), stream{initial_data}};

        auto more_data = b("def");
        s.write(more_data); // syncs Length as integer, overwriting name

        // Stream has both initial + written data
        REQUIRE(s.stream().size() == 6);
        const auto* length = s.dictionary().get_number("Length");
        REQUIRE(length != nullptr);
        REQUIRE(length->as_integer() == 6);
    }

    SECTION("write after dictionary_object was mutated externally via reference")
    {
        auto s = make_stream();
        auto& dict = s.dictionary();
        dict.set("Filter", object{name_object{"FlateDecode"}});

        auto compressed = b("compressed");
        s.write(compressed);

        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 10);
        const auto* filter = s.dictionary().get_name("Filter");
        REQUIRE(filter != nullptr);
        REQUIRE(filter->value == "FlateDecode");
    }

    SECTION("write large chunk then verify length")
    {
        auto s = make_stream();
        auto chunk = z(1024 * 1024); // 1 MB
        s.write(chunk);

        REQUIRE(s.stream().size() == chunk.size());
        REQUIRE(s.dictionary().get_number("Length")->as_integer() ==
                static_cast<std::int64_t>(chunk.size()));
    }

    SECTION("write many small chunks and verify length stays accurate")
    {
        auto s = make_stream();
        std::size_t total = 0;
        for (std::size_t i = 1; i <= 200; ++i)
        {
            auto chunk = z(i);
            total += chunk.size();
            s.write(chunk);
            REQUIRE(s.dictionary().get_number("Length")->as_integer() ==
                    static_cast<std::int64_t>(total));
        }
    }
}

// ── interaction between stream and dictionary ─────────────────────────────────

TEST_CASE("stream_object stream-dictionary_object interaction", "[stream_object][interaction]")
{
    SECTION("mutating stream via mutable data() then sync catches up")
    {
        auto s = make_stream();
        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0);

        auto& raw = s.stream().data();
        raw.resize(500, static_cast<std::byte>(0xBB));
        REQUIRE(s.stream().size() == 500);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 0); // still old value

        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 500);
    }

    SECTION("set_length to smaller value then write corrects it")
    {
        auto s = make_stream();
        auto text = b("lengthy text here");
        s.write(text);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 17);

        s.set_length(3);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 3);

        auto more = b("more");
        s.write(more);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 21);
        REQUIRE(s.stream().size() == 21);
    }

    SECTION("sync then set_length to arbitrary value (user override)")
    {
        auto s = make_stream();
        auto real = b("real data");
        s.write(real);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 9);

        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 9);

        s.set_length(99999);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 99999);

        REQUIRE(s.stream().size() == 9);
        REQUIRE(s.stream().data() == real);
    }

    SECTION("multiple operations interleaved")
    {
        auto dict = make_dict();
        dict.set("Type", object{name_object{"ObjStm"}});
        auto s = stream_object{std::move(dict), stream{b("")}};

        auto first = b("first");
        s.write(first);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 5);

        s.set_length(1);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 1);

        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 5);

        auto second = b("_second");
        s.stream().write(second);
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 5);

        s.sync_length();
        REQUIRE(s.dictionary().get_number("Length")->as_integer() == 12);

        const auto* type = s.dictionary().get_name("Type");
        REQUIRE(type != nullptr);
        REQUIRE(type->value == "ObjStm");
    }
}
} // namespace ripper::pdf::core
