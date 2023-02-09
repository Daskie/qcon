#include <format>

#include <gtest/gtest.h>

#include <qcon-encode.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;

using qcon::Encoder;
using namespace qcon::tokens;
using qcon::Density;

struct CustomVal { int x, y; };

Encoder & operator<<(Encoder & encoder, const CustomVal & v)
{
    return encoder << array(Density::uniline) << v.x << v.y << end;
}

TEST(encode, object)
{
    { // Empty
        Encoder encoder{};
        encoder << object(Density::multiline) << end;
        ASSERT_EQ(R"({})"s, encoder.finish());
        encoder << object(Density::uniline) << end;
        ASSERT_EQ(R"({})"s, encoder.finish());
        encoder << object(Density::nospace) << end;
        ASSERT_EQ(R"({})"s, encoder.finish());
    }
    { // Non-empty
        Encoder encoder{};
        encoder << object(Density::multiline) << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(R"({
    "k1": "abc",
    "k2": 123,
    "k3": true
})"s, encoder.finish());
        encoder << object(Density::uniline) << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(R"({ "k1": "abc", "k2": 123, "k3": true })"s, encoder.finish());
        encoder << object(Density::nospace) << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(R"({"k1":"abc","k2":123,"k3":true})"s, encoder.finish());
    }
    { // String view key
        Encoder encoder{Density::uniline};
        encoder << object << "k"sv << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // String key
        Encoder encoder{Density::uniline};
        encoder << object << "k"s << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Const C string key
        Encoder encoder{Density::uniline};
        encoder << object << "k" << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Mutable C string key
        Encoder encoder{Density::uniline};
        encoder << object << const_cast<char *>("k") << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Character key
        Encoder encoder{Density::uniline};
        encoder << object << 'k' << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Empty key
        Encoder encoder{Density::uniline};
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
        encoder << array(Density::multiline) << end;
        ASSERT_EQ(R"([])"s, encoder.finish());
        encoder << array(Density::uniline) << end;
        ASSERT_EQ(R"([])"s, encoder.finish());
        encoder << array(Density::nospace) << end;
        ASSERT_EQ(R"([])"s, encoder.finish());
    }
    { // Non-empty
        Encoder encoder{};
        encoder << array(Density::multiline) << "abc" << 123 << true << end;
        ASSERT_EQ(R"([
    "abc",
    123,
    true
])"s, encoder.finish());
        encoder << array(Density::uniline) << "abc" << 123 << true << end;
        ASSERT_EQ(R"([ "abc", 123, true ])"s, encoder.finish());
        encoder << array(Density::nospace) << "abc" << 123 << true << end;
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
        size_t i{0u};
        for (size_t cp{1u}; cp < 8u; ++cp, ++i)
        {
            decodeStr[i] = char(cp);
            std::format_to_n(&expectedStr[1u + 4u * i], 6, "\\x{:02X}"sv, cp);
        }
        for (size_t cp{14u}; cp < 32u; ++cp, ++i)
        {
            decodeStr[i] = char(cp);
            std::format_to_n(&expectedStr[1u + 4u * i], 6, "\\x{:02X}"sv, cp);
        }
        for (size_t cp{127u}; cp < 256u; ++cp, ++i)
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
        Encoder encoder{Density::uniline, 4u, false};
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
    { // Single quotes
        Encoder encoder{Density::uniline, 4u, true};
        std::string expected{};
        encoder << R"(s"t'r)";
        expected = R"('s"t\'r')"s;
        ASSERT_EQ(expected, encoder.finish());
        encoder << object << R"(""")" << R"(''')" << end;
        expected = R"({ '"""': '\'\'\'' })"s;
        ASSERT_EQ(expected, encoder.finish());
        encoder << object << R"(''')" << R"(""")" << end;
        expected = R"({ '\'\'\'': '"""' })"s;
        ASSERT_EQ(expected, encoder.finish());
    }
}

