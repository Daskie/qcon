#include <qcon-encode.hpp>

#include <format>

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

using qcon::Encoder;
using qcon::Datetime;
using enum qcon::Container;
using enum qcon::Density;
using enum qcon::Base;
using enum qcon::TimezoneFormat;

struct CustomVal { int x, y; };

Encoder & operator<<(Encoder & encoder, const CustomVal & v)
{
    return encoder << uniline << array << v.x << v.y << end;
}

template <typename T>
std::ostream & operator<<(std::ostream & os, const std::optional<T> & v)
{
    if (v)
    {
        os << *v;
    }
    else
    {
        os << "Optional is empty";
    }
    return os;
}

TEST(encode, object)
{
    { // Empty
        Encoder encoder{};
        encoder << multiline << object << end;
        ASSERT_EQ(R"({})"s, encoder.finish());
        encoder << uniline << object << end;
        ASSERT_EQ(R"({})"s, encoder.finish());
        encoder << nospace << object << end;
        ASSERT_EQ(R"({})"s, encoder.finish());
    }
    { // Non-empty
        Encoder encoder{};
        encoder << multiline << object << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(R"({
    "k1": "abc",
    "k2": 123,
    "k3": true
})"s, encoder.finish());
        encoder << uniline << object << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(R"({ "k1": "abc", "k2": 123, "k3": true })"s, encoder.finish());
        encoder << nospace << object << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(R"({"k1":"abc","k2":123,"k3":true})"s, encoder.finish());
    }
    { // String view key
        Encoder encoder{uniline};
        encoder << object << "k"sv << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // String key
        Encoder encoder{uniline};
        encoder << object << "k"s << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Const C string key
        Encoder encoder{uniline};
        encoder << object << "k" << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Mutable C string key
        Encoder encoder{uniline};
        encoder << object << const_cast<char *>("k") << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Character key
        Encoder encoder{uniline};
        encoder << object << 'k' << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Empty key
        Encoder encoder{uniline};
        encoder << object << "" << "" << end;
        ASSERT_EQ(R"({ "": "" })"s, encoder.finish());
    }
    { // Integer key
        Encoder encoder{};
        encoder << object;
        encoder << 123;
        ASSERT_FALSE(encoder.status());
    }
    { // Floater key
        Encoder encoder{};
        encoder << object;
        ASSERT_TRUE(encoder.status());
        encoder << 123.4;
        ASSERT_FALSE(encoder.status());
    }
    { // Boolean key
        Encoder encoder{};
        encoder << object;
        ASSERT_TRUE(encoder.status());
        encoder << true;
        ASSERT_FALSE(encoder.status());
    }
    { // Null key
        Encoder encoder{};
        encoder << object;
        ASSERT_TRUE(encoder.status());
        encoder << nullptr;
        ASSERT_FALSE(encoder.status());
    }
    { // Dangling key
        Encoder encoder{};
        encoder << object << "k1";
        ASSERT_TRUE(encoder.status());
        encoder << end;
        ASSERT_FALSE(encoder.status());
    }
}

TEST(encode, array)
{
    { // Empty
        Encoder encoder{};
        encoder << multiline << array << end;
        ASSERT_EQ(R"([])"s, encoder.finish());
        encoder << uniline << array << end;
        ASSERT_EQ(R"([])"s, encoder.finish());
        encoder << nospace << array << end;
        ASSERT_EQ(R"([])"s, encoder.finish());
    }
    { // Non-empty
        Encoder encoder{};
        encoder << multiline << array << "abc" << 123 << true << end;
        ASSERT_EQ(R"([
    "abc",
    123,
    true
])"s, encoder.finish());
        encoder << uniline << array << "abc" << 123 << true << end;
        ASSERT_EQ(R"([ "abc", 123, true ])"s, encoder.finish());
        encoder << nospace << array << "abc" << 123 << true << end;
        ASSERT_EQ(R"(["abc",123,true])"s, encoder.finish());
    }
}

