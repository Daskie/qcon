#include <qcon-decode.hpp>

#include <gtest/gtest.h>

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using unat = size_t;

using namespace std::string_literals;
using namespace std::string_view_literals;

using qcon::Decoder;
using qcon::DecodeState;
using qcon::Timepoint;

std::ostream & operator<<(std::ostream & os, const DecodeState state)
{
    switch (state)
    {
        case DecodeState::error: os << "error"; break;
        case DecodeState::object: os << "object"; break;
        case DecodeState::array: os << "array"; break;
        case DecodeState::end: os << "end"; break;
        case DecodeState::string: os << "string"; break;
        case DecodeState::integer: os << "integer"; break;
        case DecodeState::floater: os << "floater"; break;
        case DecodeState::boolean: os << "boolean"; break;
        case DecodeState::null: os << "null"; break;
        case DecodeState::date: os << "date"; break;
        case DecodeState::time: os << "time"; break;
        case DecodeState::datetime: os << "datetime"; break;
        case DecodeState::done: os << "done"; break;
    }

    return os;
}

bool fails(const std::string_view str)
{
    Decoder decoder{str};
    DecodeState state;

    while (true)
    {
        state = decoder.step();

        if (state == DecodeState::error)
        {
            return true;
        }
        else if (state == DecodeState::done)
        {
            return false;
        }
    }
}

