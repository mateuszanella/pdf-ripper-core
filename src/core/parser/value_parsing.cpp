#include "ripper/pdf/core/parser/value_parsing.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/util/text.hpp"

#include <charconv>
#include <limits>
#include <string>

namespace ripper::pdf::core
{
indirect_reference parse_indirect_reference(pdf_lexer& lexer)
{
    auto obj = lexer.next();
    auto gen = lexer.next();
    auto marker = lexer.next();

    if (obj.type != lexer_token_type::integer || gen.type != lexer_token_type::integer ||
        marker.type != lexer_token_type::keyword || marker.lexeme != "R")
        throw parse_exception{"Invalid indirect reference"};

    const auto obj_num = text::parse_size_t(obj.lexeme);
    const auto gen_num = text::parse_size_t(gen.lexeme);

    if (!obj_num || !gen_num || *obj_num > std::numeric_limits<std::uint32_t>::max() ||
        *gen_num > std::numeric_limits<std::uint16_t>::max())
        throw parse_exception{"Indirect reference numbers are out of range"};

    return indirect_reference{static_cast<std::uint32_t>(*obj_num),
                              static_cast<std::uint16_t>(*gen_num)};
}

array parse_array(pdf_lexer& lexer)
{
    array arr;
    while (true)
    {
        auto p = lexer.peek();

        if (p.type == lexer_token_type::array_end)
        {
            (void)lexer.next();
            break;
        }

        if (p.type == lexer_token_type::eof)
            throw parse_exception{"Unexpected EOF inside array"};

        auto v = parse_value(lexer);
        arr.push_back(std::move(v));
    }
    return arr;
}

dictionary parse_dictionary(pdf_lexer& lexer)
{
    dictionary dict;
    while (true)
    {
        auto p = lexer.peek();

        if (p.type == lexer_token_type::dictionary_end)
        {
            (void)lexer.next();
            break;
        }

        if (p.type == lexer_token_type::eof)
            throw parse_exception{"Unexpected EOF inside nested dictionary"};

        if (p.type != lexer_token_type::name)
        {
            lexer.skip_value();
            continue;
        }

        const auto key = lexer.next();
        auto val = parse_value(lexer);

        dict.set(text::unescape_name(key.lexeme), std::move(val));
    }
    return dict;
}

object parse_value(pdf_lexer& lexer)
{
    auto p = lexer.peek();

    if (p.type == lexer_token_type::integer)
    {
        // Peek ahead to detect `obj gen R` before consuming anything.
        auto p1 = lexer.peek(1);
        auto p2 = lexer.peek(2);
        if (p1.type == lexer_token_type::integer && p2.type == lexer_token_type::keyword &&
            p2.lexeme == "R")
        {
            auto ref = parse_indirect_reference(lexer);
            return object{ref};
        }

        const auto tok = lexer.next();
        std::int64_t i = 0;
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [ptr, ec] =
            std::from_chars(tok.lexeme.data(), tok.lexeme.data() + tok.lexeme.size(), i);
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (ec == std::errc{})
            return object{i};
        return object{std::string{tok.lexeme}};
    }

    if (p.type == lexer_token_type::real)
    {
        const auto tok = lexer.next();
        double d = 0.0;
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [ptr, ec] =
            std::from_chars(tok.lexeme.data(), tok.lexeme.data() + tok.lexeme.size(), d);
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (ec == std::errc{})
            return object{d};
        return object{std::string{tok.lexeme}};
    }

    if (p.type == lexer_token_type::name)
    {
        const auto tok = lexer.next();
        return object{name{text::unescape_name(tok.lexeme)}};
    }

    if (p.type == lexer_token_type::literal_string)
    {
        const auto tok = lexer.next();
        return object{text::unescape_literal_string(tok.lexeme)};
    }

    if (p.type == lexer_token_type::hex_string)
    {
        const auto tok = lexer.next();
        return object{std::string{tok.lexeme}};
    }

    if (p.type == lexer_token_type::keyword)
    {
        const auto tok = lexer.next();
        if (tok.lexeme == "true")
            return object{true};

        if (tok.lexeme == "false")
            return object{false};

        return object{};
    }

    if (p.type == lexer_token_type::array_begin)
    {
        (void)lexer.next();

        auto arr = parse_array(lexer);
        return object{std::move(arr)};
    }

    if (p.type == lexer_token_type::dictionary_begin)
    {
        (void)lexer.next();

        auto dict = parse_dictionary(lexer);
        return object{std::move(dict)};
    }

    // Skip unrecognized tokens
    lexer.skip_value();

    return object{};
}

std::string_view extract_object_body(std::string_view content)
{
    pdf_lexer lexer{content};

    // Find the 'obj' keyword token
    lexer_token obj_token;
    while (true)
    {
        auto token = lexer.next();
        if (token.type == lexer_token_type::eof)
            throw parse_exception{"Missing obj keyword in indirect object content"};

        if (token.type == lexer_token_type::keyword && token.lexeme == "obj")
        {
            obj_token = token;
            break;
        }
    }

    /// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto value_start = static_cast<std::size_t>(obj_token.lexeme.data() +
                                                      obj_token.lexeme.size() - content.data());
    /// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    const auto endobj_pos = content.rfind("endobj");
    if (endobj_pos == std::string_view::npos)
        throw parse_exception{"Missing endobj keyword"};

    return content.substr(value_start, endobj_pos - value_start);
}
} // namespace ripper::pdf::core