TEST(encode, string)
{
    { // Empty
        Encoder encoder{};
        encoder << "";
        ASSERT_EQ(R"("")"s, encoder.finish());
    }
    { // String view
        Encoder encoder{};
        encoder << "hello"sv;
        ASSERT_EQ(R"("hello")"s, encoder.finish());
    }
    { // String
        Encoder encoder{};
        encoder << "hello"s;
        ASSERT_EQ(R"("hello")"s, encoder.finish());
    }
    { // Const C string
        Encoder encoder{};
        encoder << "hello";
        ASSERT_EQ(R"("hello")"s, encoder.finish());
    }
    { // Mutable C string
        Encoder encoder{};
        encoder << const_cast<char *>("hello");
        ASSERT_EQ(R"("hello")"s, encoder.finish());
    }
    { // Printable characters
        Encoder encoder{};
        const std::string actual{R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"s};
        const std::string expected{R"(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")"s};
        encoder << actual;
        ASSERT_EQ(expected, encoder.finish());
    }
    { // Escape characters
        Encoder encoder{};
        encoder << "\0\b\t\n\v\f\r"sv;
        ASSERT_EQ(R"("\0\b\t\n\v\f\r")"s, encoder.finish());
    }
    { // `\x` code point
        std::string decodeStr(154, '\0');
        std::string expectedStr(1 + 154 * 4 + 1, '\0');
        expectedStr.front() = '"';
        expectedStr.back() = '"';
        unat i{0u};
        for (unat cp{1u}; cp < 8u; ++cp, ++i)
        {
            decodeStr[i] = char(cp);
            std::format_to_n(&expectedStr[1u + 4u * i], 6, "\\x{:02X}"sv, cp);
        }
        for (unat cp{14u}; cp < 32u; ++cp, ++i)
        {
            decodeStr[i] = char(cp);
            std::format_to_n(&expectedStr[1u + 4u * i], 6, "\\x{:02X}"sv, cp);
        }
        for (unat cp{127u}; cp < 256u; ++cp, ++i)
        {
            decodeStr[i] = char(cp);
            std::format_to_n(&expectedStr[1u + 4u * i], 6, "\\x{:02X}"sv, cp);
        }
        Encoder encoder{};
        encoder << decodeStr;
        ASSERT_EQ(expectedStr, encoder.finish());
    }
    { // Single char
        Encoder encoder{};
        encoder << 'a';
        ASSERT_EQ(R"("a")"s, encoder.finish());
    }
    { // Double quotes
        Encoder encoder{uniline, 4u};
        std::string expected{};
        encoder << R"(s"t'r)";
        expected = R"("s\"t'r")"s;
        ASSERT_EQ(expected, encoder.finish());
        encoder << object << R"(""")" << R"(''')" << end;
        expected = R"({ "\"\"\"": "'''" })"s;
        ASSERT_EQ(expected, encoder.finish());
        encoder << object << R"(''')" << R"(""")" << end;
        expected = R"({ "'''": "\"\"\"" })"s;
        ASSERT_EQ(expected, encoder.finish());
    }
}

TEST(encode, signedInteger)
{
    { // Zero
        Encoder encoder{};
        encoder << s64(0);
        ASSERT_EQ(R"(0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << 123;
        ASSERT_EQ(R"(123)"s, encoder.finish());
        encoder << -123;
        ASSERT_EQ(R"(-123)"s, encoder.finish());
    }
    { // Max 64
        Encoder encoder{};
        encoder << std::numeric_limits<s64>::max();
        ASSERT_EQ(R"(9223372036854775807)"s, encoder.finish());
    }
    { // Min 64
        Encoder encoder{};
        encoder << std::numeric_limits<s64>::min();
        ASSERT_EQ(R"(-9223372036854775808)"s, encoder.finish());
    }
    { // Max 32
        Encoder encoder{};
        encoder << std::numeric_limits<s32>::max();
        ASSERT_EQ(R"(2147483647)"s, encoder.finish());
    }
    { // Min 32
        Encoder encoder{};
        encoder << std::numeric_limits<s32>::min();
        ASSERT_EQ(R"(-2147483648)"s, encoder.finish());
    }
    { // Max 16
        Encoder encoder{};
        encoder << std::numeric_limits<s16>::max();
        ASSERT_EQ(R"(32767)"s, encoder.finish());
    }
    { // Min 16
        Encoder encoder{};
        encoder << std::numeric_limits<s16>::min();
        ASSERT_EQ(R"(-32768)"s, encoder.finish());
    }
    { // Max 8
        Encoder encoder{};
        encoder << std::numeric_limits<s8>::max();
        ASSERT_EQ(R"(127)"s, encoder.finish());
    }
    { // Min 8
        Encoder encoder{};
        encoder << std::numeric_limits<s8>::min();
        ASSERT_EQ(R"(-128)"s, encoder.finish());
    }
}

TEST(encode, unsignedInteger)
{
    { // Zero
        Encoder encoder{};
        encoder << 0u;
        ASSERT_EQ(R"(0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << 123u;
        ASSERT_EQ(R"(123)"s, encoder.finish());
    }
    { // Max 64
        Encoder encoder{};
        encoder << std::numeric_limits<u64>::max();
        ASSERT_EQ(R"(18446744073709551615)"s, encoder.finish());
    }
    { // Max 32
        Encoder encoder{};
        encoder << std::numeric_limits<u32>::max();
        ASSERT_EQ(R"(4294967295)"s, encoder.finish());
    }
    { // Max 16
        Encoder encoder{};
        encoder << std::numeric_limits<u16>::max();
        ASSERT_EQ(R"(65535)"s, encoder.finish());
    }
    { // Max 8
        Encoder encoder{};
        encoder << std::numeric_limits<u8>::max();
        ASSERT_EQ(R"(255)"s, encoder.finish());
    }
}

TEST(encode, hex)
{
    { // Zero
        Encoder encoder{};
        encoder << hex << 0x0;
        ASSERT_EQ(R"(0x0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << hex << 0x1A;
        ASSERT_EQ(R"(0x1A)"s, encoder.finish());
        encoder << hex << -0x1A;
        ASSERT_EQ(R"(-0x1A)"s, encoder.finish());
    }
    { // Every digit
        Encoder encoder{};
        encoder << hex << 0xFEDCBA9876543210u;
        ASSERT_EQ("0xFEDCBA9876543210", encoder.finish());
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << hex << std::numeric_limits<u64>::max();
        ASSERT_EQ(R"(0xFFFFFFFFFFFFFFFF)"s, encoder.finish());
    }
    { // Max signed
        Encoder encoder{};
        encoder << hex << std::numeric_limits<s64>::max();
        ASSERT_EQ(R"(0x7FFFFFFFFFFFFFFF)"s, encoder.finish());
    }
    { // Min signed
        Encoder encoder{};
        encoder << hex << std::numeric_limits<s64>::min();
        ASSERT_EQ(R"(-0x8000000000000000)"s, encoder.finish());
    }
}

TEST(encode, octal)
{
    { // Zero
        Encoder encoder{};
        encoder << octal << 00;
        ASSERT_EQ(R"(0o0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << octal << 012;
        ASSERT_EQ(R"(0o12)"s, encoder.finish());
        encoder << octal << -012;
        ASSERT_EQ(R"(-0o12)"s, encoder.finish());
    }
    { // Every digit
        Encoder encoder{};
        encoder << octal << 076543210u;
        ASSERT_EQ("0o76543210", encoder.finish());
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << octal << std::numeric_limits<u64>::max();
        ASSERT_EQ(R"(0o1777777777777777777777)"s, encoder.finish());
    }
    { // Max signed
        Encoder encoder{};
        encoder << octal << std::numeric_limits<s64>::max();
        ASSERT_EQ(R"(0o777777777777777777777)"s, encoder.finish());
    }
    { // Min signed
        Encoder encoder{};
        encoder << octal << std::numeric_limits<s64>::min();
        ASSERT_EQ(R"(-0o1000000000000000000000)"s, encoder.finish());
    }
}

TEST(encode, binary)
{
    { // Zero
        Encoder encoder{};
        encoder << binary << 0b0;
        ASSERT_EQ(R"(0b0)"s, encoder.finish());
    }
    { // One
        Encoder encoder{};
        encoder << binary << 0b1;
        ASSERT_EQ(R"(0b1)"s, encoder.finish());
        encoder << binary << -0b1;
        ASSERT_EQ(R"(-0b1)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << binary << 0b101;
        ASSERT_EQ(R"(0b101)"s, encoder.finish());
        encoder << binary << -0b101;
        ASSERT_EQ(R"(-0b101)"s, encoder.finish());
    }
    { // Non multiple of 4 lengths
        Encoder encoder{};
        encoder << binary << 0b1100'0011;
        ASSERT_EQ(R"(0b11000011)"s, encoder.finish());
        encoder << binary << 0b1'1100'0011;
        ASSERT_EQ(R"(0b111000011)"s, encoder.finish());
        encoder << binary << 0b11'1100'0011;
        ASSERT_EQ(R"(0b1111000011)"s, encoder.finish());
        encoder << binary << 0b111'1100'0011;
        ASSERT_EQ(R"(0b11111000011)"s, encoder.finish());
        encoder << binary << 0b1111'1100'0011;
        ASSERT_EQ(R"(0b111111000011)"s, encoder.finish());
    }
    { // Every 4 length permutation
        Encoder encoder{};
        encoder << binary << 0b0001'0010'0011'0000'0100'0101'0110'0111'1000'1001'1010'1011'1100'1101'1110'1111u;
        ASSERT_EQ(R"(0b1001000110000010001010110011110001001101010111100110111101111)"s, encoder.finish());
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << binary << std::numeric_limits<u64>::max();
        ASSERT_EQ(R"(0b1111111111111111111111111111111111111111111111111111111111111111)"s, encoder.finish());
    }
    { // Max signed
        Encoder encoder{};
        encoder << binary << std::numeric_limits<s64>::max();
        ASSERT_EQ(R"(0b111111111111111111111111111111111111111111111111111111111111111)"s, encoder.finish());
    }
    { // Min signed
        Encoder encoder{};
        encoder << binary << std::numeric_limits<s64>::min();
        ASSERT_EQ(R"(-0b1000000000000000000000000000000000000000000000000000000000000000)"s, encoder.finish());
    }
}

TEST(encode, floater)
{
    { // Zero
        Encoder encoder{};
        encoder << 0.0;
        ASSERT_EQ(R"(0.0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << 123.45;
        ASSERT_EQ(R"(123.45)"s, encoder.finish());
    }
    { // Max integer 64
        Encoder encoder{};
        u64 val{0b0'10000110011'1111111111111111111111111111111111111111111111111111u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(9007199254740991.0)"s, encoder.finish());
    }
    { // Max integer 32
        Encoder encoder{};
        u32 val{0b0'10010110'11111111111111111111111u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(R"(16777215.0)"s, encoder.finish());
    }
    { // Max 64
        Encoder encoder{};
        u64 val{0b0'11111111110'1111111111111111111111111111111111111111111111111111u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(1.7976931348623157e+308)"s, encoder.finish());
    }
    { // Max 32
        Encoder encoder{};
        u32 val{0b0'11111110'11111111111111111111111u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(R"(3.4028234663852886e+38)"s, encoder.finish());
    }
    { // Min normal 64
        Encoder encoder{};
        u64 val{0b0'00000000001'0000000000000000000000000000000000000000000000000000u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(2.2250738585072014e-308)"s, encoder.finish());
    }
    { // Min normal 32
        Encoder encoder{};
        u32 val{0b0'00000001'00000000000000000000000u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(R"(1.1754943508222875e-38)"s, encoder.finish());
    }
    { // Min subnormal 64
        Encoder encoder{};
        u64 val{0b0'00000000000'0000000000000000000000000000000000000000000000000001u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(5e-324)"s, encoder.finish());
    }
    { // Min subnormal 32
        Encoder encoder{};
        u64 val{0b0'00000000'00000000000000000000001u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(R"(1.401298464324817e-45)"s, encoder.finish());
    }
    { // Positive infinity
        Encoder encoder{};
        encoder << std::numeric_limits<double>::infinity();
        ASSERT_EQ(R"(inf)"s, encoder.finish());
    }
    { // Negative infinity
        Encoder encoder{};
        encoder << -std::numeric_limits<double>::infinity();
        ASSERT_EQ(R"(-inf)"s, encoder.finish());
    }
    { // NaN
        Encoder encoder{};
        encoder << std::numeric_limits<double>::quiet_NaN();
        ASSERT_EQ(R"(nan)"s, encoder.finish());
    }
    { // Negative NaN
        Encoder encoder{};
        encoder << -std::numeric_limits<double>::quiet_NaN();
        ASSERT_EQ(R"(nan)"s, encoder.finish());
    }
}

TEST(encode, boolean)
{
    { // True
        Encoder encoder{};
        encoder << true;
        ASSERT_EQ(R"(true)"s, encoder.finish());
    }
    { // False
        Encoder encoder{};
        encoder << false;
        ASSERT_EQ(R"(false)"s, encoder.finish());
    }
}

TEST(encode, null)
{
    Encoder encoder{};
    encoder << nullptr;
    ASSERT_EQ(R"(null)"s, encoder.finish());
}

TEST(encode, datetime)
{
    { // Epoch
        Encoder encoder{};
        encoder << utcOffset << Datetime{};
        ASSERT_EQ("D1969-12-31T16:00:00-08:00", encoder.finish());
        encoder << utc << Datetime{};
        ASSERT_EQ("D1970-01-01T00:00:00Z", encoder.finish());
        encoder << localTime << Datetime{};
        ASSERT_EQ("D1969-12-31T16:00:00", encoder.finish());
    }
    { // Positive timestamp
        const Datetime tp{std::chrono::seconds{1676337198} + std::chrono::microseconds{123456}};
        Encoder encoder{};
        encoder << utcOffset << tp;
        ASSERT_EQ("D2023-02-13T17:13:18.123456-08:00", encoder.finish());
        encoder << utc << tp;
        ASSERT_EQ("D2023-02-14T01:13:18.123456Z", encoder.finish());
        encoder << localTime << tp;
        ASSERT_EQ("D2023-02-13T17:13:18.123456", encoder.finish());
    }
    { // Negative timestamp
        const Datetime tp{std::chrono::seconds{-777777777} + std::chrono::microseconds{142536}};
        Encoder encoder{};
        encoder << utcOffset << tp;
        ASSERT_EQ("D1945-05-09T15:37:03.142536-07:00", encoder.finish());
        encoder << utc << tp;
        ASSERT_EQ("D1945-05-09T22:37:03.142536Z", encoder.finish());
        encoder << localTime << tp;
        ASSERT_EQ("D1945-05-09T15:37:03.142536", encoder.finish());
    }
    { // Future timestamp
        const Datetime tp{std::chrono::seconds{253402300799}};
        Encoder encoder{};
        encoder << utc << tp;
        ASSERT_EQ("D9999-12-31T23:59:59Z", encoder.finish());
    }
    { // Past timestamp
        const Datetime tp{std::chrono::seconds{-62167219200}};
        Encoder encoder{};
        encoder << utc << tp;
        ASSERT_EQ("D0000-01-01T00:00:00Z", encoder.finish());
    }
    { // Too far in the future
        Encoder encoder{};
        encoder << utc << Datetime{std::chrono::seconds{253402300800}};
        ASSERT_FALSE(encoder.status());
    }
    { // Too far in the past
        Encoder encoder{};
        encoder << utc << Datetime{std::chrono::seconds{-62167219201}};
        ASSERT_FALSE(encoder.status());
    }
    { // utcOffset and local match
        Encoder encoder{};
        encoder << utcOffset << std::chrono::system_clock::now();
        const std::string s1{*encoder.finish()};
        encoder << localTime << std::chrono::system_clock::now();
        const std::string s2{*encoder.finish()};
        ASSERT_EQ(s1.substr(0u, 20u), s2.substr(0u, 20u));
    }
    { // Fractional seconds
        static_assert(std::chrono::system_clock::duration::period::den == 10'000'000);
        Encoder encoder{};
        encoder << utc << Datetime{std::chrono::system_clock::duration{1'000'000}};
        ASSERT_EQ("D1970-01-01T00:00:00.1Z", encoder.finish());
        encoder << utc << Datetime{std::chrono::system_clock::duration{100'000}};
        ASSERT_EQ("D1970-01-01T00:00:00.01Z", encoder.finish());
        encoder << utc << Datetime{std::chrono::system_clock::duration{10'000}};
        ASSERT_EQ("D1970-01-01T00:00:00.001Z", encoder.finish());
        encoder << utc << Datetime{std::chrono::system_clock::duration{1'000}};
        ASSERT_EQ("D1970-01-01T00:00:00.0001Z", encoder.finish());
        encoder << utc << Datetime{std::chrono::system_clock::duration{100}};
        ASSERT_EQ("D1970-01-01T00:00:00.00001Z", encoder.finish());
        encoder << utc << Datetime{std::chrono::system_clock::duration{10}};
        ASSERT_EQ("D1970-01-01T00:00:00.000001Z", encoder.finish());
        encoder << utc << Datetime{std::chrono::system_clock::duration{1}};
        ASSERT_EQ("D1970-01-01T00:00:00.0000001Z", encoder.finish());
    }
}

TEST(encode, custom)
{
    Encoder encoder{};
    encoder << CustomVal{1, 2};
    ASSERT_EQ(R"([ 1, 2 ])"s, encoder.finish());
}

TEST(encode, reset)
{
    {
        Encoder encoder{};
        ASSERT_TRUE(encoder.status());
        encoder << end;
        ASSERT_FALSE(encoder.status());
        encoder.reset();
        ASSERT_TRUE(encoder.status());
    }
    {
        Encoder encoder{};
        encoder << nullptr;
        ASSERT_TRUE(encoder.status());
        encoder.reset();
        ASSERT_TRUE(encoder.status());
        encoder << true;
        ASSERT_EQ(R"(true)"s, encoder.finish());
    }
}

TEST(encode, finish)
{
    { // Encoder left in clean state after finish
        Encoder encoder{uniline};
        encoder << object << "val" << 123 << end;
        ASSERT_EQ(R"({ "val": 123 })"s, encoder.finish());
        encoder << array << 321 << end;
        ASSERT_EQ(R"([ 321 ])"s, encoder.finish());
    }
    { // Finishing at root
        Encoder encoder{};
        ASSERT_FALSE(encoder.finish().has_value());
    }
    { // Finishing in object
        Encoder encoder{};
        encoder << object;
        ASSERT_TRUE(encoder.status());
        ASSERT_FALSE(encoder.finish().has_value());
    }
    { // Finishing in array
        Encoder encoder{};
        encoder << array;
        ASSERT_TRUE(encoder.status());
        ASSERT_FALSE(encoder.finish().has_value());
    }
}

TEST(encode, density)
{
    { // Top level multiline
        Encoder encoder{multiline};
        encoder << object << "k1" << array << "v1" << "v2" << end << "k2" << "v3" << end;
        ASSERT_EQ(R"({
    "k1": [
        "v1",
        "v2"
    ],
    "k2": "v3"
})"s, encoder.finish());
    }
    { // Top level uniline
        Encoder encoder{uniline};
        encoder << object << "k1" << array << "v1" << "v2" << end << "k2" << "v3" << end;
        ASSERT_EQ(R"({ "k1": [ "v1", "v2" ], "k2": "v3" })"s, encoder.finish());
    }
    { // Top level nospace
        Encoder encoder{nospace};
        encoder << object << "k1" << array << "v1" << "v2" << end << "k2" << "v3" << end;
        ASSERT_EQ(R"({"k1":["v1","v2"],"k2":"v3"})"s, encoder.finish());
    }
    { // Inner density
        Encoder encoder{};
        encoder << object;
            encoder << "k1" << uniline << array << "v1" << nospace << array << "v2" << "v3" << end << end;
            encoder << "k2" << uniline << object << "k3" << "v4" << "k4" << nospace << object << "k5" << "v5" << "k6" << "v6" << end << end;
        encoder << end;
        ASSERT_EQ(R"({
    "k1": [ "v1", ["v2","v3"] ],
    "k2": { "k3": "v4", "k4": {"k5":"v5","k6":"v6"} }
})"s, encoder.finish());
    }
    { // Density priority
        Encoder encoder{};
        encoder << uniline << object << "k" << multiline << array << "v" << end << end;
        ASSERT_EQ(R"({ "k": [ "v" ] })"s, encoder.finish());
        encoder << uniline << array << multiline << object << "k" << "v" << end << end;
        ASSERT_EQ(R"([ { "k": "v" } ])"s, encoder.finish());
        encoder << nospace << object << "k" << uniline << array << "v" << end << end;
        ASSERT_EQ(R"({"k":["v"]})"s, encoder.finish());
        encoder << nospace << array << uniline << object << "k" << "v" << end << end;
        ASSERT_EQ(R"([{"k":"v"}])"s, encoder.finish());
    }
}

TEST(encode, indentSpaces)
{
    { // 0
        Encoder encoder{multiline, 0u};
        encoder << object;
            encoder << "k" << array;
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(R"({
"k": [
"v"
]
})", encoder.finish());
    }
    { // 1
        Encoder encoder{multiline, 1u};
        encoder << object;
            encoder << "k" << array;
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(R"({
 "k": [
  "v"
 ]
})", encoder.finish());
    }
    { // 7
        Encoder encoder{multiline, 7u};
        encoder << object;
            encoder << "k" << array;
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(R"({
       "k": [
              "v"
       ]
})", encoder.finish());
    }
}

TEST(encode, flagTokens)
{
    { // Density
        Encoder encoder{};

        // Mutliple densities
        encoder << nospace << uniline << array << 0 << end;
        ASSERT_EQ("[ 0 ]", encoder.finish());

        // Density is transient
        encoder << uniline << array << nospace << array << 0 << end << array << 0 << end << end;
        ASSERT_EQ("[ [0], [ 0 ] ]", encoder.finish());

        // End after density
        encoder << array << nospace << end;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // String after density
        encoder << nospace << "ok";
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Integer after density
        encoder << nospace << 0;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Floater after density
        encoder << nospace << 0.0;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Boolean after density
        encoder << nospace << true;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Null after density
        encoder << nospace << nullptr;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Datetime after density
        encoder << utc << nospace << Datetime{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Base after density
        encoder << nospace << binary << 0;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Time zone format after density
        encoder << nospace << utc;
        ASSERT_FALSE(encoder.status());
        encoder.reset();
    }
    { // Base
        Encoder encoder{};

        // Mutliple bases
        encoder << hex << binary << 0;
        ASSERT_EQ("0b0", encoder.finish());

        // Object after base
        encoder << hex << object << end;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Array after base
        encoder << hex << array << end;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // String after base
        encoder << hex << "ok";
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Floater after base
        encoder << hex << 0.0;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Boolean after base
        encoder << hex << true;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Null after base
        encoder << hex << nullptr;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Density after base
        encoder << hex << nospace;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Time zone format after base
        encoder << hex << utc;
        ASSERT_FALSE(encoder.status());
        encoder.reset();
    }
    { // Time zone format
        Encoder encoder{};

        const Datetime tp{};

        // Mutliple time zone formats
        encoder << localTime << utc << tp;
        ASSERT_EQ("D1970-01-01T00:00:00Z", encoder.finish());

        // Object after time zone format
        encoder << utc << object << end;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Array after time zone format
        encoder << utc << array << end;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // String after time zone format
        encoder << utc << "ok";
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Floater after time zone format
        encoder << utc << 0.0;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Boolean after time zone format
        encoder << utc << true;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Null after time zone formatse
        encoder << utc << nullptr;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Density after time zone format
        encoder << utc << nospace;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // bASE after time zone format
        encoder << utc << hex;
        ASSERT_FALSE(encoder.status());
        encoder.reset();
    }
}

TEST(encode, misc)
{
    { // Extraneous content
        Encoder encoder{};
        encoder << "a";
        ASSERT_TRUE(encoder.status());
        encoder << "b";
        ASSERT_FALSE(encoder.status());
    }
}

TEST(encode, general)
{
    Encoder encoder{};
    encoder << object;
        encoder << "Name"sv << "Salt's Crust"sv;
        encoder << "Founded"sv << utc << Datetime{std::chrono::seconds{-182772049}};
        encoder << "Employees"sv << array;
            encoder << uniline << object << "Name"sv << "Ol' Joe Fisher"sv << "Title"sv << "Fisherman"sv << "Age"sv << 69 << end;
            encoder << uniline << object << "Name"sv << "Mark Rower"sv << "Title"sv << "Cook"sv << "Age"sv << 41 << end;
            encoder << uniline << object << "Name"sv << "Phineas"sv << "Title"sv << "Server Boy"sv << "Age"sv << 19 << end;
        encoder << end;
        encoder << "Dishes"sv << array;
            encoder << object;
                encoder << "Name"sv << "Basket o' Barnacles"sv;
                encoder << "Price"sv << 5.45;
                encoder << "Ingredients"sv << uniline << array << "\"Salt\""sv << "Barnacles"sv << end;
                encoder << "Gluten Free"sv << false;
            encoder << end;
            encoder << object;
                encoder << "Name"sv << "Two Tuna"sv;
                encoder << "Price"sv << -std::numeric_limits<double>::infinity();
                encoder << "Ingredients"sv << uniline << array << "Tuna"sv << end;
                encoder << "Gluten Free"sv << true;
            encoder << end;
            encoder << object;
                encoder << "Name"sv << "18 Leg Bouquet"sv;
                encoder << "Price"sv << std::numeric_limits<double>::quiet_NaN();
                encoder << "Ingredients"sv << uniline << array << "\"Salt\""sv << "Octopus"sv << "Crab"sv << end;
                encoder << "Gluten Free"sv << false;
            encoder << end;
        encoder << end;
        encoder << "Profit Margin"sv << nullptr;
        encoder << "Ha\x03r Name"sv << "M\0\0n"sv;
        encoder << "Green Eggs and Ham"sv <<
R"(I do not like them in a box
I do not like them with a fox
I do not like them in a house
I do not like them with a mouse
I do not like them here or there
I do not like them anywhere
I do not like green eggs and ham
I do not like them Sam I am
)";
        encoder << "Magic Numbers"sv << nospace << array << hex << 777 << octal << 777u << binary << 777 << end;
    encoder << end;

    ASSERT_EQ(R"({
    "Name": "Salt's Crust",
    "Founded": D1964-03-17T13:59:11Z,
    "Employees": [
        { "Name": "Ol' Joe Fisher", "Title": "Fisherman", "Age": 69 },
        { "Name": "Mark Rower", "Title": "Cook", "Age": 41 },
        { "Name": "Phineas", "Title": "Server Boy", "Age": 19 }
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
            "Ingredients": [ "Tuna" ],
            "Gluten Free": true
        },
        {
            "Name": "18 Leg Bouquet",
            "Price": nan,
            "Ingredients": [ "\"Salt\"", "Octopus", "Crab" ],
            "Gluten Free": false
        }
    ],
    "Profit Margin": null,
    "Ha\x03r Name": "M\0\0n",
    "Green Eggs and Ham": "I do not like them in a box\nI do not like them with a fox\nI do not like them in a house\nI do not like them with a mouse\nI do not like them here or there\nI do not like them anywhere\nI do not like green eggs and ham\nI do not like them Sam I am\n",
    "Magic Numbers": [0x309,0o1411,0b1100001001]
})"s, encoder.finish());
}