TEST(Decode, object)
{
    { // Empty
        Decoder decoder{R"({})"sv};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Single key
        Decoder decoder{R"({ "a": null })"sv};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a"sv);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Multiple keys
        Decoder decoder{R"({ "a": null, "b": null, "c": null })"sv};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a"sv);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "b"sv);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "c"sv);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No space
        Decoder decoder{R"({"a":null,"b":null})"sv};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a"sv);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "b"sv);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Weird spacing
        Decoder decoder{R"({"a" :null ,"b" :null})"sv};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a"sv);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "b"sv);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Empty key
        Decoder decoder{R"({ "": null })"sv};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, ""sv);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No colon after key
        ASSERT_TRUE(fails(R"({ "a" 0 })"));
    }
    { // Key with single quotes
        ASSERT_TRUE(fails(R"({ 'a': 0 })"));
    }
    { // Key without quotes
        ASSERT_TRUE(fails(R"({ a: 0 })"));
    }
    { // Missing value
        ASSERT_TRUE(fails(R"({ "a": })"));
    }
    { // No comma between elements
        ASSERT_TRUE(fails(R"({ "a": 0 "b": 1 })"));
    }
    { // Empty entry
        ASSERT_TRUE(fails(R"({ "a": 0, , "b": 1 })"));
    }
    { // Cut off
        ASSERT_TRUE(fails(R"({)"));
        ASSERT_TRUE(fails(R"({")"));
        ASSERT_TRUE(fails(R"({"a)"));
        ASSERT_TRUE(fails(R"({"a")"));
        ASSERT_TRUE(fails(R"({"a":)"));
        ASSERT_TRUE(fails(R"({"a":0)"));
        ASSERT_TRUE(fails(R"({"a":0,)"));
        ASSERT_TRUE(fails(R"({"a":0,")"));
        ASSERT_TRUE(fails(R"({"a":0,"b)"));
        ASSERT_TRUE(fails(R"({"a":0,"b")"));
        ASSERT_TRUE(fails(R"({"a":0,"b":)"));
        ASSERT_TRUE(fails(R"({"a":0,"b":1)"));
        ASSERT_TRUE(fails(R"("a":0,"b":1})"));
        ASSERT_TRUE(fails(R"(a":0,"b":1})"));
        ASSERT_TRUE(fails(R"(":0,"b":1})"));
        ASSERT_TRUE(fails(R"(:0,"b":1})"));
        ASSERT_TRUE(fails(R"(0,"b":1})"));
        ASSERT_TRUE(fails(R"(,"b":1})"));
        ASSERT_TRUE(fails(R"("b":1})"));
        ASSERT_TRUE(fails(R"(b":1})"));
        ASSERT_TRUE(fails(R"(":1})"));
        ASSERT_TRUE(fails(R"(:1})"));
        ASSERT_TRUE(fails(R"(1})"));
        ASSERT_TRUE(fails(R"(})"));
    }
}

TEST(Decode, array)
{
    { // Empty
        Decoder decoder{R"([])"sv};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Single element
        Decoder decoder{R"([ null ])"sv};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Multiple elements
        Decoder decoder{R"([ null, null, null ])"sv};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No space
        Decoder decoder{R"([null,null])"sv};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Weird spacing
        Decoder decoder{R"([null ,null])"sv};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No comma between elements
        ASSERT_TRUE(fails(R"([ 0 1 ])"));
    }
    { // Empty entry
        ASSERT_TRUE(fails(R"([ 0, , 1 ])"));
    }
    { // Cut off
        ASSERT_TRUE(fails(R"([)"));
        ASSERT_TRUE(fails(R"([0)"));
        ASSERT_TRUE(fails(R"([0,)"));
        ASSERT_TRUE(fails(R"([0,1)"));
        ASSERT_TRUE(fails(R"(0,1])"));
        ASSERT_TRUE(fails(R"(,1])"));
        ASSERT_TRUE(fails(R"(1])"));
        ASSERT_TRUE(fails(R"(])"));
    }
}

TEST(Decode, string)
{
    { // Empty string
        Decoder decoder{R"("")"sv};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, ""sv);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // All ASCII
        Decoder decoder{"\"\\0\\x01\\x02\\x03\\x04\\x05\\x06\\a\\b\\t\\n\\v\\f\\r\\x0E\\x0F\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1A\\x1B\\x1C\\x1D\\x1E\\x1F !\\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F\""sv};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\0\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F"sv);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // All non-ASCII
        std::string actual{};
        for (unat c{128u}; c < 256u; ++c)
        {
            actual.push_back(char(c));
        }
        std::string decodeStr{'"' + actual + '"'};
        Decoder decoder{decodeStr};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, actual);
    }
    { // Missing escape sequence
        const std::string_view brokenSeq{R"("\\\")"};
        ASSERT_TRUE(fails(brokenSeq));
        const std::string_view brokenSeqInArray{R"([ "\\\" ])"};
        ASSERT_TRUE(fails(brokenSeqInArray));
    }
    { // Unknown escape sequence
        ASSERT_TRUE(fails("\"\\\0\""));
    }
    { // 'x' code point
        Decoder decoder{};

        for (unat i{0u}; i < 128u; ++i)
        {
            const std::string expectedStr{char(i)};
            const std::string decodeStr{std::format("\"\\x{:02X}\""sv, i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
        for (unat i{128u}; i < 256u; ++i)
        {
            const std::string expectedStr{char(u8(0b110'00000u | (i >> 6))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\x{:02X}\""sv, i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
    }
    { // 'u' code point
        Decoder decoder{};

        for (unat i{0u}; i < 128u; ++i)
        {
            const std::string expectedStr{char(i)};
            const std::string decodeStr{std::format("\"\\u{:04X}\""sv, i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
        for (unat i{128u}; i < 2048u; ++i)
        {
            const std::string expectedStr{char(u8(0b110'00000u | (i >> 6))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\u{:04X}\""sv, i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
        for (unat i{2048u}; i < 65536u; ++i)
        {
            const std::string expectedStr{char(u8(0b1110'0000u | (i >> 12))), char(u8(0b10'000000u | ((i >> 6) & 0b111111u))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\u{:04X}\""sv, i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
    }
    { // 'U' code point
        Decoder decoder{};

        decoder.load(R"("\U00000000")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string.size(), 1u);
        ASSERT_EQ(decoder.string.front(), '\0');
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"("\U0000007F")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\x7F");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"("\U00000080")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xC2\x80");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"("\U000007FF")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xDF\xBF");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"("\U00000800")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xE0\xA0\x80");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"("\U0000FFFF")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xEF\xBF\xBF");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"("\U00010000")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF0\x90\x80\x80");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"("\U001FFFFF")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF7\xBF\xBF\xBF");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        ASSERT_TRUE(fails("\"\\U00200000\""));
    }
    { // Uppercase and lowercase code point hex digits
        Decoder decoder{R"("\x0a\x0A\x0b\x0B\x0c\x0C\x0d\x0D")"};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\x0A\x0A\x0B\x0B\x0C\x0C\x0D\x0D");
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Incorrect number of code point digits
        // Raw strings, `\x`/`\u`, and macros don't play nice together
        ASSERT_TRUE(fails("\"\\x\""));
        ASSERT_TRUE(fails("\"\\x0\""));
        ASSERT_TRUE(fails("\"\\u\""));
        ASSERT_TRUE(fails("\"\\u0\""));
        ASSERT_TRUE(fails("\"\\u00\""));
        ASSERT_TRUE(fails("\"\\u000\""));
        ASSERT_TRUE(fails("\"\\U\""));
        ASSERT_TRUE(fails("\"\\U0\""));
        ASSERT_TRUE(fails("\"\\U00\""));
        ASSERT_TRUE(fails("\"\\U000\""));
        ASSERT_TRUE(fails("\"\\U0000\""));
        ASSERT_TRUE(fails("\"\\U00000\""));
        ASSERT_TRUE(fails("\"\\U000000\""));
        ASSERT_TRUE(fails("\"\\U0000000\""));
    }
    { // Missing end quote
        ASSERT_TRUE(fails(R"("abc)"));
        ASSERT_TRUE(fails(R"([ "abc ])"));
    }
    { // Escaped newlines
        Decoder decoder{};

        decoder.load("\"a\\\nb\\\nc\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "abc");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"a\\\r\nb\\\r\nc\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "abc");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\\\n\\\n\\\n\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\\\r\n\\\n\\\r\n\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"a\\\rb\\\r\\\nc\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "abc");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        ASSERT_TRUE(fails("\"a\rb\""));
        ASSERT_TRUE(fails("\"a\nb\""));
        ASSERT_TRUE(fails("\"a\r\nb\""));
    }
    { // Single quotes
        ASSERT_TRUE(fails(R"('abc')"));
    }
    { // Unicode limits
        Decoder decoder{};

        decoder.load("\"\\x00\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string.size(), 1u);
        ASSERT_EQ(decoder.string.front(), '\0');
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\\x7F\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\x7F");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\xC2\x80\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xC2\x80");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\xDF\xBF\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xDF\xBF");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\xE0\xA0\x80\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xE0\xA0\x80");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\xEF\xBF\xBF\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xEF\xBF\xBF");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\xF0\x90\x80\x80\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF0\x90\x80\x80");
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("\"\xF7\xBF\xBF\xBF\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF7\xBF\xBF\xBF");
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Unicode string
        // Randomly generated UTF-8
        const std::string_view str{
            "\xd1\x82\xe3\xa7\x8e\xef\xbf\x9d\xdf\x8e\xd3\x81\xef\xa9\xac\xf1\x8d\xb2\xa7\xf1\xbe\xae\x84\x56"
            "\xde\x88\xf1\xa3\xbd\xb4\xf3\xa0\xa3\x90\xeb\xa8\x9d\xef\x83\x8a\xd0\xa5\x4b\xe8\x84\xb3\xc9\x82"
            "\xe4\xbf\xa2\xe2\xaa\xa9\xcc\xbd\xcb\xb5\xd0\x88\xd1\xb0\xf3\x84\x8c\x82\xf3\x9a\x8f\xb2\x4f\xdc\x8b"
            "\xcf\x8a\x60\xdb\x8f\x41"};

        const std::string decodeStr{std::format("\"{}\"", str)};
        Decoder decoder{decodeStr};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, str);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
}

TEST(Decode, decimal)
{
    { // Zero
        Decoder decoder{R"(0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Normal
        Decoder decoder{R"(123)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+123)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer,123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-123)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -123);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-9223372036854775808)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+9223372036854775807)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max unsigned
        Decoder decoder{R"(+18446744073709551615)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{};

        decoder.load(R"(0123)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(00)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(+00)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(-00)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-000000009223372036854775808)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+000000009223372036854775807)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max unsigned with leading zeroes
        Decoder decoder{R"(+0000000018446744073709551615)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid minus sign
        ASSERT_TRUE(fails(R"(-)"));
        ASSERT_TRUE(fails(R"([ - ])"));
    }
    { // Invalid plus sign
        ASSERT_TRUE(fails(R"(+)"));
        ASSERT_TRUE(fails(R"([ + ])"));
    }
    { // Multiple signs
        ASSERT_TRUE(fails(R"(++0)"));
        ASSERT_TRUE(fails(R"(--0)"));
        ASSERT_TRUE(fails(R"(+-0)"));
        ASSERT_TRUE(fails(R"(-+0)"));
    }
    { // Invalid digit
        ASSERT_TRUE(fails("123A"));
    }
    { // Too big
        ASSERT_TRUE(fails("18446744073709551616"));
        ASSERT_TRUE(fails("-9223372036854775809"));
    }
}

TEST(Decode, hex)
{
    { // Zero
        Decoder decoder{R"(0x0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Lowercase
        Decoder decoder{R"(0x1a)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Uppercase
        Decoder decoder{R"(0x1A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+0x1A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-0x1A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -26);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-0x8000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+0xFFFFFFFFFFFFFFFF)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{R"(0x001A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-0x000000008000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+0x00000000FFFFFFFFFFFFFFFF)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid digit
        ASSERT_TRUE(fails(R"(0x1G)"));
        ASSERT_TRUE(fails(R"(0xG)"));
    }
    { // Uppercase X
        ASSERT_TRUE(fails(R"(0X1A)"));
    }
    { // Prefix leading zero
        ASSERT_TRUE(fails(R"(00x1A)"));
    }
    { // Decimal
        ASSERT_TRUE(fails(R"(0x1A.)"));
    }
    { // Too big
        ASSERT_TRUE(fails(R"(0x10000000000000000)"));
    }
}

TEST(Decode, octal)
{
    { // Zero
        Decoder decoder{R"(0o0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{R"(0o12)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+0o12)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-0o12)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -10);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-0o1000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+0o1777777777777777777777)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{R"(0o0012)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-0o000000001000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+0o000000001777777777777777777777)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid digit
        ASSERT_TRUE(fails(R"(0o18)"));
        ASSERT_TRUE(fails(R"(0o8)"));
    }
    { // Uppercase O
        ASSERT_TRUE(fails(R"(0O12)"));
    }
    { // Prefix leading zero
        ASSERT_TRUE(fails(R"(00x12)"));
    }
    { // Decimal
        ASSERT_TRUE(fails(R"(0x12.)"));
    }
    { // Too big
        ASSERT_TRUE(fails(R"(0o2000000000000000000000)"));
    }
}

TEST(Decode, binary)
{
    { // Zero
        Decoder decoder{R"(0b0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{R"(0b101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+0b101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-0b101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -5);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-0b1000000000000000000000000000000000000000000000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+0b1111111111111111111111111111111111111111111111111111111111111111)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{R"(0b00101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-0b000000001000000000000000000000000000000000000000000000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+0b000000001111111111111111111111111111111111111111111111111111111111111111)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid digit
        ASSERT_TRUE(fails(R"(0b121)"));
        ASSERT_TRUE(fails(R"(0b2)"));
    }
    { // Uppercase B
        ASSERT_TRUE(fails(R"(0B101)"));
    }
    { // Prefix leading zero
        ASSERT_TRUE(fails(R"(00b101)"));
    }
    { // Decimal
        ASSERT_TRUE(fails(R"(0b101.)"));
    }
    { // Too big
        ASSERT_TRUE(fails(R"(0b10000000000000000000000000000000000000000000000000000000000000000)"));
    }
}

TEST(Decode, floater)
{
    { // Zero
        Decoder decoder{R"(0.0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive zero
        Decoder decoder{R"(+0.0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative Zero
        Decoder decoder{R"(-0.0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{R"(123.456)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+123.456)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-123.456)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent lowercase
        Decoder decoder{R"(123.456e17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent uppercase
        Decoder decoder{R"(123.456E17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive exponent
        Decoder decoder{R"(123.456e+17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative exponent
        Decoder decoder{R"(-123.456e-17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456e-17);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent without fraction
        Decoder decoder{R"(123e34)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.0e34);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max integer
        Decoder decoder{R"(9007199254740991.0e0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 9007199254740991.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading and trailing decimal
        ASSERT_TRUE(fails(R"(.0)"));
        ASSERT_TRUE(fails(R"(+.0)"));
        ASSERT_TRUE(fails(R"(-.0)"));
        ASSERT_TRUE(fails(R"(0.)"));
        ASSERT_TRUE(fails(R"(+0.)"));
        ASSERT_TRUE(fails(R"(-0.)"));
        ASSERT_TRUE(fails(R"(1.e0)"));
        ASSERT_TRUE(fails(R"(.)"));
    }
    { // Leading zeroes
        Decoder decoder{};

        decoder.load(R"(01.2)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1.2);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(00.2)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.2);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e02)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e+02)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e+02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e-02)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e-02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e00)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e00);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Valid infinity
        Decoder decoder{};

        decoder.load(R"(inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(-inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(+inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid infinity
        ASSERT_TRUE(fails(R"(Inf)"));
        ASSERT_TRUE(fails(R"(iNf)"));
        ASSERT_TRUE(fails(R"(inF)"));
        ASSERT_TRUE(fails(R"(INF)"));
        ASSERT_TRUE(fails(R"(infi)"));
        ASSERT_TRUE(fails(R"(infin)"));
        ASSERT_TRUE(fails(R"(infini)"));
        ASSERT_TRUE(fails(R"(infinit)"));
        ASSERT_TRUE(fails(R"(infinity)"));
        ASSERT_TRUE(fails(R"(Infinity)"));
        ASSERT_TRUE(fails(R"(iNfinity)"));
        ASSERT_TRUE(fails(R"(inFinity)"));
        ASSERT_TRUE(fails(R"(infInity)"));
        ASSERT_TRUE(fails(R"(infiNity)"));
        ASSERT_TRUE(fails(R"(infinIty)"));
        ASSERT_TRUE(fails(R"(infiniTy)"));
        ASSERT_TRUE(fails(R"(infinitY)"));
        ASSERT_TRUE(fails(R"(INFINITY)"));
        ASSERT_TRUE(fails(R"(infstuff)"));
    }
    { // Valid NaN
        Decoder decoder{};

        decoder.load(R"(nan)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_TRUE(std::isnan(decoder.floater));
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid NaN
        ASSERT_TRUE(fails(R"(Nan)"));
        ASSERT_TRUE(fails(R"(nAn)"));
        ASSERT_TRUE(fails(R"(naN)"));
        ASSERT_TRUE(fails(R"(NaN)"));
        ASSERT_TRUE(fails(R"(NAN)"));
        ASSERT_TRUE(fails(R"(nanstuff)"));
    }
    { // Exponent decimal point
        ASSERT_TRUE(fails(R"(1.0e1.0)"));
        ASSERT_TRUE(fails(R"(1.0e1.)"));
        ASSERT_TRUE(fails(R"(1e1.0)"));
        ASSERT_TRUE(fails(R"(1e1.)"));
        ASSERT_TRUE(fails(R"(1e.1)"));
    }
    { // Dangling exponent
        ASSERT_TRUE(fails(R"(0e)"));
        ASSERT_TRUE(fails(R"(0e+)"));
        ASSERT_TRUE(fails(R"(0e-)"));
    }
    { // Magnitude too large
        ASSERT_TRUE(fails(R"(1e1000)"));
    }
    { // Magnitude too small
        ASSERT_TRUE(fails(R"(1e-1000)"));
    }
}

TEST(Decode, boolean)
{
    { // True
        Decoder decoder{R"(true)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, true);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // False
        Decoder decoder{R"(false)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, false);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
}

TEST(Decode, null)
{
    Decoder decoder{R"(null)"sv};
    ASSERT_EQ(decoder.step(), DecodeState::null);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}

TEST(Decode, date)
{
    { // General
        Decoder decoder{"D2023-02-16"};
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.year, 2023u);
        ASSERT_EQ(decoder.date.month, 2u);
        ASSERT_EQ(decoder.date.day, 16u);
    }
    { // Min
        Decoder decoder{"D0000-01-01"};
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.year, 0000u);
        ASSERT_EQ(decoder.date.month, 1u);
        ASSERT_EQ(decoder.date.day, 1u);
    }
    { // Max
        Decoder decoder{"D9999-12-31"};
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.year, 9999u);
        ASSERT_EQ(decoder.date.month, 12u);
        ASSERT_EQ(decoder.date.day, 31u);
    }
    { // Max month days
        Decoder decoder{};

        decoder.load("D1970-01-31");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 31u);

        decoder.load("D1970-02-28");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 28u);

        decoder.load("D1970-03-31");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 31u);

        decoder.load("D1970-04-30");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 30u);

        decoder.load("D1970-05-31");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 31u);

        decoder.load("D1970-06-30");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 30u);

        decoder.load("D1970-07-31");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 31u);

        decoder.load("D1970-08-31");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 31u);

        decoder.load("D1970-09-30");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 30u);

        decoder.load("D1970-10-31");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 31u);

        decoder.load("D1970-11-30");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 30u);

        decoder.load("D1970-12-31");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.day, 31u);
    }
    { // Leap
        Decoder decoder{"D2024-02-29"};
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.year, 2024u);
        ASSERT_EQ(decoder.date.month, 2u);
        ASSERT_EQ(decoder.date.day, 29u);
    }
    { // Not leap
        ASSERT_TRUE(fails("D2023-02-29"));
    }
    { // Invalid year
        ASSERT_TRUE(fails("D-1970-01-01"));
        ASSERT_TRUE(fails("D197X-01-01"));
        ASSERT_TRUE(fails("D19700-01-01"));
        ASSERT_TRUE(fails("D197-01-01"));
    }
    { // Invalid month
        ASSERT_TRUE(fails("D1970-00-01"));
        ASSERT_TRUE(fails("D1970-13-01"));
        ASSERT_TRUE(fails("D1970-1X-01"));
        ASSERT_TRUE(fails("D1970-001-01"));
        ASSERT_TRUE(fails("D1970-1-01"));
    }
    { // Invalid day
        ASSERT_TRUE(fails("D1970-01-00"));
        ASSERT_TRUE(fails("D1970-01-32"));
        ASSERT_TRUE(fails("D1970-01-0X"));
        ASSERT_TRUE(fails("D1970-01-001"));
        ASSERT_TRUE(fails("D1970-01-1"));

        ASSERT_TRUE(fails("D1970-01-32"));
        ASSERT_TRUE(fails("D1970-02-29"));
        ASSERT_TRUE(fails("D1970-03-32"));
        ASSERT_TRUE(fails("D1970-04-31"));
        ASSERT_TRUE(fails("D1970-05-32"));
        ASSERT_TRUE(fails("D1970-06-31"));
        ASSERT_TRUE(fails("D1970-07-32"));
        ASSERT_TRUE(fails("D1970-08-32"));
        ASSERT_TRUE(fails("D1970-09-31"));
        ASSERT_TRUE(fails("D1970-10-32"));
        ASSERT_TRUE(fails("D1970-11-31"));
        ASSERT_TRUE(fails("D1970-12-32"));
    }
    { // Invalid misc
        ASSERT_TRUE(fails("1970-01-01"));
        ASSERT_TRUE(fails("d1970-01-01"));
        ASSERT_TRUE(fails("D1970/01/01"));
        ASSERT_TRUE(fails("D197001-01"));
        ASSERT_TRUE(fails("D1970-0101"));
        ASSERT_TRUE(fails("D19700101"));
    }
}

TEST(Decode, time)
{
    { // General
        Decoder decoder{"T18:36:09"};
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 0u);
        ASSERT_EQ(decoder.time.zone.format, qcon::localTime);
        ASSERT_EQ(decoder.time.zone.offset, 0);
    }
    { // Subseconds
        Decoder decoder{"T18:36:09.123456789"};
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 123456789u);
        ASSERT_EQ(decoder.time.zone.format, qcon::localTime);
        ASSERT_EQ(decoder.time.zone.offset, 0);
    }
    { // Timezone
        Decoder decoder{};

        decoder.load("T18:36:09Z");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 0u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utc);
        ASSERT_EQ(decoder.time.zone.offset, 0);

        decoder.load("T18:36:09+12:34");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 0u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, 12 * 60 + 34);

        decoder.load("T18:36:09-12:34");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 0u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, -(12 * 60 + 34));
    }
    { // Timezone and subseconds
        Decoder decoder{};

        decoder.load("T18:36:09.123456789Z");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 123456789u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utc);
        ASSERT_EQ(decoder.time.zone.offset, 0);

        decoder.load("T18:36:09.123456789+12:34");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 123456789u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, 12 * 60 + 34);

        decoder.load("T18:36:09.123456789-12:34");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 123456789u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, -(12 * 60 + 34));
    }
    { // Min
        Decoder decoder{"T00:00:00-99:59"};
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 0u);
        ASSERT_EQ(decoder.time.minute, 0u);
        ASSERT_EQ(decoder.time.second, 0u);
        ASSERT_EQ(decoder.time.subsecond, 0u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, -(99 * 60 + 59));
    }
    { // Max
        Decoder decoder{"T23:59:59.999999999+99:59"};
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 23u);
        ASSERT_EQ(decoder.time.minute, 59u);
        ASSERT_EQ(decoder.time.second, 59u);
        ASSERT_EQ(decoder.time.subsecond, 999999999u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, 99 * 60 + 59);
    }
    { // Subsecond digits
        Decoder decoder{};

        decoder.load("T00:00:00.1");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 100'000'000u);

        decoder.load("T00:00:00.01");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 10'000'000u);

        decoder.load("T00:00:00.001");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 1'000'000);

        decoder.load("T00:00:00.0001");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 100'000u);

        decoder.load("T00:00:00.00001");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 10'000u);

        decoder.load("T00:00:00.000001");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 1'000u);

        decoder.load("T00:00:00.0000001");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 100u);

        decoder.load("T00:00:00.00000001");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 10u);

        decoder.load("T00:00:00.000000001");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 1u);

        decoder.load("T00:00:00.0000000005");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 1u);

        decoder.load("T00:00:00.0000000004");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 0u);
    }
    { // Zero timezone
        Decoder decoder{};

        decoder.load("T00:00:00+00:00");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, 0);

        decoder.load("T00:00:00-00:00");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, 0);
    }
    { // Subsecond clear
        Decoder decoder{};

        decoder.load("[ T00:00:00.000000123, T00:00:00 ]");
        decoder.step();
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 123u);
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.subsecond, 0u);
    }
    { // Invalid hour
        ASSERT_TRUE(fails("T60:00:00Z"));
        ASSERT_TRUE(fails("T0X:00:00Z"));
        ASSERT_TRUE(fails("T0:00:00Z"));
        ASSERT_TRUE(fails("T000:00:00Z"));
    }
    { // Invalid minute
        ASSERT_TRUE(fails("T00:60:00Z"));
        ASSERT_TRUE(fails("T00:0X:00Z"));
        ASSERT_TRUE(fails("T00:0:00Z"));
        ASSERT_TRUE(fails("T00:000:00Z"));
    }
    { // Invalid second
        ASSERT_TRUE(fails("T00:00:60Z"));
        ASSERT_TRUE(fails("T00:00:0XZ"));
        ASSERT_TRUE(fails("T00:00:0Z"));
        ASSERT_TRUE(fails("T00:00:000Z"));
    }
    { // Invalid subseconds
        ASSERT_TRUE(fails("T00:00:00.Z"));
        ASSERT_TRUE(fails("T00:00:00..Z"));
        ASSERT_TRUE(fails("T00:00:00.00XZ"));
        ASSERT_TRUE(fails("T00:00:00,0Z"));
    }
    { // Invalid timezone
        ASSERT_TRUE(fails("T00:00:00Y"));
        ASSERT_TRUE(fails("T00:00:00z"));
        ASSERT_TRUE(fails("T00:00:0000"));
        ASSERT_TRUE(fails("T00:00:00+1"));
        ASSERT_TRUE(fails("T00:00:00+11"));
        ASSERT_TRUE(fails("T00:00:00+111"));
        ASSERT_TRUE(fails("T00:00:00+11111"));
        ASSERT_TRUE(fails("T00:00:00+0X:00"));
        ASSERT_TRUE(fails("T00:00:00+00:0X"));
        ASSERT_TRUE(fails("T00:00:00+0:00"));
        ASSERT_TRUE(fails("T00:00:00+00:0"));
        ASSERT_TRUE(fails("T00:00:00+000:00"));
        ASSERT_TRUE(fails("T00:00:00+00:000"));
        ASSERT_TRUE(fails("T00:00:00+00-00"));
    }
    { // Invalid misc
        ASSERT_TRUE(fails("00:00:00Z"));
        ASSERT_TRUE(fails("t00:00:00Z"));
        ASSERT_TRUE(fails("T00-00-00Z"));
        ASSERT_TRUE(fails("T0000:00Z"));
        ASSERT_TRUE(fails("T00:0000Z"));
        ASSERT_TRUE(fails("T000000Z"));
    }
}

