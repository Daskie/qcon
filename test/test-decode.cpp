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

TEST(decode, object)
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

TEST(decode, array)
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

TEST(decode, string)
{
    { // Empty string
        Decoder decoder{R"("")"sv};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, ""sv);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // All printable
        Decoder decoder{R"(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")"sv};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // All non-printable
        std::string decodeStr{R"(" ")"};
        for (int i{0}; i < 256; ++i)
        {
            if (!std::isprint(i))
            {
                decodeStr[1] = char(i);
                ASSERT_TRUE(fails(decodeStr));
            }
        }
    }
    { // Escape characters
        Decoder decoder{R"("\0\b\t\n\v\f\r")"sv};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\0\b\t\n\v\f\r"sv);
        ASSERT_EQ(decoder.step(), DecodeState::done);
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
        std::string expectedStr(256, '\0');
        std::string decodeStr(1 + 256 * 4 + 1, '\0');
        decodeStr.front() = '"';
        decodeStr.back() = '"';
        for (unat i{0u}; i < 256u; ++i)
        {
            expectedStr[i] = char(i);
            std::format_to_n(&decodeStr[1u + 4u * i], 4, "\\x{:02X}"sv, i);
        }
        Decoder decoder{decodeStr};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, expectedStr);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // 'u' code point
        std::string expectedStr(256, '\0');
        std::string decodeStr(1 + 256 * 6 + 1, '\0');
        decodeStr.front() = '"';
        decodeStr.back() = '"';
        for (unat i{0u}; i < 256u; ++i)
        {
            expectedStr[i] = char(i);
            std::format_to_n(&decodeStr[1u + 6u * i], 6, "\\u{:04X}"sv, i);
        }
        Decoder decoder{decodeStr};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, expectedStr);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // 'U' code point
        Decoder decoder{R"("\U00000077\U000000FF\UFFFFFFFF")"};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\u0077\u00FF\u00FF");
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Uppercase and lowercase code point hex digits
        Decoder decoder{R"("\u00aa\u00BB\u00cC\u00Dd")"};
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.string, "\u00AA\u00BB\u00CC\u00DD");
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Incorrect number of code point digits
        // Raw strings, `\x`/`\u`, and macros don't play nice together
        ASSERT_TRUE(fails("\"\\x\""));
        ASSERT_TRUE(fails("\"\\x1\""));
        ASSERT_TRUE(fails("\"\\u\""));
        ASSERT_TRUE(fails("\"\\u1\""));
        ASSERT_TRUE(fails("\"\\u11\""));
        ASSERT_TRUE(fails("\"\\u111\""));
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
    }
    { // Single quotes
        ASSERT_TRUE(fails(R"('abc')"));
    }
}

