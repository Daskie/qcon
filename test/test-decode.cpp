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
        case DecodeState::date: os << "date"; break;
        case DecodeState::time: os << "time"; break;
        case DecodeState::datetime: os << "datetime"; break;
        case DecodeState::null: os << "null"; break;
        case DecodeState::done: os << "done"; break;
    }

    return os;
}

bool fails(const char * str)
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
        Decoder decoder{"{}"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Single key
        Decoder decoder{R"({ "a": null })"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Multiple keys
        Decoder decoder{R"({ "a": null, "b": null, "c": null })"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "b");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "c");
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No space
        Decoder decoder{R"({"a":null,"b":null})"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "b");
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Weird spacing
        Decoder decoder{R"({"a" :null ,"b" :null})"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "b");
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Empty key
        Decoder decoder{R"({ "": null })"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "");
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
        Decoder decoder{"[]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Single element
        Decoder decoder{"[ null ]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Multiple elements
        Decoder decoder{"[ null, null, null ]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No space
        Decoder decoder{"[null,null]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Weird spacing
        Decoder decoder{"[null ,null]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No comma between elements
        ASSERT_TRUE(fails("[ 0 1 ]"));
    }
    { // Empty entry
        ASSERT_TRUE(fails("[ 0, , 1 ]"));
    }
    { // Cut off
        ASSERT_TRUE(fails("["));
        ASSERT_TRUE(fails("[0"));
        ASSERT_TRUE(fails("[0,"));
        ASSERT_TRUE(fails("[0,1"));
        ASSERT_TRUE(fails("0,1]"));
        ASSERT_TRUE(fails(",1]"));
        ASSERT_TRUE(fails("1]"));
        ASSERT_TRUE(fails("]"));
    }
}

TEST(Decode, string)
{
    { // Empty string
        Decoder decoder{R"("")"};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "");
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // All ASCII
        Decoder decoder{"\"\\0\\x01\\x02\\x03\\x04\\x05\\x06\\a\\b\\t\\n\\v\\f\\r\\x0E\\x0F\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1A\\x1B\\x1C\\x1D\\x1E\\x1F !\\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F\""};
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
        ASSERT_TRUE(fails("\"\\\""));
        ASSERT_TRUE(fails("[ \"\\\" ]"));
    }
    { // Unknown escape sequence
        ASSERT_TRUE(fails("\"\\\0\""));
    }
    { // 'x' code point
        Decoder decoder{};

        for (unat i{0u}; i < 128u; ++i)
        {
            const std::string expectedStr{char(i)};
            const std::string decodeStr{std::format("\"\\x{:02X}\"", i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
        for (unat i{128u}; i < 256u; ++i)
        {
            const std::string expectedStr{char(u8(0b110'00000u | (i >> 6))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\x{:02X}\"", i)};

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
            const std::string decodeStr{std::format("\"\\u{:04X}\"", i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
        for (unat i{128u}; i < 2048u; ++i)
        {
            const std::string expectedStr{char(u8(0b110'00000u | (i >> 6))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\u{:04X}\"", i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_EQ(decoder.step(), DecodeState::done);
        }
        for (unat i{2048u}; i < 65536u; ++i)
        {
            const std::string expectedStr{char(u8(0b1110'0000u | (i >> 12))), char(u8(0b10'000000u | ((i >> 6) & 0b111111u))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\u{:04X}\"", i)};

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
    { // Newlines
        Decoder decoder{};

        ASSERT_TRUE(fails("\"a\nb\""));
        ASSERT_TRUE(fails("\"a\rb\""));
        ASSERT_TRUE(fails("\"a\r\nb\""));

        ASSERT_TRUE(fails("\"a\\\nb\""));
        ASSERT_TRUE(fails("\"a\\\rb\""));
        ASSERT_TRUE(fails("\"a\\\r\nb\""));
    }
    { // Single quotes
        ASSERT_TRUE(fails("'abc'"));
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
        Decoder decoder{"0"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Normal
        Decoder decoder{"123"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{"+123"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer,123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{"-123"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -123);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{"-9223372036854775808"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{"+9223372036854775807"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max unsigned
        Decoder decoder{"+18446744073709551615"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{};

        decoder.load("0123");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("00");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("+00");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("-00");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{"-000000009223372036854775808"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{"+000000009223372036854775807"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max unsigned with leading zeroes
        Decoder decoder{"+0000000018446744073709551615"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid minus sign
        ASSERT_TRUE(fails("-"));
        ASSERT_TRUE(fails("[ - ]"));
    }
    { // Invalid plus sign
        ASSERT_TRUE(fails("+"));
        ASSERT_TRUE(fails("[ + ]"));
    }
    { // Multiple signs
        ASSERT_TRUE(fails("++0"));
        ASSERT_TRUE(fails("--0"));
        ASSERT_TRUE(fails("+-0"));
        ASSERT_TRUE(fails("-+0"));
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
        Decoder decoder{"0x0"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Lowercase
        Decoder decoder{"0x1a"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Uppercase
        Decoder decoder{"0x1A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{"+0x1A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{"-0x1A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -26);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{"-0x8000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{"+0xFFFFFFFFFFFFFFFF"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{"0x001A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{"-0x000000008000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{"+0x00000000FFFFFFFFFFFFFFFF"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid digit
        ASSERT_TRUE(fails("0x1G"));
        ASSERT_TRUE(fails("0xG"));
    }
    { // Uppercase X
        ASSERT_TRUE(fails("0X1A"));
    }
    { // Prefix leading zero
        ASSERT_TRUE(fails("00x1A"));
    }
    { // Decimal
        ASSERT_TRUE(fails("0x1A."));
    }
    { // Too big
        ASSERT_TRUE(fails("0x10000000000000000"));
    }
}

TEST(Decode, octal)
{
    { // Zero
        Decoder decoder{"0o0"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{"0o12"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{"+0o12"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{"-0o12"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -10);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{"-0o1000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{"+0o1777777777777777777777"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{"0o0012"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{"-0o000000001000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{"+0o000000001777777777777777777777"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid digit
        ASSERT_TRUE(fails("0o18"));
        ASSERT_TRUE(fails("0o8"));
    }
    { // Uppercase O
        ASSERT_TRUE(fails("0O12"));
    }
    { // Prefix leading zero
        ASSERT_TRUE(fails("00x12"));
    }
    { // Decimal
        ASSERT_TRUE(fails("0x12."));
    }
    { // Too big
        ASSERT_TRUE(fails("0o2000000000000000000000"));
    }
}

TEST(Decode, binary)
{
    { // Zero
        Decoder decoder{"0b0"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{"0b101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{"+0b101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{"-0b101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -5);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{"-0b1000000000000000000000000000000000000000000000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{"+0b1111111111111111111111111111111111111111111111111111111111111111"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{"0b00101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{"-0b000000001000000000000000000000000000000000000000000000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{"+0b000000001111111111111111111111111111111111111111111111111111111111111111"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid digit
        ASSERT_TRUE(fails("0b121"));
        ASSERT_TRUE(fails("0b2"));
    }
    { // Uppercase B
        ASSERT_TRUE(fails("0B101"));
    }
    { // Prefix leading zero
        ASSERT_TRUE(fails("00b101"));
    }
    { // Decimal
        ASSERT_TRUE(fails("0b101."));
    }
    { // Too big
        ASSERT_TRUE(fails("0b10000000000000000000000000000000000000000000000000000000000000000"));
    }
}

TEST(Decode, floater)
{
    { // Zero
        Decoder decoder{"0.0"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive zero
        Decoder decoder{"+0.0"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative Zero
        Decoder decoder{"-0.0"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{"123.456"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{"+123.456"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{"-123.456"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent lowercase
        Decoder decoder{"123.456e17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent uppercase
        Decoder decoder{"123.456E17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive exponent
        Decoder decoder{"123.456e+17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative exponent
        Decoder decoder{"-123.456e-17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456e-17);
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent without fraction
        Decoder decoder{"123e34"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.0e34);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max integer
        Decoder decoder{"9007199254740991.0e0"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 9007199254740991.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading and trailing decimal
        ASSERT_TRUE(fails(".0"));
        ASSERT_TRUE(fails("+.0"));
        ASSERT_TRUE(fails("-.0"));
        ASSERT_TRUE(fails("0."));
        ASSERT_TRUE(fails("+0."));
        ASSERT_TRUE(fails("-0."));
        ASSERT_TRUE(fails("1.e0"));
        ASSERT_TRUE(fails("."));
    }
    { // Leading zeroes
        Decoder decoder{};

        decoder.load("01.2");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1.2);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("00.2");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.2);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("1e02");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("1e+02");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e+02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("1e-02");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e-02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("1e00");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e00);
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Valid infinity
        Decoder decoder{};

        decoder.load("inf");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("-inf");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
        ASSERT_FALSE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("+inf");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.positive);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid infinity
        ASSERT_TRUE(fails("Inf"));
        ASSERT_TRUE(fails("iNf"));
        ASSERT_TRUE(fails("inF"));
        ASSERT_TRUE(fails("INF"));
        ASSERT_TRUE(fails("infi"));
        ASSERT_TRUE(fails("infin"));
        ASSERT_TRUE(fails("infini"));
        ASSERT_TRUE(fails("infinit"));
        ASSERT_TRUE(fails("infinity"));
        ASSERT_TRUE(fails("Infinity"));
        ASSERT_TRUE(fails("iNfinity"));
        ASSERT_TRUE(fails("inFinity"));
        ASSERT_TRUE(fails("infInity"));
        ASSERT_TRUE(fails("infiNity"));
        ASSERT_TRUE(fails("infinIty"));
        ASSERT_TRUE(fails("infiniTy"));
        ASSERT_TRUE(fails("infinitY"));
        ASSERT_TRUE(fails("INFINITY"));
        ASSERT_TRUE(fails("infstuff"));
    }
    { // Valid NaN
        Decoder decoder{};

        decoder.load("nan");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_TRUE(std::isnan(decoder.floater));
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid NaN
        ASSERT_TRUE(fails("Nan"));
        ASSERT_TRUE(fails("nAn"));
        ASSERT_TRUE(fails("naN"));
        ASSERT_TRUE(fails("NaN"));
        ASSERT_TRUE(fails("NAN"));
        ASSERT_TRUE(fails("nanstuff"));
    }
    { // Exponent decimal point
        ASSERT_TRUE(fails("1.0e1.0"));
        ASSERT_TRUE(fails("1.0e1."));
        ASSERT_TRUE(fails("1e1.0"));
        ASSERT_TRUE(fails("1e1."));
        ASSERT_TRUE(fails("1e.1"));
    }
    { // Dangling exponent
        ASSERT_TRUE(fails("0e"));
        ASSERT_TRUE(fails("0e+"));
        ASSERT_TRUE(fails("0e-"));
    }
    { // Magnitude too large
        ASSERT_TRUE(fails("1e1000"));
    }
    { // Magnitude too small
        ASSERT_TRUE(fails("1e-1000"));
    }
}

TEST(Decode, boolean)
{
    { // True
        Decoder decoder{"true"};
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, true);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // False
        Decoder decoder{"false"};
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, false);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
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

TEST(Decode, null)
{
    Decoder decoder{"null"};
    ASSERT_EQ(decoder.step(), DecodeState::null);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}

TEST(Decode, noSpace)
{
    Decoder decoder{R"({"a":["abc",-123,-123.456e-78,true,null]})"};
    ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.key, "a");
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, "abc");
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
    Decoder decoder{" \t\n\r\v{} \t\n\r\v"};
    ASSERT_EQ(decoder.step(), DecodeState::object);
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}

TEST(Decode, trailingComma)
{
    { // Valid
        Decoder decoder{};

        decoder.load("[0,]");
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("[0, ]");
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("[0 ,]");
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"({"k":0,})");
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.key, "k");
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"({"k":0, })");
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.key, "k");
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"({"k":0 ,})");
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.key, "k");
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid
        ASSERT_TRUE(fails(R"(,)"));
        ASSERT_TRUE(fails(R"( ,)"));
        ASSERT_TRUE(fails(R"(, )"));
        ASSERT_TRUE(fails(R"(0,)"));
        ASSERT_TRUE(fails(R"(0 ,)"));
        ASSERT_TRUE(fails(R"(0, )"));
        ASSERT_TRUE(fails(R"({},)"));
        ASSERT_TRUE(fails(R"([],)"));
        ASSERT_TRUE(fails(R"([0,,])"));
        ASSERT_TRUE(fails(R"([0 ,,])"));
        ASSERT_TRUE(fails(R"([0, ,])"));
        ASSERT_TRUE(fails(R"([0,, ])"));
        ASSERT_TRUE(fails(R"({"k":0,,})"));
        ASSERT_TRUE(fails(R"({"k":0 ,,})"));
        ASSERT_TRUE(fails(R"({"k":0, ,})"));
        ASSERT_TRUE(fails(R"({"k":0,, })"));
    }
}

TEST(Decode, comments)
{
    { // Single comment
        Decoder decoder{};

        decoder.load("0 # AAAAA");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0 # AAAAA # BBBBB 1");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Multiple comments
        Decoder decoder{"# AAAAA\n#  BBBBB \n #CCCCC\n\n# DD DD\n0"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Comment in string
        Decoder decoder{R"("# AAAAA")"};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "# AAAAA");
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Comments in array
        Decoder decoder{
R"([ # AAAAA
    0, # BBBBB
    1 # CCCCC
] # DDDDD)"};
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
} # DDDDD)"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.key, "0");
            ASSERT_EQ(decoder.integer, 0);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.key, "1");
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // CRLF
        Decoder decoder{};

        decoder.load("[ # AAAAA\r\n    0, # BBBBB\n    1 # CCCCC\r\n] # DDDDD \n");
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 0);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("[ # AAAAA\r 0, # BBBBB\n    1 # CCCCC\r\n] # DDDDD \n");
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Weirdness
        Decoder decoder{};

        decoder.load("#\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("# \n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("##\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("# #\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("#\n#\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0#");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0# ");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0#\n");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load("0#\n#");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Nothing but comments
        ASSERT_TRUE(fails("# AAAAA\n# CCCCC\n"));
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
        ASSERT_TRUE(fails(""));
    }
    { // Only Space
        ASSERT_TRUE(fails("   "));
    }
    { // Unknown value
        ASSERT_TRUE(fails("v"));
    }
    { // Multiple root values
        ASSERT_TRUE(fails("1 2"));
        ASSERT_TRUE(fails("1, 2"));
    }
    { // Lone decimal
        ASSERT_TRUE(fails("."));
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
    const char * qcon{
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
    "Green Eggs and Ham": "I do not like them in a box\nI do not like them with a fox\nI do not like them in a house\nI do not like them with a mouse\nI do not like them here or there\nI do not like them anywhere\nI do not like green eggs and ham\nI do not like them Sam I am\n",
    "Magic Numbers": [0x309,0o1411,0b1100001001], # What could they mean?!
    "Last Updated": D2003-06-28T13:59:11.067Z
})"};
    Decoder decoder{qcon};
    ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.key, "Name");
        ASSERT_EQ(decoder.string, "Salt's Crust");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.key, "Founded");
        ASSERT_EQ(decoder.date.year, 1964u);
        ASSERT_EQ(decoder.date.month, 3u);
        ASSERT_EQ(decoder.date.day, 17u);
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.key, "Opens");
        ASSERT_EQ(decoder.time.hour, 8u);
        ASSERT_EQ(decoder.time.minute, 30u);
        ASSERT_EQ(decoder.time.second, 0u);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.key, "Employees");
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.string, "Ol' Joe Fisher");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_EQ(decoder.string, "Fisherman");
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_EQ(decoder.integer, 69);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.string, "Mark Rower");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_EQ(decoder.string, "Cook");
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_EQ(decoder.integer, 41);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.string, "Phineas");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_EQ(decoder.string, "Server Boy");
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_EQ(decoder.integer, 19);
            ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.key, "Dishes");
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.string, "Basket o' Barnacles");
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_EQ(decoder.floater, 5.45);
                ASSERT_EQ(decoder.step(), DecodeState::array);
                ASSERT_EQ(decoder.key, "Ingredients");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "\"Salt\"");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Barnacles");
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.string, "Two Tuna");
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
                ASSERT_EQ(decoder.step(), DecodeState::array);
                ASSERT_EQ(decoder.key, "Ingredients");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Tuna");
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_EQ(decoder.boolean, true);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.string, "18 Leg Bouquet");
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_TRUE(std::isnan(decoder.floater));
                ASSERT_EQ(decoder.step(), DecodeState::array);
                ASSERT_EQ(decoder.key, "Ingredients");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "\"Salt\"");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Octopus");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Crab");
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.key, "Profit Margin");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.key, "Ha\x03r Name");
        ASSERT_EQ(decoder.string, "M\0\0n"sv);
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.key, "Green Eggs and Ham");
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
        ASSERT_EQ(decoder.key, "Magic Numbers");
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.key, "Last Updated");
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::seconds{1056808751} + std::chrono::milliseconds{67}});
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}