TEST(Decode, datetime)
{
    { // Epoch
        Decoder decoder{};

        decoder.load("D1969-12-31T16:00:00-08:00");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{});

        decoder.load("D1970-01-01T00:00:00Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{});

        decoder.load("D1969-12-31T16:00:00");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{});
    }
    { // Positive timestamp
        const Timepoint tp{std::chrono::seconds{1676337198} + std::chrono::microseconds{123456}};
        Decoder decoder{};

        decoder.load("D2023-02-13T17:13:18.123456-08:00");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.date.year, 2023u);
        ASSERT_EQ(decoder.date.month, 2u);
        ASSERT_EQ(decoder.date.day, 13u);
        ASSERT_EQ(decoder.time.hour, 17u);
        ASSERT_EQ(decoder.time.minute, 13u);
        ASSERT_EQ(decoder.time.second, 18u);
        ASSERT_EQ(decoder.time.subsecond, 123456000u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.time.zone.offset, -480);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);

        decoder.load("D2023-02-14T01:13:18.123456Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.date.year, 2023u);
        ASSERT_EQ(decoder.date.month, 2u);
        ASSERT_EQ(decoder.date.day, 14u);
        ASSERT_EQ(decoder.time.hour, 1u);
        ASSERT_EQ(decoder.time.minute, 13u);
        ASSERT_EQ(decoder.time.second, 18u);
        ASSERT_EQ(decoder.time.subsecond, 123456000u);
        ASSERT_EQ(decoder.time.zone.format, qcon::utc);
        ASSERT_EQ(decoder.time.zone.offset, 0);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);

        decoder.load("D2023-02-13T17:13:18.123456");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.date.year, 2023u);
        ASSERT_EQ(decoder.date.month, 2u);
        ASSERT_EQ(decoder.date.day, 13u);
        ASSERT_EQ(decoder.time.hour, 17u);
        ASSERT_EQ(decoder.time.minute, 13u);
        ASSERT_EQ(decoder.time.second, 18u);
        ASSERT_EQ(decoder.time.subsecond, 123456000u);
        ASSERT_EQ(decoder.time.zone.format, qcon::localTime);
        ASSERT_EQ(decoder.time.zone.offset, 0);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);
    }
    { // Negative timestamp
        const Timepoint tp{std::chrono::seconds{-777777777} + std::chrono::microseconds{142536}};
        Decoder decoder{};

        decoder.load("D1945-05-09T15:37:03.142536-07:00");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);

        decoder.load("D1945-05-09T22:37:03.142536Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);

        decoder.load("D1945-05-09T15:37:03.142536");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);
    }
    { // Future timestamp
        const Timepoint tp{std::chrono::seconds{253402300799}};
        Decoder decoder{"D9999-12-31T23:59:59Z"};
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);
    }
    { // Past timestamp
        const Timepoint tp{std::chrono::seconds{-62167219200}};
        Decoder decoder{"D0000-01-01T00:00:00Z"};
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);
    }
    { // Subseconds
        static_assert(std::chrono::system_clock::duration::period::den == 10'000'000);

        Decoder decoder{};

        decoder.load("D1970-01-01T00:00:00.1Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{1'000'000}});

        decoder.load("D1970-01-01T00:00:00.01Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{100'000}});

        decoder.load("D1970-01-01T00:00:00.001Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{10'000}});

        decoder.load("D1970-01-01T00:00:00.0001Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{1'000}});

        decoder.load("D1970-01-01T00:00:00.00001Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{100}});

        decoder.load("D1970-01-01T00:00:00.000001Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{10}});

        decoder.load("D1970-01-01T00:00:00.0000001Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{1}});

        decoder.load("D1970-01-01T00:00:00.00000006Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::system_clock::duration{1}});

        decoder.load("D1970-01-01T00:00:00.00000004Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{});
    }
    { // Zero timezone
        Decoder decoder{};

        decoder.load("D1970-01-01T00:00:00+00:00");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{});

        decoder.load("D1970-01-01T00:00:00-00:00");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{});
    }
    { // Timezone minutes
        Decoder decoder{};

        decoder.load("D1970-01-01T00:00:00+12:34");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::hours{-12} + std::chrono::minutes{-34}});

        decoder.load("D1970-01-01T00:00:00-12:34");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::hours{12} + std::chrono::minutes{34}});
    }
    { // Invalid combo
        ASSERT_TRUE(fails("D1970-01-0100:00:00Z"));
        ASSERT_TRUE(fails("D19700101T000000Z"));
    }
}

TEST(Decode, noSpace)
{
    Decoder decoder{R"({"a":["abc",-123,-123.456e-78,true,null]})"sv};
    ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.key, "a"sv);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, "abc"sv);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, -123);
            ASSERT_EQ(decoder.step(), DecodeState::floater);
            ASSERT_EQ(decoder.floater, -123.456e-78);
            ASSERT_EQ(decoder.step(), DecodeState::boolean);
            ASSERT_EQ(decoder.boolean, true);
            ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}