TEST(encode, signedInteger)
{
    { // Zero
        Encoder encoder{};
        encoder << int64_t(0);
        ASSERT_EQ(R"(0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << 123;
        ASSERT_EQ(R"(123)"s, encoder.finish());
    }
    { // Max 64
        Encoder encoder{};
        encoder << std::numeric_limits<int64_t>::max();
        ASSERT_EQ(R"(9223372036854775807)"s, encoder.finish());
    }
    { // Min 64
        Encoder encoder{};
        encoder << std::numeric_limits<int64_t>::min();
        ASSERT_EQ(R"(-9223372036854775808)"s, encoder.finish());
    }
    { // Max 32
        Encoder encoder{};
        encoder << std::numeric_limits<int32_t>::max();
        ASSERT_EQ(R"(2147483647)"s, encoder.finish());
    }
    { // Min 32
        Encoder encoder{};
        encoder << std::numeric_limits<int32_t>::min();
        ASSERT_EQ(R"(-2147483648)"s, encoder.finish());
    }
    { // Max 16
        Encoder encoder{};
        encoder << std::numeric_limits<int16_t>::max();
        ASSERT_EQ(R"(32767)"s, encoder.finish());
    }
    { // Min 16
        Encoder encoder{};
        encoder << std::numeric_limits<int16_t>::min();
        ASSERT_EQ(R"(-32768)"s, encoder.finish());
    }
    { // Max 8
        Encoder encoder{};
        encoder << std::numeric_limits<int8_t>::max();
        ASSERT_EQ(R"(127)"s, encoder.finish());
    }
    { // Min 8
        Encoder encoder{};
        encoder << std::numeric_limits<int8_t>::min();
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
        encoder << std::numeric_limits<uint64_t>::max();
        ASSERT_EQ(R"(18446744073709551615)"s, encoder.finish());
    }
    { // Max 32
        Encoder encoder{};
        encoder << std::numeric_limits<uint32_t>::max();
        ASSERT_EQ(R"(4294967295)"s, encoder.finish());
    }
    { // Max 16
        Encoder encoder{};
        encoder << std::numeric_limits<uint16_t>::max();
        ASSERT_EQ(R"(65535)"s, encoder.finish());
    }
    { // Max 8
        Encoder encoder{};
        encoder << std::numeric_limits<uint8_t>::max();
        ASSERT_EQ(R"(255)"s, encoder.finish());
    }
}

TEST(encode, hex)
{
    { // Zero
        Encoder encoder{};
        encoder << hex(0u);
        ASSERT_EQ(R"(0x0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << hex(26u);
        ASSERT_EQ(R"(0x1A)"s, encoder.finish());
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << hex(std::numeric_limits<uint64_t>::max());
        ASSERT_EQ(R"(0xFFFFFFFFFFFFFFFF)"s, encoder.finish());
    }
    { // Min signed
        Encoder encoder{};
        encoder << hex(uint64_t(std::numeric_limits<int64_t>::min()));
        ASSERT_EQ(R"(0x8000000000000000)"s, encoder.finish());
    }
    { // -1
        Encoder encoder{};
        #pragma warning(suppress: 4245)
        encoder << hex(uint64_t(int64_t(-1)));
        ASSERT_EQ(R"(0xFFFFFFFFFFFFFFFF)"s, encoder.finish());
    }
}

TEST(encode, octal)
{
    { // Zero
        Encoder encoder{};
        encoder << octal(0u);
        ASSERT_EQ(R"(0o0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << octal(10u);
        ASSERT_EQ(R"(0o12)"s, encoder.finish());
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << octal(std::numeric_limits<uint64_t>::max());
        ASSERT_EQ(R"(0o1777777777777777777777)"s, encoder.finish());
    }
    { // Min signed
        Encoder encoder{};
        encoder << octal(uint64_t(std::numeric_limits<int64_t>::min()));
        ASSERT_EQ(R"(0o1000000000000000000000)"s, encoder.finish());
    }
    { // -1
        Encoder encoder{};
        #pragma warning(suppress: 4245)
        encoder << octal(uint64_t(int64_t(-1)));
        ASSERT_EQ(R"(0o1777777777777777777777)"s, encoder.finish());
    }
}

TEST(encode, binary)
{
    { // Zero
        Encoder encoder{};
        encoder << binary(0u);
        ASSERT_EQ(R"(0b0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << binary(5u);
        ASSERT_EQ(R"(0b101)"s, encoder.finish());
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << binary(std::numeric_limits<uint64_t>::max());
        ASSERT_EQ(R"(0b1111111111111111111111111111111111111111111111111111111111111111)"s, encoder.finish());
    }
    { // Min signed
        Encoder encoder{};
        encoder << binary(uint64_t(std::numeric_limits<int64_t>::min()));
        ASSERT_EQ(R"(0b1000000000000000000000000000000000000000000000000000000000000000)"s, encoder.finish());
    }
    { // -1
        Encoder encoder{};
        #pragma warning(suppress: 4245)
        encoder << binary(uint64_t(int64_t(-1)));
        ASSERT_EQ(R"(0b1111111111111111111111111111111111111111111111111111111111111111)"s, encoder.finish());
    }
}

TEST(encode, floater)
{
    { // Zero
        Encoder encoder{};
        encoder << 0.0;
        ASSERT_EQ(R"(0)"s, encoder.finish());
    }
    { // Typical
        Encoder encoder{};
        encoder << 123.45;
        ASSERT_EQ(R"(123.45)"s, encoder.finish());
    }
    { // Max integer 64
        Encoder encoder{};
        uint64_t val{0b0'10000110011'1111111111111111111111111111111111111111111111111111u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(9007199254740991)"s, encoder.finish());
    }
    { // Max integer 32
        Encoder encoder{};
        uint32_t val{0b0'10010110'11111111111111111111111u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(R"(16777215)"s, encoder.finish());
    }
    { // Max 64
        Encoder encoder{};
        uint64_t val{0b0'11111111110'1111111111111111111111111111111111111111111111111111u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(1.7976931348623157e+308)"s, encoder.finish());
    }
    { // Max 32
        Encoder encoder{};
        uint32_t val{0b0'11111110'11111111111111111111111u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(R"(3.4028234663852886e+38)"s, encoder.finish());
    }
    { // Min normal 64
        Encoder encoder{};
        uint64_t val{0b0'00000000001'0000000000000000000000000000000000000000000000000000u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(2.2250738585072014e-308)"s, encoder.finish());
    }
    { // Min normal 32
        Encoder encoder{};
        uint32_t val{0b0'00000001'00000000000000000000000u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(R"(1.1754943508222875e-38)"s, encoder.finish());
    }
    { // Min subnormal 64
        Encoder encoder{};
        uint64_t val{0b0'00000000000'0000000000000000000000000000000000000000000000000001u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(R"(5e-324)"s, encoder.finish());
    }
    { // Min subnormal 32
        Encoder encoder{};
        uint64_t val{0b0'00000000'00000000000000000000001u};
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
        Encoder encoder{Density::uniline};
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
        Encoder encoder{Density::multiline};
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
        Encoder encoder{Density::uniline};
        encoder << object << "k1" << array << "v1" << "v2" << end << "k2" << "v3" << end;
        ASSERT_EQ(R"({ "k1": [ "v1", "v2" ], "k2": "v3" })"s, encoder.finish());
    }
    { // Top level nospace
        Encoder encoder{Density::nospace};
        encoder << object << "k1" << array << "v1" << "v2" << end << "k2" << "v3" << end;
        ASSERT_EQ(R"({"k1":["v1","v2"],"k2":"v3"})"s, encoder.finish());
    }
    { // Inner density
        Encoder encoder{};
        encoder << object;
            encoder << "k1" << array(Density::uniline) << "v1" << array(Density::nospace) << "v2" << "v3" << end << end;
            encoder << "k2" << object(Density::uniline) << "k3" << "v4" << "k4" << object(Density::nospace) << "k5" << "v5" << "k6" << "v6" << end << end;
        encoder << end;
        ASSERT_EQ(R"({
    "k1": [ "v1", ["v2","v3"] ],
    "k2": { "k3": "v4", "k4": {"k5":"v5","k6":"v6"} }
})"s, encoder.finish());
    }
    { // Density priority
        Encoder encoder{};
        encoder << object(Density::uniline) << "k" << array(Density::multiline) << "v" << end << end;
        ASSERT_EQ(R"({ "k": [ "v" ] })"s, encoder.finish());
        encoder << array(Density::uniline) << object(Density::multiline) << "k" << "v" << end << end;
        ASSERT_EQ(R"([ { "k": "v" } ])"s, encoder.finish());
        encoder << object(Density::nospace) << "k" << array(Density::uniline) << "v" << end << end;
        ASSERT_EQ(R"({"k":["v"]})"s, encoder.finish());
        encoder << array(Density::nospace) << object(Density::uniline) << "k" << "v" << end << end;
        ASSERT_EQ(R"([{"k":"v"}])"s, encoder.finish());
    }
}

TEST(encode, identifiers)
{
    { // Valid identifiers
        Encoder encoder{Density::uniline, 4u, false, true};
        encoder << object << "a" << "v" << end;
        ASSERT_EQ(R"({ a: "v" })"s, encoder.finish());
        encoder << object << "A" << "v" << end;
        ASSERT_EQ(R"({ A: "v" })"s, encoder.finish());
        encoder << object << "0" << "v" << end;
        ASSERT_EQ(R"({ 0: "v" })"s, encoder.finish());
        encoder << object << "_" << "v" << end;
        ASSERT_EQ(R"({ _: "v" })"s, encoder.finish());
        encoder << object << "_0a" << "v" << end;
        ASSERT_EQ(R"({ _0a: "v" })"s, encoder.finish());
        encoder << object << "_0a" << "v" << end;
        ASSERT_EQ(R"({ _0a: "v" })"s, encoder.finish());
    }
    { // Invalid identifiers
        Encoder encoder{Density::uniline, 4u, false, true};
        encoder << object << "w o a" << "v" << end;
        ASSERT_EQ(R"({ "w o a": "v" })"s, encoder.finish());
    }
    { // Preference off
        Encoder encoder{Density::uniline};
        encoder << object << "k" << "v" << end;
        ASSERT_EQ(R"({ "k": "v" })"s, encoder.finish());
    }
    { // Empty key
        Encoder encoder{Density::uniline, 4u, false, true};
        encoder << object;
        ASSERT_TRUE(encoder.status());
        encoder << "";
        ASSERT_FALSE(encoder.status());
    }
}

TEST(encode, comments)
{
    { // Simple one-line comments
        Encoder encoder{Density::multiline};
        const std::string_view str{"A comment"};
        encoder << comment(str) << comment(str) << 0 << comment(str) << comment(str);
        ASSERT_EQ(
R"(// A comment
// A comment
0,
// A comment
// A comment)"s, encoder.finish());
    }
    { // Simple multi-line comments
        Encoder encoder{Density::multiline};
        const std::string_view str{"A comment\nand some more"};
        encoder << comment(str) << comment(str) << 0 << comment(str) << comment(str);
        ASSERT_EQ(
R"(// A comment
// and some more
// A comment
// and some more
0,
// A comment
// and some more
// A comment
// and some more)"s, encoder.finish());
    }
    { // Complex
        Encoder encoder{Density::multiline};
        const std::string_view single{"A comment"};
        const std::string_view multi{"A comment\nand some more"};
        encoder << comment(single);
        encoder << array;
            encoder << comment(multi);
            encoder << comment(single);
            encoder << 0;
            encoder << comment(multi);
            encoder << 1;
            encoder << object;
                encoder << comment(single);
                encoder << "a" << comment(single) << 0;
                encoder << comment(multi);
                encoder << "b" << comment(multi) << 1;
                encoder << comment(multi);
                encoder << comment(multi);
                encoder << "c" << comment(multi) << comment(multi) << 2;
                encoder << "d" << object(Density::uniline) << comment(single) << "k1" << comment(single) << comment(multi) << 0 << comment(single) << comment(multi) << "k2" << array << comment(single) << comment(multi) << end << comment(multi) << end;
                encoder << "e" << object(Density::nospace) << comment(single) << "k1" << comment(single) << comment(multi) << 0 << comment(single) << comment(multi) << "k2" << array << comment(single) << comment(multi) << end << comment(multi) << end;
                encoder << "f" << array;
                    encoder << comment(single);
                encoder << end;
                encoder << "g" << array;
                    encoder << comment(multi);
                encoder << end;
                encoder << comment(single);
                encoder << comment(multi);
            encoder << end;
        encoder << comment(single);
        encoder << end;
    encoder << comment(multi);
        ASSERT_EQ(
R"(// A comment
[
    // A comment
    // and some more
    // A comment
    0,
    // A comment
    // and some more
    1,
    {
        // A comment
        "a": /* A comment */ 0,
        // A comment
        // and some more
        "b": /* A comment
and some more */ 1,
        // A comment
        // and some more
        // A comment
        // and some more
        "c": /* A comment
and some more */ /* A comment
and some more */ 2,
        "d": { /* A comment */ "k1": /* A comment */ /* A comment
and some more */ 0, /* A comment */ /* A comment
and some more */ "k2": [ /* A comment */ /* A comment
and some more */ ], /* A comment
and some more */ },
        "e": {/*A comment*/"k1":/*A comment*//*A comment
and some more*/0,/*A comment*//*A comment
and some more*/"k2":[/*A comment*//*A comment
and some more*/],/*A comment
and some more*/},
        "f": [
            // A comment
        ],
        "g": [
            // A comment
            // and some more
        ],
        // A comment
        // A comment
        // and some more
    },
    // A comment
],
// A comment
// and some more)"s, encoder.finish());
    }
    { // Escape sequence in block comment
        Encoder encoder{Density::uniline};
        encoder << comment("A comment /* and some more") << nullptr;
        ASSERT_EQ(R"(/* A comment /* and some more */ null)", encoder.finish());
        encoder << comment("A comment */ and some more");
        ASSERT_FALSE(encoder.status());
    }
    { // Escape sequence in line comment
        Encoder encoder{Density::multiline};
        encoder << comment("A comment /* and some more") << nullptr;
        ASSERT_EQ(
R"(// A comment /* and some more
null)", encoder.finish());
        encoder << comment("A comment */ and some more") << nullptr;
        ASSERT_EQ(
R"(// A comment */ and some more
null)", encoder.finish());
    }
    { // Invalid characters
        Encoder encoder{Density::uniline};
        encoder << comment("A\rcomment"sv);
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << comment("A\tcomment"sv);
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << comment("A\0comment"sv);
        ASSERT_FALSE(encoder.status());
    }
    { // Nospace
        Encoder encoder{Density::nospace};
        encoder << comment("A comment") << array << comment("A comment") << comment("A comment") << 0 << comment("A comment") << end << comment("A comment");
        ASSERT_EQ("/*A comment*/[/*A comment*//*A comment*/0,/*A comment*/],/*A comment*/", encoder.finish());
    }
    { // Weird
        Encoder encoder{Density::multiline};
        encoder << comment("") << nullptr;
        ASSERT_EQ("// \nnull", encoder.finish());
        encoder << comment("\n") << nullptr;
        ASSERT_EQ("// \n// \nnull", encoder.finish());
        encoder << comment("a\nb\nc") << nullptr;
        ASSERT_EQ("// a\n// b\n// c\nnull", encoder.finish());
        encoder << comment("\n\n\n") << nullptr;
        ASSERT_EQ("// \n// \n// \n// \nnull", encoder.finish());
    }
}

TEST(encode, indentSpaces)
{
    { // 0
        Encoder encoder{Density::multiline, 0u};
        encoder << object;
            encoder << "k" << array;
                encoder << comment("c");
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(R"({
"k": [
// c
"v"
]
})", encoder.finish());
    }
    { // 1
        Encoder encoder{Density::multiline, 1u};
        encoder << object;
            encoder << "k" << array;
                encoder << comment("c");
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(R"({
 "k": [
  // c
  "v"
 ]
})", encoder.finish());
    }
    { // 7
        Encoder encoder{Density::multiline, 7u};
        encoder << object;
            encoder << "k" << array;
                encoder << comment("c");
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(R"({
       "k": [
              // c
              "v"
       ]
})", encoder.finish());
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
    encoder << comment("Third quarter summary document\nProtected information, do not propagate!"sv);
    encoder << object;
        encoder << "Name"sv << "Salt's Crust"sv;
        encoder << "Founded"sv << 1964;
        encoder << comment("Not necessarily up to date"sv) << "Employees"sv << array;
            encoder << object(Density::uniline) << "Name"sv << "Ol' Joe Fisher"sv << "Title"sv << "Fisherman"sv << "Age"sv << 69 << end;
            encoder << object(Density::uniline) << "Name"sv << "Mark Rower"sv << "Title"sv << "Cook"sv << "Age"sv << 41 << end;
            encoder << object(Density::uniline) << "Name"sv << "Phineas"sv << "Title"sv << "Server Boy"sv << "Age"sv << 19 << end;
        encoder << end;
        encoder << "Dishes"sv << array;
            encoder << object;
                encoder << "Name"sv << "Basket o' Barnacles"sv;
                encoder << "Price"sv << 5.45;
                encoder << "Ingredients"sv << array(Density::uniline) << "\"Salt\""sv << "Barnacles"sv << end;
                encoder << "Gluten Free"sv << false;
            encoder << end;
            encoder << object;
                encoder << "Name"sv << "Two Tuna"sv;
                encoder << "Price"sv << -std::numeric_limits<double>::infinity();
                encoder << "Ingredients"sv << array(Density::uniline) << "Tuna"sv << comment("It's actually cod lmao") << end;
                encoder << "Gluten Free"sv << true;
            encoder << end;
            encoder << object;
                encoder << "Name"sv << "18 Leg Bouquet"sv;
                encoder << "Price"sv << std::numeric_limits<double>::quiet_NaN();
                encoder << "Ingredients"sv << array(Density::uniline) << "\"Salt\""sv << "Octopus"sv << "Crab"sv << end;
                encoder << "Gluten Free"sv << false;
            encoder << end;
        encoder << end;
        encoder << comment("Pay no heed"sv) << "Profit Margin"sv << nullptr;
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
        encoder << comment("What could they mean?!"sv) << "Magic Numbers"sv << array(Density::nospace) << hex(777) << octal(777u) << binary(777) << end;
    encoder << end;

    ASSERT_EQ(R"(// Third quarter summary document
// Protected information, do not propagate!
{
    "Name": "Salt's Crust",
    "Founded": 1964,
    // Not necessarily up to date
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
            "Ingredients": [ "Tuna", /* It's actually cod lmao */ ],
            "Gluten Free": true
        },
        {
            "Name": "18 Leg Bouquet",
            "Price": nan,
            "Ingredients": [ "\"Salt\"", "Octopus", "Crab" ],
            "Gluten Free": false
        }
    ],
    // Pay no heed
    "Profit Margin": null,
    "Ha\x03r Name": "M\0\0n",
    "Green Eggs and Ham": "I do not like them in a box\nI do not like them with a fox\nI do not like them in a house\nI do not like them with a mouse\nI do not like them here or there\nI do not like them anywhere\nI do not like green eggs and ham\nI do not like them Sam I am\n",
    // What could they mean?!
    "Magic Numbers": [0x309,0o1411,0b1100001001]
})"s, encoder.finish());
}
