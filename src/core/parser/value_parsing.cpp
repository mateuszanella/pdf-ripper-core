#include "core/parser/value_parsing.hpp"

#include <charconv>
#include <limits>
#include <string>

#include "core/exceptions/exception.hpp"
#include "core/util/text.hpp"

namespace ripper::pdf::core
{
    indirect_reference parse_indirect_reference(pdf_lexer &lexer)
    {
        auto obj = lexer.next();
        auto gen = lexer.next();
        auto marker = lexer.next();

        if (obj.type != lexer_token_type::integer ||
            gen.type != lexer_token_type::integer ||
            marker.type != lexer_token_type::keyword ||
            marker.lexeme != "R")
            throw parse_exception{"Invalid indirect reference"};

        const auto obj_num = text::parse_size_t(obj.lexeme);
        const auto gen_num = text::parse_size_t(gen.lexeme);

        if (!obj_num || !gen_num ||
            *obj_num > std::numeric_limits<std::uint32_t>::max() ||
            *gen_num > std::numeric_limits<std::uint16_t>::max())
            throw parse_exception{"Indirect reference numbers are out of range"};

        return indirect_reference{
            static_cast<std::uint32_t>(*obj_num),
            static_cast<std::uint16_t>(*gen_num)};
    }

    array parse_array(pdf_lexer &lexer)
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

    dictionary parse_dictionary(pdf_lexer &lexer)
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

            dict.set(std::string{key.lexeme}, std::move(val));
        }
        return dict;
    }

    value parse_value(pdf_lexer &lexer)
    {
        auto p = lexer.peek();

        if (p.type == lexer_token_type::integer)
        {
            // Peek ahead to detect `obj gen R` before consuming anything.
            auto p1 = lexer.peek(1);
            auto p2 = lexer.peek(2);
            if (p1.type == lexer_token_type::integer &&
                p2.type == lexer_token_type::keyword && p2.lexeme == "R")
            {
                auto ref = parse_indirect_reference(lexer);
                return value{ref};
            }

            const auto tok = lexer.next();
            std::int64_t i = 0;
            auto [ptr, ec] = std::from_chars(tok.lexeme.data(), tok.lexeme.data() + tok.lexeme.size(), i);
            if (ec == std::errc{})
                return value{i};
            return value{std::string{tok.lexeme}};
        }

        if (p.type == lexer_token_type::real)
        {
            const auto tok = lexer.next();
            double d = 0.0;
            auto [ptr, ec] = std::from_chars(tok.lexeme.data(), tok.lexeme.data() + tok.lexeme.size(), d);
            if (ec == std::errc{})
                return value{d};
            return value{std::string{tok.lexeme}};
        }

        if (p.type == lexer_token_type::name)
        {
            const auto tok = lexer.next();
            return value{name{std::string{tok.lexeme}}};
        }

        if (p.type == lexer_token_type::literal_string ||
            p.type == lexer_token_type::hex_string)
        {
            const auto tok = lexer.next();
            return value{std::string{tok.lexeme}};
        }

        if (p.type == lexer_token_type::keyword)
        {
            const auto tok = lexer.next();
            if (tok.lexeme == "true")
                return value{true};

            if (tok.lexeme == "false")
                return value{false};

            return value{};
        }

        if (p.type == lexer_token_type::array_begin)
        {
            (void)lexer.next();

            auto arr = parse_array(lexer);
            return value{std::move(arr)};
        }

        if (p.type == lexer_token_type::dictionary_begin)
        {
            (void)lexer.next();

            auto dict = parse_dictionary(lexer);
            return value{std::move(dict)};
        }

        // Skip unrecognized tokens
        lexer.skip_value();

        return value{};
    }
}