TEST(Decode, extraneousSpace)
{
    Decoder decoder{" \t\n\r\v{} \t\n\r\v"sv};
    ASSERT_EQ(decoder.step(), DecodeState::object);
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}

TEST(Decode, trailingComma)
{
    { // Valid
        Decoder decoder{};

        decoder.load(R"([0,])"sv);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"([0, ])"sv);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"([0 ,])"sv);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"({"k":0,})"sv);
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.key, "k"sv);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"({"k":0, })"sv);
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.key, "k"sv);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"({"k":0 ,})"sv);
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.key, "k"sv);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid
        ASSERT_TRUE(fails(R"(,)"sv));
        ASSERT_TRUE(fails(R"( ,)"sv));
        ASSERT_TRUE(fails(R"(, )"sv));
        ASSERT_TRUE(fails(R"(0,)"sv));
        ASSERT_TRUE(fails(R"(0 ,)"sv));
        ASSERT_TRUE(fails(R"(0, )"sv));
        ASSERT_TRUE(fails(R"({},)"sv));
        ASSERT_TRUE(fails(R"([],)"sv));
        ASSERT_TRUE(fails(R"([0,,])"sv));
        ASSERT_TRUE(fails(R"([0 ,,])"sv));
        ASSERT_TRUE(fails(R"([0, ,])"sv));
        ASSERT_TRUE(fails(R"([0,, ])"sv));
        ASSERT_TRUE(fails(R"({"k":0,,})"sv));
        ASSERT_TRUE(fails(R"({"k":0 ,,})"sv));
        ASSERT_TRUE(fails(R"({"k":0, ,})"sv));
        ASSERT_TRUE(fails(R"({"k":0,, })"sv));
    }
}

