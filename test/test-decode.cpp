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
using qcon::Date;
using qcon::Time;
using qcon::Datetime;
using qcon::Timepoint;
using enum qcon::Container;

std::ostream & operator<<(std::ostream & os, const DecodeState state)
{
    switch (state)
    {
        case DecodeState::error: os << "error"; break;
        case DecodeState::ready: os << "ready"; break;
        case DecodeState::object: os << "object"; break;
        case DecodeState::array: os << "array"; break;
        case DecodeState::end: os << "end"; break;
        case DecodeState::key: os << "key"; break;
        case DecodeState::string: os << "string"; break;
        case DecodeState::integer: os << "integer"; break;
        case DecodeState::floater: os << "floater"; break;
        case DecodeState::boolean: os << "boolean"; break;
        case DecodeState::date: os << "date"; break;
        case DecodeState::time: os << "time"; break;
        case DecodeState::datetime: os << "datetime"; break;
        case DecodeState::null: os << "null"; break;
    }

    return os;
}

bool fails(const char * str)
{
    Decoder decoder{str};
    while (!decoder.done())
    {
        decoder.step();
    }
    return !decoder;
}

TEST(Decode, object)
{
    { // Empty
        Decoder decoder{"{}"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // Single key
        Decoder decoder{R"({ "a": null })"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // Multiple keys
        Decoder decoder{R"({ "a": null, "b": null, "c": null })"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "b");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "c");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // No space
        Decoder decoder{R"({"a":null,"b":null})"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "b");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // Weird spacing
        Decoder decoder{R"({"a" :null ,"b" :null})"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "b");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // Empty key
        Decoder decoder{R"({ "": null })"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
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
    { // Only comma
        ASSERT_TRUE(fails(R"({,})"));
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
        ASSERT_TRUE(decoder);
    }
    { // Single element
        Decoder decoder{"[ null ]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // Multiple elements
        Decoder decoder{"[ null, null, null ]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // No space
        Decoder decoder{"[null,null]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // Weird spacing
        Decoder decoder{"[null ,null]"};
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // No comma between elements
        ASSERT_TRUE(fails("[ 0 1 ]"));
    }
    { // Empty entry
        ASSERT_TRUE(fails("[ 0, , 1 ]"));
    }
    { // Only comma
        ASSERT_TRUE(fails("[,]"));
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
        ASSERT_TRUE(decoder);
    }
    { // All ASCII
        Decoder decoder{"\"\\0\\x01\\x02\\x03\\x04\\x05\\x06\\a\\b\\t\\n\\v\\f\\r\\x0E\\x0F\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1A\\x1B\\x1C\\x1D\\x1E\\x1F !\\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F\""};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\0\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F"sv);
        ASSERT_TRUE(decoder);
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
    { // Escaped forward slash
        Decoder decoder{"\"\\/\""};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "/");
        ASSERT_TRUE(decoder);
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
            ASSERT_TRUE(decoder);
        }
        for (unat i{128u}; i < 256u; ++i)
        {
            const std::string expectedStr{char(u8(0b110'00000u | (i >> 6))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\x{:02X}\"", i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_TRUE(decoder);
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
            ASSERT_TRUE(decoder);
        }
        for (unat i{128u}; i < 2048u; ++i)
        {
            const std::string expectedStr{char(u8(0b110'00000u | (i >> 6))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\u{:04X}\"", i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_TRUE(decoder);
        }
        for (unat i{2048u}; i < 65536u; ++i)
        {
            const std::string expectedStr{char(u8(0b1110'0000u | (i >> 12))), char(u8(0b10'000000u | ((i >> 6) & 0b111111u))), char(u8(0b10'000000u | (i & 0b111111u)))};
            const std::string decodeStr{std::format("\"\\u{:04X}\"", i)};

            decoder.load(decodeStr);
            ASSERT_EQ(decoder.step(), DecodeState::string);
            ASSERT_EQ(decoder.string, expectedStr);
            ASSERT_TRUE(decoder);
        }
    }
    { // 'U' code point
        Decoder decoder{};

        decoder.load(R"("\U00000000")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string.size(), 1u);
        ASSERT_EQ(decoder.string.front(), '\0');
        ASSERT_TRUE(decoder);

        decoder.load(R"("\U0000007F")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\x7F");
        ASSERT_TRUE(decoder);

        decoder.load(R"("\U00000080")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xC2\x80");
        ASSERT_TRUE(decoder);

        decoder.load(R"("\U000007FF")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xDF\xBF");
        ASSERT_TRUE(decoder);

        decoder.load(R"("\U00000800")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xE0\xA0\x80");
        ASSERT_TRUE(decoder);

        decoder.load(R"("\U0000FFFF")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xEF\xBF\xBF");
        ASSERT_TRUE(decoder);

        decoder.load(R"("\U00010000")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF0\x90\x80\x80");
        ASSERT_TRUE(decoder);

        decoder.load(R"("\U001FFFFF")");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF7\xBF\xBF\xBF");
        ASSERT_TRUE(decoder);

        ASSERT_TRUE(fails("\"\\U00200000\""));
    }
    { // Uppercase and lowercase code point hex digits
        Decoder decoder{R"("\x0a\x0A\x0b\x0B\x0c\x0C\x0d\x0D")"};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\x0A\x0A\x0B\x0B\x0C\x0C\x0D\x0D");
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);

        decoder.load("\"\\x7F\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\x7F");
        ASSERT_TRUE(decoder);

        decoder.load("\"\xC2\x80\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xC2\x80");
        ASSERT_TRUE(decoder);

        decoder.load("\"\xDF\xBF\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xDF\xBF");
        ASSERT_TRUE(decoder);

        decoder.load("\"\xE0\xA0\x80\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xE0\xA0\x80");
        ASSERT_TRUE(decoder);

        decoder.load("\"\xEF\xBF\xBF\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xEF\xBF\xBF");
        ASSERT_TRUE(decoder);

        decoder.load("\"\xF0\x90\x80\x80\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF0\x90\x80\x80");
        ASSERT_TRUE(decoder);

        decoder.load("\"\xF7\xBF\xBF\xBF\"");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\xF7\xBF\xBF\xBF");
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);
    }
    { // Multi string
        Decoder decoder{"\"a\"\"b\" \"c\"\n\"d\""};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "abcd");
        ASSERT_TRUE(decoder);
    }
    { // Multi string key
        Decoder decoder{"{ \"A\"\" somewhat\"      \" rather\"\n\n\r\n\n\" long\"\t  \t\" key\": \"a\"\"b\" \"c\"\n\"d\" }"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "A somewhat rather long key");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "abcd");
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
}

TEST(Decode, decimal)
{
    { // Zero
        Decoder decoder{"0"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Normal
        Decoder decoder{"123"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Positive
        Decoder decoder{"+123"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer,123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Negative
        Decoder decoder{"-123"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -123);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min
        Decoder decoder{"-9223372036854775808"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max
        Decoder decoder{"+9223372036854775807"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max unsigned
        Decoder decoder{"+18446744073709551615"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Leading zeroes
        Decoder decoder{};

        decoder.load("0123");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("00");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("+00");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("-00");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min with leading zeroes
        Decoder decoder{"-000000009223372036854775808"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max with leading zeroes
        Decoder decoder{"+000000009223372036854775807"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max unsigned with leading zeroes
        Decoder decoder{"+0000000018446744073709551615"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);
    }
    { // Lowercase
        Decoder decoder{"0x1a"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Uppercase
        Decoder decoder{"0x1A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Positive
        Decoder decoder{"+0x1A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Negative
        Decoder decoder{"-0x1A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -26);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min
        Decoder decoder{"-0x8000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max
        Decoder decoder{"+0xFFFFFFFFFFFFFFFF"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Leading zeroes
        Decoder decoder{"0x001A"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min with leading zeroes
        Decoder decoder{"-0x000000008000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max with leading zeroes
        Decoder decoder{"+0x00000000FFFFFFFFFFFFFFFF"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);
    }
    { // No sign
        Decoder decoder{"0o12"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Positive
        Decoder decoder{"+0o12"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Negative
        Decoder decoder{"-0o12"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -10);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min
        Decoder decoder{"-0o1000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max
        Decoder decoder{"+0o1777777777777777777777"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Leading zeroes
        Decoder decoder{"0o0012"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min with leading zeroes
        Decoder decoder{"-0o000000001000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max with leading zeroes
        Decoder decoder{"+0o000000001777777777777777777777"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);
    }
    { // No sign
        Decoder decoder{"0b101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Positive
        Decoder decoder{"+0b101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Negative
        Decoder decoder{"-0b101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -5);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min
        Decoder decoder{"-0b1000000000000000000000000000000000000000000000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max
        Decoder decoder{"+0b1111111111111111111111111111111111111111111111111111111111111111"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Leading zeroes
        Decoder decoder{"0b00101"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Min with leading zeroes
        Decoder decoder{"-0b000000001000000000000000000000000000000000000000000000000000000000000000"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max with leading zeroes
        Decoder decoder{"+0b000000001111111111111111111111111111111111111111111111111111111111111111"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);
    }
    { // Positive zero
        Decoder decoder{"+0.0"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Negative Zero
        Decoder decoder{"-0.0"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // No sign
        Decoder decoder{"123.456"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Positive
        Decoder decoder{"+123.456"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Negative
        Decoder decoder{"-123.456"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Exponent lowercase
        Decoder decoder{"123.456e17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Exponent uppercase
        Decoder decoder{"123.456E17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Positive exponent
        Decoder decoder{"123.456e+17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Negative exponent
        Decoder decoder{"-123.456e-17"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456e-17);
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Exponent without fraction
        Decoder decoder{"123e34"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.0e34);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Max integer
        Decoder decoder{"9007199254740991.0e0"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 9007199254740991.0);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);

        decoder.load("00.2");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.2);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("1e02");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("1e+02");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e+02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("1e-02");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e-02);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("1e00");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e00);
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
    }
    { // Valid infinity
        Decoder decoder{};

        decoder.load("inf");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("-inf");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
        ASSERT_FALSE(decoder.positive);
        ASSERT_TRUE(decoder);

        decoder.load("+inf");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.positive);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);

        decoder.load("+nan");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_TRUE(std::isnan(decoder.floater));
        ASSERT_TRUE(decoder);

        decoder.load("-nan");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_TRUE(std::isnan(decoder.floater));
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);
    }
    { // False
        Decoder decoder{"false"};
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, false);
        ASSERT_TRUE(decoder);
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
    }
    { // Subseconds
        Decoder decoder{"T18:36:09.123456789"};
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 18u);
        ASSERT_EQ(decoder.time.minute, 36u);
        ASSERT_EQ(decoder.time.second, 9u);
        ASSERT_EQ(decoder.time.subsecond, 123456789u);
    }
    { // Min
        Decoder decoder{"T00:00:00"};
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 0u);
        ASSERT_EQ(decoder.time.minute, 0u);
        ASSERT_EQ(decoder.time.second, 0u);
        ASSERT_EQ(decoder.time.subsecond, 0u);
    }
    { // Max
        Decoder decoder{"T23:59:59.999999999"};
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 23u);
        ASSERT_EQ(decoder.time.minute, 59u);
        ASSERT_EQ(decoder.time.second, 59u);
        ASSERT_EQ(decoder.time.subsecond, 999999999u);
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
        ASSERT_TRUE(fails("T60:00:00"));
        ASSERT_TRUE(fails("T0X:00:00"));
        ASSERT_TRUE(fails("T0:00:00"));
        ASSERT_TRUE(fails("T000:00:00"));
    }
    { // Invalid minute
        ASSERT_TRUE(fails("T00:60:00"));
        ASSERT_TRUE(fails("T00:0X:00"));
        ASSERT_TRUE(fails("T00:0:00"));
        ASSERT_TRUE(fails("T00:000:00"));
    }
    { // Invalid second
        ASSERT_TRUE(fails("T00:00:60"));
        ASSERT_TRUE(fails("T00:00:0X"));
        ASSERT_TRUE(fails("T00:00:0"));
        ASSERT_TRUE(fails("T00:00:000"));
    }
    { // Invalid subseconds
        ASSERT_TRUE(fails("T00:00:00."));
        ASSERT_TRUE(fails("T00:00:00.."));
        ASSERT_TRUE(fails("T00:00:00.00X"));
        ASSERT_TRUE(fails("T00:00:00,0"));
    }
    { // Invalid misc
        ASSERT_TRUE(fails("00:00:00"));
        ASSERT_TRUE(fails("t00:00:00"));
        ASSERT_TRUE(fails("T00-00-00"));
        ASSERT_TRUE(fails("T0000:00"));
        ASSERT_TRUE(fails("T00:0000"));
        ASSERT_TRUE(fails("T000000"));
    }
    { // Has timezone
        ASSERT_TRUE(fails("T00:00:00Z"));
        ASSERT_TRUE(fails("T00:00:00+00:00"));
        ASSERT_TRUE(fails("T00:00:00-00:00"));
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
    { // Min
        Decoder decoder{"D0000-01-01T00:00:00-23:59"};
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.date.year, 0u);
        ASSERT_EQ(decoder.datetime.date.month, 1u);
        ASSERT_EQ(decoder.datetime.date.day, 1u);
        ASSERT_EQ(decoder.datetime.time.hour, 0u);
        ASSERT_EQ(decoder.datetime.time.minute, 0u);
        ASSERT_EQ(decoder.datetime.time.second, 0u);
        ASSERT_EQ(decoder.datetime.time.subsecond, 0u);
        ASSERT_EQ(decoder.datetime.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.datetime.zone.offset, -(23 * 60 + 59));
    }
    { // Max
        Decoder decoder{"D9999-12-31T23:59:59.999999999+23:59"};
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.date.year, 9999u);
        ASSERT_EQ(decoder.datetime.date.month, 12u);
        ASSERT_EQ(decoder.datetime.date.day, 31u);
        ASSERT_EQ(decoder.datetime.time.hour, 23u);
        ASSERT_EQ(decoder.datetime.time.minute, 59u);
        ASSERT_EQ(decoder.datetime.time.second, 59u);
        ASSERT_EQ(decoder.datetime.time.subsecond, 999999999u);
        ASSERT_EQ(decoder.datetime.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.datetime.zone.offset, 23 * 60 + 59);
    }
    { // Positive timestamp
        const Timepoint tp{std::chrono::seconds{1676337198} + std::chrono::microseconds{123456}};
        Decoder decoder{};

        decoder.load("D2023-02-13T17:13:18.123456-08:00");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.date.year, 2023u);
        ASSERT_EQ(decoder.datetime.date.month, 2u);
        ASSERT_EQ(decoder.datetime.date.day, 13u);
        ASSERT_EQ(decoder.datetime.time.hour, 17u);
        ASSERT_EQ(decoder.datetime.time.minute, 13u);
        ASSERT_EQ(decoder.datetime.time.second, 18u);
        ASSERT_EQ(decoder.datetime.time.subsecond, 123456000u);
        ASSERT_EQ(decoder.datetime.zone.format, qcon::utcOffset);
        ASSERT_EQ(decoder.datetime.zone.offset, -480);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);

        decoder.load("D2023-02-14T01:13:18.123456Z");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.date.year, 2023u);
        ASSERT_EQ(decoder.datetime.date.month, 2u);
        ASSERT_EQ(decoder.datetime.date.day, 14u);
        ASSERT_EQ(decoder.datetime.time.hour, 1u);
        ASSERT_EQ(decoder.datetime.time.minute, 13u);
        ASSERT_EQ(decoder.datetime.time.second, 18u);
        ASSERT_EQ(decoder.datetime.time.subsecond, 123456000u);
        ASSERT_EQ(decoder.datetime.zone.format, qcon::utc);
        ASSERT_EQ(decoder.datetime.zone.offset, 0);
        ASSERT_EQ(decoder.datetime.toTimepoint(), tp);

        decoder.load("D2023-02-13T17:13:18.123456");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.date.year, 2023u);
        ASSERT_EQ(decoder.datetime.date.month, 2u);
        ASSERT_EQ(decoder.datetime.date.day, 13u);
        ASSERT_EQ(decoder.datetime.time.hour, 17u);
        ASSERT_EQ(decoder.datetime.time.minute, 13u);
        ASSERT_EQ(decoder.datetime.time.second, 18u);
        ASSERT_EQ(decoder.datetime.time.subsecond, 123456000u);
        ASSERT_EQ(decoder.datetime.zone.format, qcon::localTime);
        ASSERT_EQ(decoder.datetime.zone.offset, 0);
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
    { // Timezone without time
        ASSERT_TRUE(fails("D2023-02-13Z"));
        ASSERT_TRUE(fails("D2023-02-13+00:00"));
        ASSERT_TRUE(fails("D2023-02-13-00:00"));
    }
    { // Invalid timezone
        ASSERT_TRUE(fails("D1970-01-01T00:00:00Y"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00z"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:0000"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+1"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+11"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+111"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+11111"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+0X:00"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+00:0X"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+0:00"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+00:0"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+000:00"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+00:000"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+00-00"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+24:00"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00+00:60"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00-24:00"));
        ASSERT_TRUE(fails("D1970-01-01T00:00:00-00:60"));
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
    ASSERT_TRUE(decoder);
}

TEST(Decode, noSpace)
{
    Decoder decoder{R"({"a":["abc",-123,-123.456e-78,true,null]})"};
    ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "a");
        ASSERT_EQ(decoder.step(), DecodeState::array);
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
    ASSERT_TRUE(decoder);
}

TEST(Decode, extraneousSpace)
{
    Decoder decoder{" \t\n\r\v{} \t\n\r\v"};
    ASSERT_EQ(decoder.step(), DecodeState::object);
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);

        decoder.load("[0, ]");
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);

        decoder.load("[0 ,]");
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);

        decoder.load(R"({"k":0,})");
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "k");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);

        decoder.load(R"({"k":0, })");
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "k");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);

        decoder.load(R"({"k":0 ,})");
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "k");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);

        decoder.load("0 # AAAAA # BBBBB 1");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);
    }
    { // Multiple comments
        Decoder decoder{"# AAAAA\n#  BBBBB \n #CCCCC\n\n# DD DD\n0"};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);
    }
    { // Comment in string
        Decoder decoder{R"("# AAAAA")"};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "# AAAAA");
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);
    }
    { // Comments in object
        Decoder decoder{
R"({ # AAAAA
    "0": 0, # BBBBB
    "1": 1 # CCCCC
} # DDDDD)"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
            ASSERT_EQ(decoder.step(), DecodeState::key);
            ASSERT_EQ(decoder.key, "0");
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 0);
            ASSERT_EQ(decoder.step(), DecodeState::key);
            ASSERT_EQ(decoder.key, "1");
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
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
        ASSERT_TRUE(decoder);

        decoder.load("[ # AAAAA\r 0, # BBBBB\n    1 # CCCCC\r\n] # DDDDD \n");
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 1);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // Weirdness
        Decoder decoder{};

        decoder.load("#\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("# \n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("##\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("# #\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("#\n#\n0");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("0#");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("0# ");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("0#\n");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);

        decoder.load("0#\n#");
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder);
    }
    { // Nothing but comments
        ASSERT_TRUE(fails("# AAAAA\n# CCCCC\n"));
    }
}

TEST(Decode, depth)
{
    { // 64 nested objects
        Decoder decoder{R"({"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"v":true}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}})"};
        ASSERT_EQ(decoder.step(), DecodeState::object);
        for (int i{1}; i < 64; ++i)
        {
            ASSERT_EQ(decoder.step(), DecodeState::key);
            ASSERT_EQ(decoder.step(), DecodeState::object);
        }
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "v");
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, true);
        for (int i{0}; i < 64; ++i) ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // 64 nested arrays
        Decoder decoder{"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[true]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"};
        for (int i{0}; i < 64; ++i) ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_EQ(decoder.step(), DecodeState::boolean);
        ASSERT_EQ(decoder.boolean, true);
        for (int i{0}; i < 64; ++i) ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder);
    }
    { // 65 nested objects
        ASSERT_TRUE(fails(R"({"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"":{"v":true}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}})"));
    }
    { // 65 nested arrays
        ASSERT_TRUE(fails("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[true]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"));
    }
}

TEST(Decode, done)
{
    {
        Decoder decoder{""};
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    {
        Decoder decoder{"1"};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    {
        Decoder decoder{"["};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::error);
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    {
        Decoder decoder{"[1"};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::error);
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    {
        Decoder decoder{"[1]"};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::array);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    {
        Decoder decoder{"{"};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::error);
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    {
        Decoder decoder{"{\"k\":"};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::error);
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    {
        Decoder decoder{"{\"k\":1"};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::error);
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    {
        Decoder decoder{"{\"k\":1}"};
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_FALSE(decoder.done());
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
}

TEST(Decode, streamObject)
{
    Decoder decoder;
    std::string k1, k2, k3;
    s64 v1, v2, v3;

    decoder.load(R"({})");
    ASSERT_TRUE(decoder >> object >> end);

    decoder.load(R"( { } )");
    ASSERT_TRUE(decoder >> object >> end);

    decoder.load(R"({"k":1})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> end);
    ASSERT_EQ(k1, "k");
    ASSERT_EQ(v1, 1);

    decoder.load(R"({"key":1})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> end);
    ASSERT_EQ(k1, "key");
    ASSERT_EQ(v1, 1);

    decoder.load(R"( { "k1" : 1 } )");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> end);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(v1, 1);

    decoder.load(R"({"k1":1,"k2":2,"k3":3})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2 >> k3 >> v3 >> end);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(k3, "k3");
    ASSERT_EQ(v1, 1);
    ASSERT_EQ(v2, 2);
    ASSERT_EQ(v3, 3);

    decoder.load(R"( { "k1" : 1 , "k2" : 2 , "k3" : 3 } )");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2 >> k3 >> v3 >> end);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(k3, "k3");
    ASSERT_EQ(v1, 1);
    ASSERT_EQ(v2, 2);
    ASSERT_EQ(v3, 3);

    decoder.load(R"({"k1":1,})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> end);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(v1, 1);

    decoder.load(R"({"k1":{"k2":1,"k3":{}}})");
    ASSERT_TRUE(decoder >> object >> k1 >> object >> k2 >> v1 >> k3 >> object >> end >> end >> end);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(k3, "k3");
    ASSERT_EQ(v1, 1);

    decoder.load(R"( { "k1" : { "k2" : 1 , "k3" : { } , } , } )");
    ASSERT_TRUE(decoder >> object >> k1 >> object >> k2 >> v1 >> k3 >> object >> end >> end >> end);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(k3, "k3");
    ASSERT_EQ(v1, 1);

    decoder.load(R"({)");
    ASSERT_TRUE(decoder >> object);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"({,)");
    ASSERT_TRUE(decoder >> object);
    ASSERT_FALSE(decoder >> k1);

    decoder.load(R"({,})");
    ASSERT_TRUE(decoder >> object);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"({:)");
    ASSERT_TRUE(decoder >> object);
    ASSERT_FALSE(decoder >> k1);

    decoder.load(R"({1)");
    ASSERT_TRUE(decoder >> object);
    ASSERT_FALSE(decoder >> k1);

    decoder.load(R"({"k1")");
    ASSERT_TRUE(decoder >> object);
    ASSERT_FALSE(decoder >> k1);

    decoder.load(R"({"k1":)");
    ASSERT_TRUE(decoder >> object >> k1);
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"({"k1":1)");
    ASSERT_TRUE(decoder >> object >> k1 >> v1);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"(})");
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"({}})");
    ASSERT_TRUE(decoder >> object);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"({"k1":1,,"k2")");
    ASSERT_TRUE(decoder >> object >> k1 >> v1);
    ASSERT_FALSE(decoder >> k2);

    decoder.load(R"({"k1"::1)");
    ASSERT_TRUE(decoder >> object >> k1);
    ASSERT_FALSE(decoder >> v1);

    // Error propagation
    decoder.load(R"(^{})");
    ASSERT_FALSE(decoder >> object);
    ASSERT_FALSE(decoder >> object);
}

TEST(Decode, streamArray)
{
    Decoder decoder;
    s64 v1, v2, v3;

    decoder.load(R"([])");
    ASSERT_TRUE(decoder >> array >> end);

    decoder.load(R"( [ ] )");
    ASSERT_TRUE(decoder >> array >> end);

    decoder.load(R"([1])");
    ASSERT_TRUE(decoder >> array >> v1 >> end);
    ASSERT_EQ(v1, 1);

    decoder.load(R"([111])");
    ASSERT_TRUE(decoder >> array >> v1 >> end);
    ASSERT_EQ(v1, 111);

    decoder.load(R"( [ 1 ] )");
    ASSERT_TRUE(decoder >> array >> v1 >> end);
    ASSERT_EQ(v1, 1);

    decoder.load(R"([1,2,3])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2 >> v3 >> end);
    ASSERT_EQ(v1, 1);
    ASSERT_EQ(v2, 2);
    ASSERT_EQ(v3, 3);

    decoder.load(R"( [ 1 , 2 , 3 ] )");
    ASSERT_TRUE(decoder >> array >> v1 >> v2 >> v3 >> end);
    ASSERT_EQ(v1, 1);
    ASSERT_EQ(v2, 2);
    ASSERT_EQ(v3, 3);

    decoder.load(R"([1,])");
    ASSERT_TRUE(decoder >> array >> v1 >> end);
    ASSERT_EQ(v1, 1);

    decoder.load(R"([[1,[]]])");
    ASSERT_TRUE(decoder >> array >> array >> v1 >> array >> end >> end >> end);
    ASSERT_EQ(v1, 1);

    decoder.load(R"( [ [ 1 , [ ] , ] , ] )");
    ASSERT_TRUE(decoder >> array >> array >> v1 >> array >> end >> end >> end);
    ASSERT_EQ(v1, 1);

    decoder.load(R"([)");
    ASSERT_TRUE(decoder >> array);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"([,)");
    ASSERT_TRUE(decoder >> array);
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"([,])");
    ASSERT_TRUE(decoder >> array);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"([1)");
    ASSERT_TRUE(decoder >> array >> v1);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"(])");
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"([]])");
    ASSERT_TRUE(decoder >> array);
    ASSERT_FALSE(decoder >> end);

    decoder.load(R"([1,,2)");
    ASSERT_TRUE(decoder >> array >> v1);
    ASSERT_FALSE(decoder >> v2);

    // Error propagation
    decoder.load(R"(^[])");
    ASSERT_FALSE(decoder >> array);
    ASSERT_FALSE(decoder >> array);
}

TEST(Decode, streamString)
{
    Decoder decoder;
    std::string k1;
    std::string k2;
    std::string v1;
    std::string v2;

    decoder.load(R"("a")");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, "a");

    decoder.load(R"("abc")");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, "abc");

    decoder.load(R"("")");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, "");

    decoder.load(R"( "abc" )");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, "abc");

    decoder.load(R"({"k1": "v1", "k2": "v2"})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(v1, "v1");
    ASSERT_EQ(v2, "v2");

    decoder.load(R"(["v1", "v2"])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2);
    ASSERT_EQ(v1, "v1");
    ASSERT_EQ(v2, "v2");

    decoder.load(R"(")");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"("abc)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(abc")");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(""")");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"("", "")");
    ASSERT_FALSE(decoder >> v1);

    // Error propagation
    decoder.load(R"(^"abc")");
    ASSERT_FALSE(decoder >> v1);
    ASSERT_FALSE(decoder >> v1);
}

TEST(Decode, streamInteger)
{
    Decoder decoder;
    std::string k1;
    std::string k2;
    s64 v1;
    s64 v2;

    decoder.load(R"(0)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 0);

    decoder.load(R"(123)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 123);

    decoder.load(R"(+1)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 1);

    decoder.load(R"(-1)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, -1);

    decoder.load(R"( 1 )");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 1);

    decoder.load(R"({"k1": 1, "k2": 2})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(v1, 1);
    ASSERT_EQ(v2, 2);

    decoder.load(R"([1, 2])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2);
    ASSERT_EQ(v1, 1);
    ASSERT_EQ(v2, 2);

    decoder.load(R"(+ 1)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(- 1)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(++1)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(+-1)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(-+1)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(--1)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(1, 2)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(0x1A)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 0x1A);

    decoder.load(R"(0o17)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 017);

    decoder.load(R"(0b101)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 0b101);

    // Error propagation
    decoder.load(R"(^1)");
    ASSERT_FALSE(decoder >> v1);
    ASSERT_FALSE(decoder >> v1);
}

TEST(Decode, streamS64)
{
    Decoder decoder;
    s64 v;

    decoder.load(R"(18446744073709551615)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, -1);

    decoder.load(R"(18446744073709551616)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(9223372036854775807)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s64>::max());

    decoder.load(R"(-9223372036854775808)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_FALSE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s64>::min());

    decoder.load(R"(-9223372036854775809)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamS32)
{
    Decoder decoder;
    s32 v;

    decoder.load(R"(2147483647)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s32>::max());

    decoder.load(R"(2147483648)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(-2147483648)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_FALSE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s32>::min());

    decoder.load(R"(-2147483649)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamS16)
{
    Decoder decoder;
    s16 v;

    decoder.load(R"(32767)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s16>::max());

    decoder.load(R"(32768)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(-32768)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_FALSE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s16>::min());

    decoder.load(R"(-32769)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamS8)
{
    Decoder decoder;
    s8 v;

    decoder.load(R"(127)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s8>::max());

    decoder.load(R"(128)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(-128)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_FALSE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<s8>::min());

    decoder.load(R"(-129)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamU64)
{
    Decoder decoder;
    u64 v;

    decoder.load(R"(18446744073709551615)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<u64>::max());

    decoder.load(R"(18446744073709551616)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(-1)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamU32)
{
    Decoder decoder;
    u32 v;

    decoder.load(R"(4294967295)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<u32>::max());

    decoder.load(R"(4294967296)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(-1)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamU16)
{
    Decoder decoder;
    u16 v;

    decoder.load(R"(65535)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<u16>::max());

    decoder.load(R"(65536)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(-1)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamU8)
{
    Decoder decoder;
    u8 v;

    decoder.load(R"(255)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_TRUE(decoder.positive);
    ASSERT_EQ(v, std::numeric_limits<u8>::max());

    decoder.load(R"(256)");
    ASSERT_FALSE(decoder >> v);

    decoder.load(R"(-1)");
    ASSERT_FALSE(decoder >> v);
}

TEST(Decode, streamFloater)
{
    Decoder decoder;
    std::string k1;
    std::string k2;
    double v1;
    double v2;

    decoder.load(R"(0.0)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 0.0);

    decoder.load(R"(123.4)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 123.4);

    decoder.load(R"(+1.0)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 1.0);

    decoder.load(R"(-1.0)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, -1.0);

    decoder.load(R"( 1.0 )");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 1.0);

    decoder.load(R"({"k1": 1.0, "k2": 2.0})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(v1, 1.0);
    ASSERT_EQ(v2, 2.0);

    decoder.load(R"([1.0, 2.0])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2);
    ASSERT_EQ(v1, 1.0);
    ASSERT_EQ(v2, 2.0);

    decoder.load(R"(+ 1.0)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(- 1.0)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(++1.0)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(+-1.0)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(-+1.0)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(--1.0)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(1.0, 2.0)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(1.234e2)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, 123.4);

    decoder.load(R"(inf)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, std::numeric_limits<double>::infinity());

    decoder.load(R"(+inf)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, std::numeric_limits<double>::infinity());

    decoder.load(R"(-inf)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, -std::numeric_limits<double>::infinity());

    decoder.load(R"(in)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(nan)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_TRUE(std::isnan(v1));

    decoder.load(R"(+nan)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_TRUE(std::isnan(v1));

    decoder.load(R"(-nan)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_TRUE(std::isnan(v1));

    decoder.load(R"(na)");
    ASSERT_FALSE(decoder >> v1);

    // Error propagation
    decoder.load(R"(^1.0)");
    ASSERT_FALSE(decoder >> v1);
    ASSERT_FALSE(decoder >> v1);
}

TEST(Decode, streamFloaterOther)
{
    Decoder decoder;
    float v;

    decoder.load(R"(-12.5)");
    ASSERT_TRUE(decoder >> v);
    ASSERT_EQ(v, -12.5f);
}

TEST(Decode, streamBoolean)
{
    Decoder decoder;
    std::string k1;
    std::string k2;
    bool v1;
    bool v2;

    decoder.load(R"(true)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, true);

    decoder.load(R"(false)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, false);

    decoder.load(R"( true )");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1, true);

    decoder.load(R"({"k1": true, "k2": false})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(v1, true);
    ASSERT_EQ(v2, false);

    decoder.load(R"([true, false])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2);
    ASSERT_EQ(v1, true);
    ASSERT_EQ(v2, false);

    decoder.load(R"(true, false)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(tru)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(fals)");
    ASSERT_FALSE(decoder >> v1);

    // Error propagation
    decoder.load(R"(^false)");
    ASSERT_FALSE(decoder >> v1);
    ASSERT_FALSE(decoder >> v1);
}

TEST(Decode, streamDate)
{
    Decoder decoder;
    std::string k1;
    std::string k2;
    Date v1;
    Date v2;

    decoder.load(R"(D1986-04-22)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1.year, 1986);
    ASSERT_EQ(v1.month, 4);
    ASSERT_EQ(v1.day, 22);

    decoder.load(R"( D1986-04-22 )");
    ASSERT_TRUE(decoder >> v1);

    decoder.load(R"({"k1": D1986-04-22, "k2": D1987-05-23})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(v1.year, 1986);
    ASSERT_EQ(v1.month, 4);
    ASSERT_EQ(v1.day, 22);
    ASSERT_EQ(v2.year, 1987);
    ASSERT_EQ(v2.month, 5);
    ASSERT_EQ(v2.day, 23);

    decoder.load(R"([D1986-04-22, D1987-05-23])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2);
    ASSERT_EQ(v1.year, 1986);
    ASSERT_EQ(v1.month, 4);
    ASSERT_EQ(v1.day, 22);
    ASSERT_EQ(v2.year, 1987);
    ASSERT_EQ(v2.month, 5);
    ASSERT_EQ(v2.day, 23);

    decoder.load(R"(D1986-04-22, D1987-05-23)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(D1986-04)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(D1986)");
    ASSERT_FALSE(decoder >> v1);

    // Error propagation
    decoder.load(R"(^D1986-04-22)");
    ASSERT_FALSE(decoder >> v1);
    ASSERT_FALSE(decoder >> v1);
}

TEST(Decode, streamTime)
{
    Decoder decoder;
    std::string k1;
    std::string k2;
    Time v1;
    Time v2;

    decoder.load(R"(T06:31:50)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1.hour, 6);
    ASSERT_EQ(v1.minute, 31);
    ASSERT_EQ(v1.second, 50);

    decoder.load(R"( T06:31:50 )");
    ASSERT_TRUE(decoder >> v1);

    decoder.load(R"({"k1": T06:31:50, "k2": T07:32:51})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(v1.hour, 6);
    ASSERT_EQ(v1.minute, 31);
    ASSERT_EQ(v1.second, 50);
    ASSERT_EQ(v2.hour, 7);
    ASSERT_EQ(v2.minute, 32);
    ASSERT_EQ(v2.second, 51);

    decoder.load(R"([T06:31:50, T07:32:51])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2);
    ASSERT_EQ(v1.hour, 6);
    ASSERT_EQ(v1.minute, 31);
    ASSERT_EQ(v1.second, 50);
    ASSERT_EQ(v2.hour, 7);
    ASSERT_EQ(v2.minute, 32);
    ASSERT_EQ(v2.second, 51);

    decoder.load(R"(T06:31:50, T07:32:51)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(T06:31)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(T06)");
    ASSERT_FALSE(decoder >> v1);

    // Error propagation
    decoder.load(R"(^T06:31:50)");
    ASSERT_FALSE(decoder >> v1);
    ASSERT_FALSE(decoder >> v1);
}

TEST(Decode, streamDatetime)
{
    Decoder decoder;
    std::string k1;
    std::string k2;
    Datetime v1;
    Datetime v2;

    decoder.load(R"(D1986-04-22T06:31:50)");
    ASSERT_TRUE(decoder >> v1);
    ASSERT_EQ(v1.date.year, 1986);
    ASSERT_EQ(v1.date.month, 4);
    ASSERT_EQ(v1.date.day, 22);

    decoder.load(R"( D1986-04-22T06:31:50 )");
    ASSERT_TRUE(decoder >> v1);

    decoder.load(R"({"k1": D1986-04-22T06:31:50, "k2": D1987-05-23T07:32:51})");
    ASSERT_TRUE(decoder >> object >> k1 >> v1 >> k2 >> v2);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");
    ASSERT_EQ(v1.date.year, 1986);
    ASSERT_EQ(v1.date.month, 4);
    ASSERT_EQ(v1.date.day, 22);
    ASSERT_EQ(v2.time.hour, 7);
    ASSERT_EQ(v2.time.minute, 32);
    ASSERT_EQ(v2.time.second, 51);
    ASSERT_EQ(v2.date.year, 1987);
    ASSERT_EQ(v2.date.month, 5);
    ASSERT_EQ(v2.date.day, 23);
    ASSERT_EQ(v1.time.hour, 6);
    ASSERT_EQ(v1.time.minute, 31);
    ASSERT_EQ(v1.time.second, 50);

    decoder.load(R"([D1986-04-22T06:31:50, D1987-05-23T07:32:51])");
    ASSERT_TRUE(decoder >> array >> v1 >> v2);
    ASSERT_EQ(v1.date.year, 1986);
    ASSERT_EQ(v1.date.month, 4);
    ASSERT_EQ(v1.date.day, 22);
    ASSERT_EQ(v1.time.hour, 6);
    ASSERT_EQ(v1.time.minute, 31);
    ASSERT_EQ(v1.time.second, 50);
    ASSERT_EQ(v2.date.year, 1987);
    ASSERT_EQ(v2.date.month, 5);
    ASSERT_EQ(v2.date.day, 23);
    ASSERT_EQ(v2.time.hour, 7);
    ASSERT_EQ(v2.time.minute, 32);
    ASSERT_EQ(v2.time.second, 51);

    decoder.load(R"(D1986-04-22T06:31:50, D1987-05-23T07:32:51)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(D1986-04-22T)");
    ASSERT_FALSE(decoder >> v1);

    decoder.load(R"(D1986-04T06:31)");
    ASSERT_FALSE(decoder >> v1);

    // Error propagation
    decoder.load(R"(^D1986-04-22T06:31:50)");
    ASSERT_FALSE(decoder >> v1);
    ASSERT_FALSE(decoder >> v1);
}

TEST(Decode, streamNull)
{
    Decoder decoder;
    std::string k1;
    std::string k2;

    decoder.load(R"(null)");
    ASSERT_TRUE(decoder >> nullptr);

    decoder.load(R"( null )");
    ASSERT_TRUE(decoder >> nullptr);

    decoder.load(R"({"k1": null, "k2": null})");
    ASSERT_TRUE(decoder >> object >> k1 >> nullptr >> k2 >> nullptr);
    ASSERT_EQ(k1, "k1");
    ASSERT_EQ(k2, "k2");

    decoder.load(R"([null, null])");
    ASSERT_TRUE(decoder >> array >> nullptr >> nullptr);

    decoder.load(R"(null, null)");
    ASSERT_FALSE(decoder >> nullptr);

    decoder.load(R"(nul)");
    ASSERT_FALSE(decoder >> nullptr);

    // Error propagation
    decoder.load(R"(^null)");
    ASSERT_FALSE(decoder >> nullptr);
    ASSERT_FALSE(decoder >> nullptr);
}

TEST(Decode, tryEnd)
{
    { // Array empty
        Decoder decoder{R"([])"};
        ASSERT_TRUE(decoder >> array);
        ASSERT_TRUE(decoder.tryEnd());
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    { // Array single
        s64 v;
        Decoder decoder{R"([ 0 ])"};
        ASSERT_TRUE(decoder >> array);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 0);
        ASSERT_TRUE(decoder.tryEnd());
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    { // Array multiple
        s64 v;
        Decoder decoder{R"([ 0, 1, 2 ])"};
        ASSERT_TRUE(decoder >> array);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 0);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 1);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 2);
        ASSERT_TRUE(decoder.tryEnd());
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    { // Object empty
        Decoder decoder{R"({})"};
        ASSERT_TRUE(decoder >> object);
        ASSERT_TRUE(decoder.tryEnd());
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    { // Object single
        std::string k;
        s64 v;
        Decoder decoder{R"({ "0": 0 })"};
        ASSERT_TRUE(decoder >> object);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> k);
        ASSERT_EQ(k, "0");
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 0);
        ASSERT_TRUE(decoder.tryEnd());
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    { // Object multiple
        std::string k;
        s64 v;
        Decoder decoder{R"({ "0": 0, "1": 1, "2": 2 })"};
        ASSERT_TRUE(decoder >> object);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> k);
        ASSERT_EQ(k, "0");
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 0);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> k);
        ASSERT_EQ(k, "1");
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 1);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> k);
        ASSERT_EQ(k, "2");
        ASSERT_TRUE(decoder >> v);
        ASSERT_EQ(v, 2);
        ASSERT_TRUE(decoder.tryEnd());
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    { // Value
        Decoder decoder{R"(1)"};
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder);
    }
    { // Empty
        Decoder decoder{R"()"};
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_FALSE(decoder);
    }
    { // Trailing comma
        Decoder decoder{R"([ 0, ])"};
        ASSERT_TRUE(decoder >> array >> decoder.integer);
        ASSERT_TRUE(decoder.tryEnd());
        ASSERT_TRUE(decoder.done());
        ASSERT_TRUE(decoder);
    }
    { // Lone brace
        Decoder decoder{R"(])"};
        ASSERT_FALSE(decoder.tryEnd());
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
    { // Trailing root comma
        ASSERT_TRUE(fails("1,"));
    }
    { // Lone decimal
        ASSERT_TRUE(fails("."));
    }
    { // Stepping on error
        Decoder decoder{"squash"};
        ASSERT_EQ(decoder.step(), DecodeState::error);
        ASSERT_EQ(decoder.step(), DecodeState::error);
    }
    { // Stepping once done
        Decoder decoder{"null"};
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_TRUE(decoder);
        ASSERT_EQ(decoder.step(), DecodeState::error);
    }
}

TEST(Decoder, miscStream)
{
    { // Mismatched containers
        Decoder decoder{R"({])"};
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_TRUE(decoder >> object);
        ASSERT_FALSE(decoder >> end);
    }
    { // Mismatched containers
        Decoder decoder{R"([})"};
        ASSERT_TRUE(decoder >> array);
        ASSERT_FALSE(decoder.tryEnd());
        ASSERT_FALSE(decoder >> end);
    }
    { // Empty
        Decoder decoder{""};
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    { // Space only
        Decoder decoder{"   "};
        ASSERT_TRUE(decoder.done());
        ASSERT_FALSE(decoder);
    }
    { // Unknown value
        Decoder decoder{"^"};
        ASSERT_FALSE(decoder.done());
        ASSERT_FALSE(decoder >> nullptr);
    }
    { // Multiple root values
        Decoder decoder{"1 2"};
        ASSERT_FALSE(decoder >> decoder.integer);
    }
    { // Trailing root comma
        Decoder decoder{"1,"};
        ASSERT_FALSE(decoder >> decoder.integer);
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
    "Green Eggs and Ham": "I do not like them in a box\n"
                          "I do not like them with a fox\n"
                          "I do not like them in a house\n"
                          "I do not like them with a mouse\n"
                          "I do not like them here or there\n"
                          "I do not like them anywhere\n"
                          "I do not like green eggs and ham\n"
                          "I do not like them Sam I am\n",
    "Magic Numbers": [0x309,0o1411,0b1100001001], # What could they mean?!
    "Last Updated": D2003-06-28T13:59:11.067Z
})"};
    Decoder decoder{qcon};
    ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Name");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "Salt's Crust");
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Founded");
        ASSERT_EQ(decoder.step(), DecodeState::date);
        ASSERT_EQ(decoder.date.year, 1964u);
        ASSERT_EQ(decoder.date.month, 3u);
        ASSERT_EQ(decoder.date.day, 17u);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Opens");
        ASSERT_EQ(decoder.step(), DecodeState::time);
        ASSERT_EQ(decoder.time.hour, 8u);
        ASSERT_EQ(decoder.time.minute, 30u);
        ASSERT_EQ(decoder.time.second, 0u);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Employees");
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Ol' Joe Fisher");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Fisherman");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.integer, 69);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Mark Rower");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Cook");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.integer, 41);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Phineas");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Server Boy");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_EQ(decoder.step(), DecodeState::integer);
                ASSERT_EQ(decoder.integer, 19);
            ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Dishes");
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Basket o' Barnacles");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.floater, 5.45);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Ingredients");
                ASSERT_EQ(decoder.step(), DecodeState::array);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "\"Salt\"");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Barnacles");
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "Two Tuna");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Ingredients");
                ASSERT_EQ(decoder.step(), DecodeState::array);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Tuna");
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.boolean, true);
            ASSERT_EQ(decoder.step(), DecodeState::end);
            ASSERT_EQ(decoder.step(), DecodeState::object);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_EQ(decoder.step(), DecodeState::string);
                ASSERT_EQ(decoder.string, "18 Leg Bouquet");
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_EQ(decoder.step(), DecodeState::floater);
                ASSERT_TRUE(std::isnan(decoder.floater));
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Ingredients");
                ASSERT_EQ(decoder.step(), DecodeState::array);
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "\"Salt\"");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Octopus");
                    ASSERT_EQ(decoder.step(), DecodeState::string);
                    ASSERT_EQ(decoder.string, "Crab");
                ASSERT_EQ(decoder.step(), DecodeState::end);
                ASSERT_EQ(decoder.step(), DecodeState::key);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_EQ(decoder.step(), DecodeState::boolean);
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Profit Margin");
        ASSERT_EQ(decoder.step(), DecodeState::null);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Ha\x03r Name");
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "M\0\0n"sv);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Green Eggs and Ham");
        ASSERT_EQ(decoder.step(), DecodeState::string);
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
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Magic Numbers");
        ASSERT_EQ(decoder.step(), DecodeState::array);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_EQ(decoder.step(), DecodeState::integer);
            ASSERT_EQ(decoder.integer, 777);
        ASSERT_EQ(decoder.step(), DecodeState::end);
        ASSERT_EQ(decoder.step(), DecodeState::key);
        ASSERT_EQ(decoder.key, "Last Updated");
        ASSERT_EQ(decoder.step(), DecodeState::datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::seconds{1056808751} + std::chrono::milliseconds{67}});
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_TRUE(decoder);

    // Stream
    decoder.load(qcon);
    ASSERT_TRUE(decoder >> object);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Name");
        ASSERT_TRUE(decoder >> decoder.string);
        ASSERT_EQ(decoder.string, "Salt's Crust");
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Founded");
        ASSERT_TRUE(decoder >> decoder.date);
        ASSERT_EQ(decoder.date.year, 1964u);
        ASSERT_EQ(decoder.date.month, 3u);
        ASSERT_EQ(decoder.date.day, 17u);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Opens");
        ASSERT_TRUE(decoder >> decoder.time);
        ASSERT_EQ(decoder.time.hour, 8u);
        ASSERT_EQ(decoder.time.minute, 30u);
        ASSERT_EQ(decoder.time.second, 0u);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Employees");
        ASSERT_TRUE(decoder >> array);
            ASSERT_TRUE(decoder >> object);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Ol' Joe Fisher");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Fisherman");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_TRUE(decoder >> decoder.integer);
                ASSERT_EQ(decoder.integer, 69);
            ASSERT_TRUE(decoder >> end);
            ASSERT_TRUE(decoder >> object);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Mark Rower");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Cook");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_TRUE(decoder >> decoder.integer);
                ASSERT_EQ(decoder.integer, 41);
            ASSERT_TRUE(decoder >> end);
            ASSERT_TRUE(decoder >> object);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Phineas");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Title");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Server Boy");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Age");
                ASSERT_TRUE(decoder >> decoder.integer);
                ASSERT_EQ(decoder.integer, 19);
            ASSERT_TRUE(decoder >> end);
        ASSERT_TRUE(decoder >> end);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Dishes");
        ASSERT_TRUE(decoder >> array);
            ASSERT_TRUE(decoder >> object);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Basket o' Barnacles");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_TRUE(decoder >> decoder.floater);
                ASSERT_EQ(decoder.floater, 5.45);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Ingredients");
                ASSERT_TRUE(decoder >> array);
                    ASSERT_TRUE(decoder >> decoder.string);
                    ASSERT_EQ(decoder.string, "\"Salt\"");
                    ASSERT_TRUE(decoder >> decoder.string);
                    ASSERT_EQ(decoder.string, "Barnacles");
                ASSERT_TRUE(decoder >> end);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_TRUE(decoder >> decoder.boolean);
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_TRUE(decoder >> end);
            ASSERT_TRUE(decoder >> object);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "Two Tuna");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_TRUE(decoder >> decoder.floater);
                ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Ingredients");
                ASSERT_TRUE(decoder >> array);
                    ASSERT_TRUE(decoder >> decoder.string);
                    ASSERT_EQ(decoder.string, "Tuna");
                ASSERT_TRUE(decoder >> end);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_TRUE(decoder >> decoder.boolean);
                ASSERT_EQ(decoder.boolean, true);
            ASSERT_TRUE(decoder >> end);
            ASSERT_TRUE(decoder >> object);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Name");
                ASSERT_TRUE(decoder >> decoder.string);
                ASSERT_EQ(decoder.string, "18 Leg Bouquet");
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Price");
                ASSERT_TRUE(decoder >> decoder.floater);
                ASSERT_TRUE(std::isnan(decoder.floater));
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Ingredients");
                ASSERT_TRUE(decoder >> array);
                    ASSERT_TRUE(decoder >> decoder.string);
                    ASSERT_EQ(decoder.string, "\"Salt\"");
                    ASSERT_TRUE(decoder >> decoder.string);
                    ASSERT_EQ(decoder.string, "Octopus");
                    ASSERT_TRUE(decoder >> decoder.string);
                    ASSERT_EQ(decoder.string, "Crab");
                ASSERT_TRUE(decoder >> end);
                ASSERT_TRUE(decoder >> decoder.key);
                ASSERT_EQ(decoder.key, "Gluten Free");
                ASSERT_TRUE(decoder >> decoder.boolean);
                ASSERT_EQ(decoder.boolean, false);
            ASSERT_TRUE(decoder >> end);
        ASSERT_TRUE(decoder >> end);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Profit Margin");
        ASSERT_TRUE(decoder >> nullptr);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Ha\x03r Name");
        ASSERT_TRUE(decoder >> decoder.string);
        ASSERT_EQ(decoder.string, "M\0\0n"sv);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Green Eggs and Ham");
        ASSERT_TRUE(decoder >> decoder.string);
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
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Magic Numbers");
        ASSERT_TRUE(decoder >> array);
            ASSERT_TRUE(decoder >> decoder.integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_TRUE(decoder >> decoder.integer);
            ASSERT_EQ(decoder.integer, 777);
            ASSERT_TRUE(decoder >> decoder.integer);
            ASSERT_EQ(decoder.integer, 777);
        ASSERT_TRUE(decoder >> end);
        ASSERT_TRUE(decoder >> decoder.key);
        ASSERT_EQ(decoder.key, "Last Updated");
        ASSERT_TRUE(decoder >> decoder.datetime);
        ASSERT_EQ(decoder.datetime.toTimepoint(), Timepoint{std::chrono::seconds{1056808751} + std::chrono::milliseconds{67}});
    ASSERT_TRUE(decoder >> end);
    ASSERT_TRUE(decoder);
}