TEST(decode, decimal)
{
    { // Zero
        Decoder decoder{R"(0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Normal
        Decoder decoder{R"(123)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+123)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer,123);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-123)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -123);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-9223372036854775808)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+9223372036854775807)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max unsigned
        Decoder decoder{R"(+18446744073709551615)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{};

        decoder.load(R"(0123)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 123);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(00)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(+00)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(-00)"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-000000009223372036854775808)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+000000009223372036854775807)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::max());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max unsigned with leading zeroes
        Decoder decoder{R"(+0000000018446744073709551615)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
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

TEST(decode, hex)
{
    { // Zero
        Decoder decoder{R"(0x0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Lowercase
        Decoder decoder{R"(0x1a)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Uppercase
        Decoder decoder{R"(0x1A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+0x1A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-0x1A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -26);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-0x8000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+0xFFFFFFFFFFFFFFFF)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{R"(0x001A)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 26);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-0x000000008000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+0x00000000FFFFFFFFFFFFFFFF)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
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

TEST(decode, octal)
{
    { // Zero
        Decoder decoder{R"(0o0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{R"(0o12)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+0o12)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-0o12)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -10);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-0o1000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+0o1777777777777777777777)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{R"(0o0012)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 10);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-0o000000001000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+0o000000001777777777777777777777)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
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

TEST(decode, binary)
{
    { // Zero
        Decoder decoder{R"(0b0)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{R"(0b101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+0b101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-0b101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, -5);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min
        Decoder decoder{R"(-0b1000000000000000000000000000000000000000000000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max
        Decoder decoder{R"(+0b1111111111111111111111111111111111111111111111111111111111111111)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Leading zeroes
        Decoder decoder{R"(0b00101)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, 5);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Min with leading zeroes
        Decoder decoder{R"(-0b000000001000000000000000000000000000000000000000000000000000000000000000)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<s64>::min());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max with leading zeroes
        Decoder decoder{R"(+0b000000001111111111111111111111111111111111111111111111111111111111111111)"sv};
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.integer, std::numeric_limits<u64>::max());
        ASSERT_TRUE(decoder.boolean);
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

TEST(decode, floater)
{
    { // Zero
        Decoder decoder{R"(0.0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive zero
        Decoder decoder{R"(+0.0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative Zero
        Decoder decoder{R"(-0.0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.0);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // No sign
        Decoder decoder{R"(123.456)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive
        Decoder decoder{R"(+123.456)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative
        Decoder decoder{R"(-123.456)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent lowercase
        Decoder decoder{R"(123.456e17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent uppercase
        Decoder decoder{R"(123.456E17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Positive exponent
        Decoder decoder{R"(123.456e+17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.456e17);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Negative exponent
        Decoder decoder{R"(-123.456e-17)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -123.456e-17);
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Exponent without fraction
        Decoder decoder{R"(123e34)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 123.0e34);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Max integer
        Decoder decoder{R"(9007199254740991.0e0)"};
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 9007199254740991.0);
        ASSERT_TRUE(decoder.boolean);
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
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(00.2)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 0.2);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e02)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e02);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e+02)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e+02);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e-02)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e-02);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(1e00)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, 1e00);
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Valid infinity
        Decoder decoder{};

        decoder.load(R"(inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(-inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(+inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(Inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(-Inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(+Inf)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(infinity)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(-infinity)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(+infinity)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(Infinity)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(-Infinity)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, -std::numeric_limits<double>::infinity());
        ASSERT_FALSE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(+Infinity)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_EQ(decoder.floater, std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decoder.boolean);
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid infinity
        ASSERT_TRUE(fails(R"(iNf)"));
        ASSERT_TRUE(fails(R"(inF)"));
        ASSERT_TRUE(fails(R"(INF)"));
        ASSERT_TRUE(fails(R"(infi)"));
        ASSERT_TRUE(fails(R"(infin)"));
        ASSERT_TRUE(fails(R"(infini)"));
        ASSERT_TRUE(fails(R"(infinit)"));
        ASSERT_TRUE(fails(R"(iNfinity)"));
        ASSERT_TRUE(fails(R"(inFinity)"));
        ASSERT_TRUE(fails(R"(infInity)"));
        ASSERT_TRUE(fails(R"(infiNity)"));
        ASSERT_TRUE(fails(R"(infinIty)"));
        ASSERT_TRUE(fails(R"(infiniTy)"));
        ASSERT_TRUE(fails(R"(infinitY)"));
        ASSERT_TRUE(fails(R"(Infi)"));
        ASSERT_TRUE(fails(R"(Infin)"));
        ASSERT_TRUE(fails(R"(Infini)"));
        ASSERT_TRUE(fails(R"(Infinit)"));
        ASSERT_TRUE(fails(R"(INfinity)"));
        ASSERT_TRUE(fails(R"(InFinity)"));
        ASSERT_TRUE(fails(R"(InfInity)"));
        ASSERT_TRUE(fails(R"(InfiNity)"));
        ASSERT_TRUE(fails(R"(InfinIty)"));
        ASSERT_TRUE(fails(R"(InfiniTy)"));
        ASSERT_TRUE(fails(R"(InfinitY)"));
        ASSERT_TRUE(fails(R"(INFINITY)"));
        ASSERT_TRUE(fails(R"(infstuff)"));
        ASSERT_TRUE(fails(R"(infinitystuff)"));
    }
    { // Valid NaN
        Decoder decoder{};

        decoder.load(R"(nan)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_TRUE(std::isnan(decoder.floater));
        ASSERT_EQ(decoder.step(), DecodeState::done);

        decoder.load(R"(NaN)");
        ASSERT_EQ(decoder.step(), DecodeState::floater);
        ASSERT_TRUE(std::isnan(decoder.floater));
        ASSERT_EQ(decoder.step(), DecodeState::done);
    }
    { // Invalid NaN
        ASSERT_TRUE(fails(R"(Nan)"));
        ASSERT_TRUE(fails(R"(nAn)"));
        ASSERT_TRUE(fails(R"(naN)"));
        ASSERT_TRUE(fails(R"(NAN)"));
        ASSERT_TRUE(fails(R"(nanstuff)"));
        ASSERT_TRUE(fails(R"(NaNstuff)"));
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

TEST(decode, boolean)
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

TEST(decode, null)
{
    Decoder decoder{R"(null)"sv};
    ASSERT_EQ(decoder.step(), DecodeState::null);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}

TEST(decode, noSpace)
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

TEST(decode, extraneousSpace)
{
    Decoder decoder{" \t\n\r\v{} \t\n\r\v"sv};
    ASSERT_EQ(decoder.step(), DecodeState::object);
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}

TEST(decode, trailingComma)
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

TEST(decode, comments)
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

TEST(decode, depth)
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

TEST(decode, misc)
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

TEST(decode, general)
{
    const std::string_view qcon{
R"(
# Third quarter summary document
# Protected information, do not propagate!
{
    "Name": "Salt's Crust",
    "Founded": 1964,
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
    "Magic Numbers": [0x309,0o1411,0b1100001001] # What could they mean?!
})"sv};
    Decoder decoder{qcon};
    ASSERT_EQ(decoder.step(), DecodeState::object);
        ASSERT_EQ(decoder.step(), DecodeState::string);
        ASSERT_EQ(decoder.key, "Name"sv);
        ASSERT_EQ(decoder.string, "Salt's Crust"sv);
        ASSERT_EQ(decoder.step(), DecodeState::integer);
        ASSERT_EQ(decoder.key, "Founded"sv);
        ASSERT_EQ(decoder.integer, 1964);
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
    ASSERT_EQ(decoder.step(), DecodeState::end);
    ASSERT_EQ(decoder.step(), DecodeState::done);
}