TEST(Decode, comments)
{
    { // Single comment
        Decoder decoder{};

        decoder.load(R"(0 # AAAAA)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(0 # AAAAA # BBBBB 1)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Multiple comments
        Decoder decoder{"# AAAAA\n#  BBBBB \n #CCCCC\n\n# DD DD\n0"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Comment in string
        Decoder decoder{R"("# AAAAA")"sv};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "# AAAAA"sv);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Comments in array
        Decoder decoder{
R"([ # AAAAA
    0, # BBBBB
    1 # CCCCC
] # DDDDD)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 0);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Comments in object
        Decoder decoder{
R"({ # AAAAA
    "0": 0, # BBBBB
    "1": 1 # CCCCC
} # DDDDD)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::object);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.key, "0"sv);
            ASSERT_EQ(decoder.integer, 0);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.key, "1"sv);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // CRLF
        Decoder decoder{};

        decoder.load("[ # AAAAA\r\n    0, # BBBBB\n    1 # CCCCC\r\n] # DDDDD \n"sv);
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 0);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("[ # AAAAA\r 0, # BBBBB\n    1 # CCCCC\r\n] # DDDDD \n"sv);
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Weirdness
        Decoder decoder{};

        decoder.load("#\n0"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("# \n0"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("##\n0"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("# #\n0"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("#\n#\n0"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0#"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0# "sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0#\n"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0#\n#"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Nothing but comments
        ASSERT_TRUE(fails("# AAAAA\n# CCCCC\n"sv));
    }
}

TEST(Decode, depth)
{
    { // 64 nested objects
        Decoder decoder{R"({"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"v":true}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}})"};
        for (int i{0}; i < 64; ++i) ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.key, "v");
        ASSERT_EQ(decoder.boolean, true);
        for (int i{0}; i < 64; ++i) ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // 64 nested arrays
        Decoder decoder{"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[true]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"};
        for (int i{0}; i < 64; ++i) ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, true);
        for (int i{0}; i < 64; ++i) ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // 65 nested objects
        ASSERT_TRUE(fails(R"({"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"v":true}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}})"));
    }
    { // 65 nested arrays
        ASSERT_TRUE(fails("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[true]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"));
    }
}

TEST(Decode, misc)
{
    { // Empty
        ASSERT_TRUE(fails(R"()"sv));
    }
    { // Only Space
        ASSERT_TRUE(fails(R"(   )"sv));
    }
    { // Unknown value
        ASSERT_TRUE(fails(R"(v)"sv));
    }
    { // Multiple root values
        ASSERT_TRUE(fails(R"(1 2)"sv));
        ASSERT_TRUE(fails(R"(1, 2)"sv));
    }
    { // Lone decimal
        ASSERT_TRUE(fails(R"(.)"sv));
    }
    { // Stepping on error
        Decoder decoder{"nullnull"};
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::error);
        ASSERT_EQ(decoder.step(), DecodeState::error);
    }
    { // Stepping once done
        Decoder decoder{"null"};
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::done);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
}

TEST(Decode, general)
{
    const std::string_view qcon{
R"(
# Third quarter summary document
# Protected information, do not propagate!
{
    "Name": "Salt's Crust",
    "Founded": D1964-03-17,
    "Opens": T08:30:00,
    # Not necessarily up to date
    "Employees": [
        { "Name": "Ol' Joe Fisher", "Title": "Fisherman", "Age": 69 },
        { "Name": "Mark Rower", "Title": "Cook", "Age": 41 },
        { "Name": "Phineas", "Title": "Server Boy", "Age": 19 },
    ],
    "Dishes": [
        {
            "Name": "Basket o' Barnacles",
            "Price": 5.45,
            "Ingredients": [ "\"Salt\"", "Barnacles" ],
            "Gluten Free": false
        },
        {
            "Name": "Two Tuna",
            "Price": -inf,
            "Ingredients": [ "Tuna" ], # It's actually cod lmao
            "Gluten Free": true
        },
        {
            "Name": "18 Leg Bouquet",
            "Price": nan,
            "Ingredients": [ "\"Salt\"", "Octopus", "Crab", ],
            "Gluten Free": false
        }
    ],
    "Profit Margin": null, # Pay no heed
    "Ha\x03r Name": "M\u0000\0n",
    "Green Eggs and Ham":
"\
I do not like them in a box\n\
I do not like them with a fox\n\
I do not like them in a house\n\
I do not like them with a mouse\n\
I do not like them here or there\n\
I do not like them anywhere\n\
I do not like green eggs and ham\n\
I do not like them Sam I am\n\
",
    "Magic Numbers": [0x309,0o1411,0b1100001001], # What could they mean?!
    "Last Updated": D2003-06-28T13:59:11.067Z
})"sv};
    Decoder decoder{qcon};
    ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.key, "Name"sv);
        ASSERT_EQ(decoder.string, "Salt's Crust"sv);
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.key, "Founded"sv);
        ASSERT_EQ(decoder.date.year, 1964u);
        ASSERT_EQ(decoder.date.month, 3u);
        ASSERT_EQ(decoder.date.day, 17u);
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.key, "Opens"sv);
        ASSERT_EQ(decoder.time.hour, 8u);
        ASSERT_EQ(decoder.time.minute, 30u);
        ASSERT_EQ(decoder.time.second, 0u);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.key, "Employees"sv);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name"sv);
                ASSERT_EQ(decoder.string, "Ol' Joe Fisher"sv);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Title"sv);
                ASSERT_EQ(decoder.string, "Fisherman"sv);
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.key, "Age"sv);
                ASSERT_EQ(decoder.integer, 69);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name"sv);
                ASSERT_EQ(decoder.string, "Mark Rower"sv);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Title"sv);
                ASSERT_EQ(decoder.string, "Cook"sv);
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.key, "Age"sv);
                ASSERT_EQ(decoder.integer, 41);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name"sv);
                ASSERT_EQ(decoder.string, "Phineas"sv);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Title"sv);
                ASSERT_EQ(decoder.string, "Server Boy"sv);
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.key, "Age"sv);
                ASSERT_EQ(decoder.integer, 19);
            ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.key, "Dishes"sv);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name"sv);
                ASSERT_EQ(decoder.string, "Basket o' Barnacles"sv);
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.key, "Price"sv);
                ASSERT_EQ(decoder.floater, 5.45);
                ASSERT_EQ(decoder.step(), DecodeState::array);
                ASSERT_EQ(decoder.key, "Ingredients"sv);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "\"Salt\""sv);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Barnacles"sv);
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.key, "Gluten Free"sv);
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name"sv);
                ASSERT_EQ(decoder.string, "Two Tuna"sv);
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.key, "Price"sv);
                ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
                ASSERT_EQ(decoder.step(), DecodeState::array);
                ASSERT_EQ(decoder.key, "Ingredients"sv);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Tuna"sv);
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.key, "Gluten Free"sv);
                ASSERT_EQ(decoder.boolean, true);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name"sv);
                ASSERT_EQ(decoder.string, "18 Leg Bouquet"sv);
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.key, "Price"sv);
                ASSERT_TRUE(std::isnan(decoder.floater));
                ASSERT_EQ(decoder.step(), DecodeState::array);
                ASSERT_EQ(decoder.key, "Ingredients"sv);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "\"Salt\""sv);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Octopus"sv);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Crab"sv);
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.key, "Gluten Free"sv);
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "Profit Margin"sv);
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.key, "Ha\x03r Name"sv);
        ASSERT_EQ(decoder.string, "M\0\0n"sv);
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.key, "Green Eggs and Ham"sv);
        ASSERT_EQ(decoder.string,
R"(I do not like them in a box
I do not like them with a fox
I do not like them in a house
I do not like them with a mouse
I do not like them here or there
I do not like them anywhere
I do not like green eggs and ham
I do not like them Sam I am
)");
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.key, "Magic Numbers"sv);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.key, "Last Updated"sv);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::seconds{1056808751} + std::chrono::milliseconds{67}});
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}
