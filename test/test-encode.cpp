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
using qcon::Timepoint;
using qcon::Date;
using qcon::Time;
using qcon::Datetime;
using qcon::Timezone;
using enum qcon::Container;
using enum qcon::Density;
using enum qcon::Base;
using enum qcon::TimezoneFormat;

struct CustomVal { s32 x, y; };

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

TEST(Encode, object)
{
    { // Empty
        Encoder encoder{};
        encoder << multiline << object << end;
        ASSERT_EQ(encoder.finish(), "{}");
        encoder << uniline << object << end;
        ASSERT_EQ(encoder.finish(), "{}");
        encoder << nospace << object << end;
        ASSERT_EQ(encoder.finish(), "{}");
    }
    { // Non-empty
        Encoder encoder{};
        encoder << multiline << object << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(encoder.finish(), R"({
    "k1": "abc",
    "k2": 123,
    "k3": true
})");
        encoder << uniline << object << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(encoder.finish(), R"({ "k1": "abc", "k2": 123, "k3": true })");
        encoder << nospace << object << "k1" << "abc" << "k2" << 123 << "k3" << true << end;
        ASSERT_EQ(encoder.finish(), R"({"k1":"abc","k2":123,"k3":true})");
    }
    { // String view key
        Encoder encoder{uniline};
        encoder << object << "k"sv << "v" << end;
        ASSERT_EQ(encoder.finish(), R"({ "k": "v" })");
    }
    { // String key
        Encoder encoder{uniline};
        encoder << object << "k"s << "v" << end;
        ASSERT_EQ(encoder.finish(), R"({ "k": "v" })");
    }
    { // Const C string key
        Encoder encoder{uniline};
        encoder << object << "k" << "v" << end;
        ASSERT_EQ(encoder.finish(), R"({ "k": "v" })");
    }
    { // Mutable C string key
        Encoder encoder{uniline};
        encoder << object << const_cast<char *>("k") << "v" << end;
        ASSERT_EQ(encoder.finish(), R"({ "k": "v" })");
    }
    { // Character key
        Encoder encoder{uniline};
        encoder << object << 'k' << "v" << end;
        ASSERT_EQ(encoder.finish(), R"({ "k": "v" })");
    }
    { // Empty key
        Encoder encoder{uniline};
        encoder << object << "" << "" << end;
        ASSERT_EQ(encoder.finish(), R"({ "": "" })");
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

TEST(Encode, array)
{
    { // Empty
        Encoder encoder{};
        encoder << multiline << array << end;
        ASSERT_EQ(encoder.finish(), "[]");
        encoder << uniline << array << end;
        ASSERT_EQ(encoder.finish(), "[]");
        encoder << nospace << array << end;
        ASSERT_EQ(encoder.finish(), "[]");
    }
    { // Non-empty
        Encoder encoder{};
        encoder << multiline << array << "abc" << 123 << true << end;
        ASSERT_EQ(encoder.finish(), R"([
    "abc",
    123,
    true
])");
        encoder << uniline << array << "abc" << 123 << true << end;
        ASSERT_EQ(encoder.finish(), R"([ "abc", 123, true ])");
        encoder << nospace << array << "abc" << 123 << true << end;
        ASSERT_EQ(encoder.finish(), R"(["abc",123,true])");
    }
}

TEST(Encode, string)
{
    { // Empty
        Encoder encoder{};
        encoder << "";
        ASSERT_EQ(encoder.finish(), R"("")");
    }
    { // String view
        Encoder encoder{};
        encoder << "hello"sv;
        ASSERT_EQ(encoder.finish(), R"("hello")");
    }
    { // String
        Encoder encoder{};
        encoder << "hello"s;
        ASSERT_EQ(encoder.finish(), R"("hello")");
    }
    { // Const C string
        Encoder encoder{};
        encoder << "hello";
        ASSERT_EQ(encoder.finish(), R"("hello")");
    }
    { // Mutable C string
        Encoder encoder{};
        encoder << const_cast<char *>("hello");
        ASSERT_EQ(encoder.finish(), R"("hello")");
    }
    { // All ASCII characters
        Encoder encoder{nospace};
        const std::string actual{"\0\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F"sv};
        const std::string expected{"\"\\0\\x01\\x02\\x03\\x04\\x05\\x06\\a\\b\\t\\n\\v\\f\\r\\x0E\\x0F\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1A\\x1B\\x1C\\x1D\\x1E\\x1F !\\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F\""sv};
        encoder << actual;
        ASSERT_EQ(encoder.finish(), expected);
    }
    { // All non-ASCII characters
        std::string actual{};
        for (unat c{128u}; c < 256u; ++c)
        {
            actual.push_back(char(c));
        }
        Encoder encoder{};
        encoder << actual;
        const std::string expectedStr{'"' + actual + '"'};
        ASSERT_EQ(encoder.finish(), expectedStr);
    }
    { // Single char
        Encoder encoder{};
        encoder << 'a';
        ASSERT_EQ(encoder.finish(), R"("a")");
    }
    { // Double quotes
        Encoder encoder{uniline};
        std::string expected{};
        encoder << R"(s"t'r)";
        expected = R"("s\"t'r")";
        ASSERT_EQ(encoder.finish(), expected);
        encoder << object << R"(""")" << R"(''')" << end;
        expected = R"({ "\"\"\"": "'''" })";
        ASSERT_EQ(encoder.finish(), expected);
        encoder << object << R"(''')" << R"(""")" << end;
        expected = R"({ "'''": "\"\"\"" })";
        ASSERT_EQ(encoder.finish(), expected);
    }
    { // Unicode
        Encoder encoder{};
        encoder << "\x41 \xD7\x90 \xEA\xB0\x80 \xF0\x9F\x98\x82";
        ASSERT_EQ(encoder.finish(), "\"\x41 \xD7\x90 \xEA\xB0\x80 \xF0\x9F\x98\x82\"");
    }
    { // Multi line string
        Encoder encoder{multiline};

        encoder << "a\nb\r\nc";
        ASSERT_EQ(encoder.finish(),
            R"("a\n"
"b\r\n"
"c")");

        encoder << "a\nb\r\nc\n";
        ASSERT_EQ(encoder.finish(),
            R"("a\n"
"b\r\n"
"c\n")");

        encoder << object << "A\nrather\nlong\nkey" << "A\nrather\nlong\nvalue" << end;
        ASSERT_EQ(encoder.finish(),
            R"({
    "A\n"
    "rather\n"
    "long\n"
    "key": "A\n"
           "rather\n"
           "long\n"
           "value"
})");

        encoder << "a\n";
        ASSERT_EQ(encoder.finish(), R"("a\n")");

        encoder << object << "k\n" << "a\n" << end;
        ASSERT_EQ(encoder.finish(),
            R"({
    "k\n": "a\n"
})");

        encoder << "\n";
        ASSERT_EQ(encoder.finish(), R"("\n")");

        encoder << object << "\n" << "\n" << end;
        ASSERT_EQ(encoder.finish(),
            R"({
    "\n": "\n"
})");

        encoder << object << "\n\na\n\nb\n\n" << "\n\nc\n\nd\n\n" << end;
        ASSERT_EQ(encoder.finish(),
            R"({
    "\n"
    "\n"
    "a\n"
    "\n"
    "b\n"
    "\n": "\n"
          "\n"
          "c\n"
          "\n"
          "d\n"
          "\n"
})");
    }
    { // Multi string higher density
        Encoder encoder{uniline};

        encoder << "a\nb";
        ASSERT_EQ(encoder.finish(), R"("a\nb")");

        encoder << object << "a\nb" << "c\nd" << end;
        ASSERT_EQ(encoder.finish(), R"({ "a\nb": "c\nd" })");
    }
}

TEST(Encode, signedInteger)
{
    { // Zero
        Encoder encoder{};
        encoder << s64(0);
        ASSERT_EQ(encoder.finish(), "0");
    }
    { // Typical
        Encoder encoder{};
        encoder << 123;
        ASSERT_EQ(encoder.finish(), "123");
        encoder << -123;
        ASSERT_EQ(encoder.finish(), "-123");
    }
    { // Max 64
        Encoder encoder{};
        encoder << std::numeric_limits<s64>::max();
        ASSERT_EQ(encoder.finish(), "9223372036854775807");
    }
    { // Min 64
        Encoder encoder{};
        encoder << std::numeric_limits<s64>::min();
        ASSERT_EQ(encoder.finish(), "-9223372036854775808");
    }
    { // Max 32
        Encoder encoder{};
        encoder << std::numeric_limits<s32>::max();
        ASSERT_EQ(encoder.finish(), "2147483647");
    }
    { // Min 32
        Encoder encoder{};
        encoder << std::numeric_limits<s32>::min();
        ASSERT_EQ(encoder.finish(), "-2147483648");
    }
    { // Max 16
        Encoder encoder{};
        encoder << std::numeric_limits<s16>::max();
        ASSERT_EQ(encoder.finish(), "32767");
    }
    { // Min 16
        Encoder encoder{};
        encoder << std::numeric_limits<s16>::min();
        ASSERT_EQ(encoder.finish(), "-32768");
    }
    { // Max 8
        Encoder encoder{};
        encoder << std::numeric_limits<s8>::max();
        ASSERT_EQ(encoder.finish(), "127");
    }
    { // Min 8
        Encoder encoder{};
        encoder << std::numeric_limits<s8>::min();
        ASSERT_EQ(encoder.finish(), "-128");
    }
}

TEST(Encode, unsignedInteger)
{
    { // Zero
        Encoder encoder{};
        encoder << 0u;
        ASSERT_EQ(encoder.finish(), "0");
    }
    { // Typical
        Encoder encoder{};
        encoder << 123u;
        ASSERT_EQ(encoder.finish(), "123");
    }
    { // Max 64
        Encoder encoder{};
        encoder << std::numeric_limits<u64>::max();
        ASSERT_EQ(encoder.finish(), "18446744073709551615");
    }
    { // Max 32
        Encoder encoder{};
        encoder << std::numeric_limits<u32>::max();
        ASSERT_EQ(encoder.finish(), "4294967295");
    }
    { // Max 16
        Encoder encoder{};
        encoder << std::numeric_limits<u16>::max();
        ASSERT_EQ(encoder.finish(), "65535");
    }
    { // Max 8
        Encoder encoder{};
        encoder << std::numeric_limits<u8>::max();
        ASSERT_EQ(encoder.finish(), "255");
    }
}

TEST(Encode, hex)
{
    { // Zero
        Encoder encoder{};
        encoder << hex << 0x0;
        ASSERT_EQ(encoder.finish(), "0x0");
    }
    { // Typical
        Encoder encoder{};
        encoder << hex << 0x1A;
        ASSERT_EQ(encoder.finish(), "0x1A");
        encoder << hex << -0x1A;
        ASSERT_EQ(encoder.finish(), "-0x1A");
    }
    { // Every digit
        Encoder encoder{};
        encoder << hex << 0xFEDCBA9876543210u;
        ASSERT_EQ(encoder.finish(), "0xFEDCBA9876543210");
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << hex << std::numeric_limits<u64>::max();
        ASSERT_EQ(encoder.finish(), "0xFFFFFFFFFFFFFFFF");
    }
    { // Max signed
        Encoder encoder{};
        encoder << hex << std::numeric_limits<s64>::max();
        ASSERT_EQ(encoder.finish(), "0x7FFFFFFFFFFFFFFF");
    }
    { // Min signed
        Encoder encoder{};
        encoder << hex << std::numeric_limits<s64>::min();
        ASSERT_EQ(encoder.finish(), "-0x8000000000000000");
    }
}

TEST(Encode, octal)
{
    { // Zero
        Encoder encoder{};
        encoder << octal << 00;
        ASSERT_EQ(encoder.finish(), "0o0");
    }
    { // Typical
        Encoder encoder{};
        encoder << octal << 012;
        ASSERT_EQ(encoder.finish(), "0o12");
        encoder << octal << -012;
        ASSERT_EQ(encoder.finish(), "-0o12");
    }
    { // Every digit
        Encoder encoder{};
        encoder << octal << 076543210u;
        ASSERT_EQ(encoder.finish(), "0o76543210");
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << octal << std::numeric_limits<u64>::max();
        ASSERT_EQ(encoder.finish(), "0o1777777777777777777777");
    }
    { // Max signed
        Encoder encoder{};
        encoder << octal << std::numeric_limits<s64>::max();
        ASSERT_EQ(encoder.finish(), "0o777777777777777777777");
    }
    { // Min signed
        Encoder encoder{};
        encoder << octal << std::numeric_limits<s64>::min();
        ASSERT_EQ(encoder.finish(), "-0o1000000000000000000000");
    }
}

TEST(Encode, binary)
{
    { // Zero
        Encoder encoder{};
        encoder << binary << 0b0;
        ASSERT_EQ(encoder.finish(), "0b0");
    }
    { // One
        Encoder encoder{};
        encoder << binary << 0b1;
        ASSERT_EQ(encoder.finish(), "0b1");
        encoder << binary << -0b1;
        ASSERT_EQ(encoder.finish(), "-0b1");
    }
    { // Typical
        Encoder encoder{};
        encoder << binary << 0b101;
        ASSERT_EQ(encoder.finish(), "0b101");
        encoder << binary << -0b101;
        ASSERT_EQ(encoder.finish(), "-0b101");
    }
    { // Non multiple of 4 lengths
        Encoder encoder{};
        encoder << binary << 0b1100'0011;
        ASSERT_EQ(encoder.finish(), "0b11000011");
        encoder << binary << 0b1'1100'0011;
        ASSERT_EQ(encoder.finish(), "0b111000011");
        encoder << binary << 0b11'1100'0011;
        ASSERT_EQ(encoder.finish(), "0b1111000011");
        encoder << binary << 0b111'1100'0011;
        ASSERT_EQ(encoder.finish(), "0b11111000011");
        encoder << binary << 0b1111'1100'0011;
        ASSERT_EQ(encoder.finish(), "0b111111000011");
    }
    { // Every 4 length permutation
        Encoder encoder{};
        encoder << binary << 0b0001'0010'0011'0000'0100'0101'0110'0111'1000'1001'1010'1011'1100'1101'1110'1111u;
        ASSERT_EQ(encoder.finish(), "0b1001000110000010001010110011110001001101010111100110111101111");
    }
    { // Max unsigned
        Encoder encoder{};
        encoder << binary << std::numeric_limits<u64>::max();
        ASSERT_EQ(encoder.finish(), "0b1111111111111111111111111111111111111111111111111111111111111111");
    }
    { // Max signed
        Encoder encoder{};
        encoder << binary << std::numeric_limits<s64>::max();
        ASSERT_EQ(encoder.finish(), "0b111111111111111111111111111111111111111111111111111111111111111");
    }
    { // Min signed
        Encoder encoder{};
        encoder << binary << std::numeric_limits<s64>::min();
        ASSERT_EQ(encoder.finish(), "-0b1000000000000000000000000000000000000000000000000000000000000000");
    }
}

TEST(Encode, floater)
{
    { // Zero
        Encoder encoder{};
        encoder << 0.0;
        ASSERT_EQ(encoder.finish(), "0.0");
    }
    { // Typical
        Encoder encoder{};
        encoder << 123.45;
        ASSERT_EQ(encoder.finish(), "123.45");
    }
    { // Max integer 64
        Encoder encoder{};
        u64 val{0b0'10000110011'1111111111111111111111111111111111111111111111111111u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(encoder.finish(), "9007199254740991.0");
    }
    { // Max integer 32
        Encoder encoder{};
        u32 val{0b0'10010110'11111111111111111111111u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(encoder.finish(), "16777215.0");
    }
    { // Max 64
        Encoder encoder{};
        u64 val{0b0'11111111110'1111111111111111111111111111111111111111111111111111u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(encoder.finish(), "1.7976931348623157e+308");
    }
    { // Max 32
        Encoder encoder{};
        u32 val{0b0'11111110'11111111111111111111111u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(encoder.finish(), "3.4028234663852886e+38");
    }
    { // Min normal 64
        Encoder encoder{};
        u64 val{0b0'00000000001'0000000000000000000000000000000000000000000000000000u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(encoder.finish(), "2.2250738585072014e-308");
    }
    { // Min normal 32
        Encoder encoder{};
        u32 val{0b0'00000001'00000000000000000000000u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(encoder.finish(), "1.1754943508222875e-38");
    }
    { // Min subnormal 64
        Encoder encoder{};
        u64 val{0b0'00000000000'0000000000000000000000000000000000000000000000000001u};
        encoder << reinterpret_cast<const double &>(val);
        ASSERT_EQ(encoder.finish(), "5e-324");
    }
    { // Min subnormal 32
        Encoder encoder{};
        u64 val{0b0'00000000'00000000000000000000001u};
        encoder << reinterpret_cast<const float &>(val);
        ASSERT_EQ(encoder.finish(), "1.401298464324817e-45");
    }
    { // Positive infinity
        Encoder encoder{};
        encoder << std::numeric_limits<double>::infinity();
        ASSERT_EQ(encoder.finish(), "inf");
    }
    { // Negative infinity
        Encoder encoder{};
        encoder << -std::numeric_limits<double>::infinity();
        ASSERT_EQ(encoder.finish(), "-inf");
    }
    { // NaN
        Encoder encoder{};
        encoder << std::numeric_limits<double>::quiet_NaN();
        ASSERT_EQ(encoder.finish(), "nan");
    }
    { // Negative NaN
        Encoder encoder{};
        encoder << -std::numeric_limits<double>::quiet_NaN();
        ASSERT_EQ(encoder.finish(), "nan");
    }
}

TEST(Encode, boolean)
{
    { // True
        Encoder encoder{};
        encoder << true;
        ASSERT_EQ(encoder.finish(), "true");
    }
    { // False
        Encoder encoder{};
        encoder << false;
        ASSERT_EQ(encoder.finish(), "false");
    }
}

TEST(Encode, date)
{
    { // Default
        Encoder encoder{};
        encoder << Date{};
        ASSERT_EQ(encoder.finish(), "D1970-01-01");
    }
    { // General
        Encoder encoder{};
        encoder << Date{.year = 2023u, .month = 2u, .day = 17u};
        ASSERT_EQ(encoder.finish(), "D2023-02-17");
    }
    { // Min
        Encoder encoder{};
        encoder << Date{.year = 0u, .month = 1u, .day = 1u};
        ASSERT_EQ(encoder.finish(), "D0000-01-01");
    }
    { // Max
        Encoder encoder{};
        encoder << Date{.year = 9999u, .month = 12u, .day = 31u};
        ASSERT_EQ(encoder.finish(), "D9999-12-31");
    }
    { // Invalid
        Encoder encoder{};
        encoder << Date{.year = 10000u, .month = 1u, .day = 1u};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Date{.year = 0u, .month = 0u, .day = 1u};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Date{.year = 0u, .month = 13u, .day = 1u};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Date{.year = 0u, .month = 1u, .day = 0u};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Date{.year = 0u, .month = 1u, .day = 32u};
        ASSERT_FALSE(encoder.status());
    }
}

TEST(Encode, time)
{
    { // Default
        Encoder encoder{};
        encoder << Time{};
        ASSERT_EQ(encoder.finish(), "T00:00:00");
    }
    { // General
        Encoder encoder{};
        encoder << Time{.hour = 12u, .minute = 34u, .second = 56u};
        ASSERT_EQ(encoder.finish(), "T12:34:56");
    }
    { // Min
        Encoder encoder{};
        encoder << Time{.hour = 0u, .minute = 0u, .second = 0u, .subsecond = 0u};
        ASSERT_EQ(encoder.finish(), "T00:00:00");
    }
    { // Max
        Encoder encoder{};
        encoder << Time{.hour = 23u, .minute = 59u, .second = 59u, .subsecond = 999'999'999u};
        ASSERT_EQ(encoder.finish(), "T23:59:59.999999999");
    }
    { // Too large
        Encoder encoder{};
        encoder << Time{.hour = 24u};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Time{.minute = 60u};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Time{.second = 60u};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Time{.subsecond = 1'000'000'000u};
        ASSERT_FALSE(encoder.status());
    }
    { // Subsecond digits
        Encoder encoder{};
        encoder << Time{.subsecond = 1u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.000000001");

        encoder.reset();
        encoder << Time{.subsecond = 10u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.00000001");

        encoder.reset();
        encoder << Time{.subsecond = 100u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.0000001");

        encoder.reset();
        encoder << Time{.subsecond = 1000u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.000001");

        encoder.reset();
        encoder << Time{.subsecond = 10000u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.00001");

        encoder.reset();
        encoder << Time{.subsecond = 100000u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.0001");

        encoder.reset();
        encoder << Time{.subsecond = 1000000u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.001");

        encoder.reset();
        encoder << Time{.subsecond = 10000000u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.01");

        encoder.reset();
        encoder << Time{.subsecond = 100000000u};
        ASSERT_EQ(encoder.finish(), "T00:00:00.1");
    }
}

TEST(Encode, datetime)
{
    { // General
        Encoder encoder{};
        Datetime datetime{.date = {.year = 2023, .month = 2, .day = 17}, .time = {.hour = 12, .minute = 34, .second = 56, .subsecond = 123456789}, .zone = {.format = localTime, .offset = 12 * 60 + 34}};
        encoder << datetime;
        ASSERT_EQ(encoder.finish(), "D2023-02-17T12:34:56.123456789");
        datetime.zone.format = utc;
        encoder << datetime;
        ASSERT_EQ(encoder.finish(), "D2023-02-17T12:34:56.123456789Z");
        datetime.zone.format = utcOffset;
        encoder << datetime;
        ASSERT_EQ(encoder.finish(), "D2023-02-17T12:34:56.123456789+12:34");
    }
    { // Default
        Encoder encoder{};
        const Datetime datetime{};
        encoder << datetime;
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00");
    }
    { // Either is invalid
        Encoder encoder{};
        encoder << Datetime{.date = {.year = 10000}};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Datetime{.time = {.hour = 24}};
        ASSERT_FALSE(encoder.status());
    }
    { // UTC timezone format
        Encoder encoder{};
        encoder << Datetime{.zone = {.format = utc}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00Z");
    }
    { // UTC offset timezone format
        Encoder encoder{};
        encoder << Datetime{.zone = {.format = utcOffset}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00+00:00");

        encoder.reset();
        encoder << Datetime{.zone = {.format = utcOffset, .offset = 12 * 60 + 34}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00+12:34");

        encoder.reset();
        encoder << Datetime{.zone = {.format = utcOffset, .offset = -(12 * 60 + 34)}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00-12:34");

        encoder.reset();
        encoder << Datetime{.zone = {.format = utcOffset, .offset = 100 * 60 - 1}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00+99:59");

        encoder.reset();
        encoder << Datetime{.zone = {.format = utcOffset, .offset = 100 * 60}};
        ASSERT_FALSE(encoder.status());

        encoder.reset();
        encoder << Datetime{.zone = {.format = utcOffset, .offset = -(100 * 60 - 1)}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00-99:59");

        encoder.reset();
        encoder << Datetime{.zone = {.format = utcOffset, .offset = -(100 * 60)}};
        ASSERT_FALSE(encoder.status());
    }
}

TEST(Encode, timepoint)
{
    { // Epoch
        Encoder encoder{};
        encoder << utcOffset << Timepoint{};
        ASSERT_EQ(encoder.finish(), "D1969-12-31T16:00:00-08:00");
        encoder << utc << Timepoint{};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00Z");
        encoder << localTime << Timepoint{};
        ASSERT_EQ(encoder.finish(), "D1969-12-31T16:00:00");
    }
    { // Positive timestamp
        const Timepoint tp{std::chrono::seconds{1676337198} + std::chrono::microseconds{123456}};
        Encoder encoder{};
        encoder << utcOffset << tp;
        ASSERT_EQ(encoder.finish(), "D2023-02-13T17:13:18.123456-08:00");
        encoder << utc << tp;
        ASSERT_EQ(encoder.finish(), "D2023-02-14T01:13:18.123456Z");
        encoder << localTime << tp;
        ASSERT_EQ(encoder.finish(), "D2023-02-13T17:13:18.123456");
    }
    { // Negative timestamp
        const Timepoint tp{std::chrono::seconds{-777777777} + std::chrono::microseconds{142536}};
        Encoder encoder{};
        encoder << utcOffset << tp;
        ASSERT_EQ(encoder.finish(), "D1945-05-09T15:37:03.142536-07:00");
        encoder << utc << tp;
        ASSERT_EQ(encoder.finish(), "D1945-05-09T22:37:03.142536Z");
        encoder << localTime << tp;
        ASSERT_EQ(encoder.finish(), "D1945-05-09T15:37:03.142536");
    }
    { // Future timestamp
        const Timepoint tp{std::chrono::seconds{253402300799}};
        Encoder encoder{};
        encoder << utc << tp;
        ASSERT_EQ(encoder.finish(), "D9999-12-31T23:59:59Z");
    }
    { // Past timestamp
        const Timepoint tp{std::chrono::seconds{-62167219200}};
        Encoder encoder{};
        encoder << utc << tp;
        ASSERT_EQ(encoder.finish(), "D0000-01-01T00:00:00Z");
    }
    { // Too far in the future
        Encoder encoder{};
        encoder << utc << Timepoint{std::chrono::seconds{253402300800}};
        ASSERT_FALSE(encoder.status());
    }
    { // Too far in the past
        Encoder encoder{};
        encoder << utc << Timepoint{std::chrono::seconds{-62167219201}};
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
    { // Subseconds
        static_assert(std::chrono::system_clock::duration::period::den == 10'000'000);
        Encoder encoder{};
        encoder << utc << Timepoint{std::chrono::system_clock::duration{1'000'000}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00.1Z");
        encoder << utc << Timepoint{std::chrono::system_clock::duration{100'000}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00.01Z");
        encoder << utc << Timepoint{std::chrono::system_clock::duration{10'000}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00.001Z");
        encoder << utc << Timepoint{std::chrono::system_clock::duration{1'000}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00.0001Z");
        encoder << utc << Timepoint{std::chrono::system_clock::duration{100}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00.00001Z");
        encoder << utc << Timepoint{std::chrono::system_clock::duration{10}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00.000001Z");
        encoder << utc << Timepoint{std::chrono::system_clock::duration{1}};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00.0000001Z");
    }
}

TEST(Encode, null)
{
    Encoder encoder{};
    encoder << nullptr;
    ASSERT_EQ(encoder.finish(), "null");
}

TEST(Encode, custom)
{
    Encoder encoder{};
    encoder << CustomVal{1, 2};
    ASSERT_EQ(encoder.finish(), "[ 1, 2 ]");
}

TEST(Encode, reset)
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
        ASSERT_EQ(encoder.finish(), "true");
    }
}

TEST(Encode, finish)
{
    { // Encoder left in clean state after finish
        Encoder encoder{uniline};
        encoder << object << "val" << 123 << end;
        ASSERT_EQ(encoder.finish(), R"({ "val": 123 })");
        encoder << array << 321 << end;
        ASSERT_EQ(encoder.finish(), "[ 321 ]");
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

TEST(Encode, density)
{
    { // Default density
        Encoder encoder{};
        ASSERT_EQ(encoder.density(), multiline);
    }
    { // Top level multiline
        Encoder encoder{multiline};
        ASSERT_EQ(encoder.density(), multiline);
        encoder << object;
            ASSERT_EQ(encoder.density(), multiline);
            encoder << "k1" << array;
                ASSERT_EQ(encoder.density(), multiline);
                encoder << "v1" << "v2";
            encoder << end;
            ASSERT_EQ(encoder.density(), multiline);
            encoder << "k2" << "v3";
        encoder << end;
        ASSERT_EQ(encoder.density(), multiline);
        ASSERT_EQ(encoder.finish(), R"({
    "k1": [
        "v1",
        "v2"
    ],
    "k2": "v3"
})");
    }
    { // Top level uniline
        Encoder encoder{uniline};
        ASSERT_EQ(encoder.density(), uniline);
        encoder << object;
            ASSERT_EQ(encoder.density(), uniline);
            encoder << "k1" << array;
                ASSERT_EQ(encoder.density(), uniline);
                encoder << "v1" << "v2";
            encoder << end;
            ASSERT_EQ(encoder.density(), uniline);
            encoder << "k2" << "v3";
        encoder << end;
        ASSERT_EQ(encoder.density(), uniline);
        ASSERT_EQ(encoder.finish(), R"({ "k1": [ "v1", "v2" ], "k2": "v3" })");
    }
    { // Top level nospace
        Encoder encoder{nospace};
        ASSERT_EQ(encoder.density(), nospace);
        encoder << object;
            ASSERT_EQ(encoder.density(), nospace);
            encoder << "k1" << array;
                ASSERT_EQ(encoder.density(), nospace);
                encoder << "v1" << "v2";
            encoder << end;
            ASSERT_EQ(encoder.density(), nospace);
            encoder << "k2" << "v3";
        encoder << end;
        ASSERT_EQ(encoder.density(), nospace);
        ASSERT_EQ(encoder.finish(), R"({"k1":["v1","v2"],"k2":"v3"})");
    }
    { // Inner density
        Encoder encoder{};
        encoder << object;
            ASSERT_EQ(encoder.density(), multiline);
            encoder << "k1" << uniline << array;
                ASSERT_EQ(encoder.density(), uniline);
                encoder << "v1" << nospace << array;
                    ASSERT_EQ(encoder.density(), nospace);
                    encoder << "v2" << "v3";
                encoder << end;
                ASSERT_EQ(encoder.density(), uniline);
            encoder << end;
            ASSERT_EQ(encoder.density(), multiline);
            encoder << "k2" << uniline << object;
                ASSERT_EQ(encoder.density(), uniline);
                encoder << "k3" << "v4" << "k4" << nospace << object;
                    ASSERT_EQ(encoder.density(), nospace);
                    encoder << "k5" << "v5" << "k6" << "v6";
                encoder << end;
                ASSERT_EQ(encoder.density(), uniline);
            encoder << end;
            ASSERT_EQ(encoder.density(), multiline);
        encoder << end;
        ASSERT_EQ(encoder.density(), multiline);
        ASSERT_EQ(encoder.finish(), R"({
    "k1": [ "v1", ["v2","v3"] ],
    "k2": { "k3": "v4", "k4": {"k5":"v5","k6":"v6"} }
})");
    }
    { // Density priority
        Encoder encoder{};
        ASSERT_EQ(encoder.density(), multiline);
        encoder << uniline << object;
            ASSERT_EQ(encoder.density(), uniline);
            encoder << "k" << multiline << array;
                ASSERT_EQ(encoder.density(), uniline);
                encoder << "v";
            encoder << end;
            ASSERT_EQ(encoder.density(), uniline);
        encoder << end;
        ASSERT_EQ(encoder.density(), multiline);
        ASSERT_EQ(encoder.finish(), R"({ "k": [ "v" ] })");

        ASSERT_EQ(encoder.density(), multiline);
        encoder << uniline << array;
            ASSERT_EQ(encoder.density(), uniline);
            encoder << multiline << object;
                ASSERT_EQ(encoder.density(), uniline);
                encoder << "k" << "v";
            encoder << end;
            ASSERT_EQ(encoder.density(), uniline);
        encoder << end;
        ASSERT_EQ(encoder.density(), multiline);
        ASSERT_EQ(encoder.finish(), R"([ { "k": "v" } ])");

        ASSERT_EQ(encoder.density(), multiline);
        encoder << nospace << object;
            ASSERT_EQ(encoder.density(), nospace);
            encoder << "k" << uniline << array;
                ASSERT_EQ(encoder.density(), nospace);
                encoder << "v";
            encoder << end;
            ASSERT_EQ(encoder.density(), nospace);
        encoder << end;
        ASSERT_EQ(encoder.density(), multiline);
        ASSERT_EQ(encoder.finish(), R"({"k":["v"]})");

        ASSERT_EQ(encoder.density(), multiline);
        encoder << nospace << array;
            ASSERT_EQ(encoder.density(), nospace);
            encoder << uniline << object;
                ASSERT_EQ(encoder.density(), nospace);
                encoder << "k" << "v";
            encoder << end;
            ASSERT_EQ(encoder.density(), nospace);
        encoder << end;
        ASSERT_EQ(encoder.density(), multiline);
        ASSERT_EQ(encoder.finish(), R"([{"k":"v"}])");
    }
}

TEST(Encode, customIndentation)
{
    { // Empty
        Encoder encoder{multiline, ""};
        encoder << object;
            encoder << "k" << array;
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(encoder.finish(), R"({
"k": [
"v"
]
})");
    }
    { // 1 space
        Encoder encoder{multiline, " "};
        encoder << object;
            encoder << "k" << array;
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(encoder.finish(), R"({
 "k": [
  "v"
 ]
})");
    }
    { // 7 spaces
        Encoder encoder{multiline, "       "};
        encoder << object;
            encoder << "k" << array;
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(encoder.finish(), R"({
       "k": [
              "v"
       ]
})");
    }
    { // Tab
        Encoder encoder{multiline, "\t"};
        encoder << object;
            encoder << "k" << array;
                encoder << "v";
            encoder << end;
        encoder << end;
        ASSERT_EQ(encoder.finish(), "{\n\t\"k\": [\n\t\t\"v\"\n\t]\n}");
    }
}

TEST(Encode, flagTokens)
{
    { // Density
        Encoder encoder{};

        // Mutliple densities
        encoder << nospace << uniline << array << 0 << end;
        ASSERT_EQ(encoder.finish(), "[ 0 ]");

        // Density is transient
        encoder << uniline << array << nospace << array << 0 << end << array << 0 << end << end;
        ASSERT_EQ(encoder.finish(), "[ [0], [ 0 ] ]");

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

        // Date after density
        encoder << nospace << Date{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Time after density
        encoder << nospace << Time{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Datetime after density
        encoder << nospace << Datetime{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Timestamp after density
        encoder << nospace << Timepoint{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Null after density
        encoder << nospace << nullptr;
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
        ASSERT_EQ(encoder.finish(), "0b0");

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

        // Date after base
        encoder << hex << Date{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Time after base
        encoder << hex << Time{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Datetime after base
        encoder << hex << Datetime{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Timestamp after base
        encoder << hex << Timepoint{};
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

        // Mutliple time zone formats
        encoder << localTime << utc << Timepoint{};
        ASSERT_EQ(encoder.finish(), "D1970-01-01T00:00:00Z");

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

        // Date after time zone format
        encoder << utc << Date{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Time after time zone format
        encoder << utc << Time{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Datetime after time zone format
        encoder << utc << Datetime{};
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Null after time zone format
        encoder << utc << nullptr;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Density after time zone format
        encoder << utc << nospace;
        ASSERT_FALSE(encoder.status());
        encoder.reset();

        // Base after time zone format
        encoder << utc << hex;
        ASSERT_FALSE(encoder.status());
        encoder.reset();
    }
}

TEST(Encode, dateOrdering)
{
    { // Year
        const Date d1{1900, 02, 02};
        const Date d2{1901, 01, 01};
        ASSERT_LT(d1, d2);
        ASSERT_LE(d1, d2);
        ASSERT_EQ(d1, d1);
        ASSERT_GE(d2, d1);
        ASSERT_GT(d2, d1);
    }
    { // Month
        const Date d1{1900, 01, 02};
        const Date d2{1900, 02, 01};
        ASSERT_LT(d1, d2);
        ASSERT_LE(d1, d2);
        ASSERT_EQ(d1, d1);
        ASSERT_GE(d2, d1);
        ASSERT_GT(d2, d1);
    }
    { // Day
        const Date d1{1900, 01, 01};
        const Date d2{1900, 01, 02};
        ASSERT_LT(d1, d2);
        ASSERT_LE(d1, d2);
        ASSERT_EQ(d1, d1);
        ASSERT_GE(d2, d1);
        ASSERT_GT(d2, d1);
    }
}

TEST(Encode, timeOrdering)
{
    { // Hour
        const Time t1{0, 1, 1, 1};
        const Time t2{1, 0, 0, 0};
        ASSERT_LT(t1, t2);
        ASSERT_LE(t1, t2);
        ASSERT_EQ(t1, t1);
        ASSERT_GE(t2, t1);
        ASSERT_GT(t2, t1);
    }
    { // Minute
        const Time t1{0, 0, 1, 1};
        const Time t2{0, 1, 0, 0};
        ASSERT_LT(t1, t2);
        ASSERT_LE(t1, t2);
        ASSERT_EQ(t1, t1);
        ASSERT_GE(t2, t1);
        ASSERT_GT(t2, t1);
    }
    { // Second
        const Time t1{0, 0, 0, 1};
        const Time t2{0, 0, 1, 0};
        ASSERT_LT(t1, t2);
        ASSERT_LE(t1, t2);
        ASSERT_EQ(t1, t1);
        ASSERT_GE(t2, t1);
        ASSERT_GT(t2, t1);
    }
    { // Subsecond
        const Time t1{0, 0, 0, 0};
        const Time t2{0, 0, 0, 1};
        ASSERT_LT(t1, t2);
        ASSERT_LE(t1, t2);
        ASSERT_EQ(t1, t1);
        ASSERT_GE(t2, t1);
        ASSERT_GT(t2, t1);
    }
}

TEST(Encode, misc)
{
    { // Extraneous content
        Encoder encoder{};
        encoder << "a";
        ASSERT_TRUE(encoder.status());
        encoder << "b";
        ASSERT_FALSE(encoder.status());
    }
    { // Container accessor
        Encoder encoder{};
        ASSERT_EQ(encoder.container(), end);
        encoder << object;
            ASSERT_EQ(encoder.container(), object);
            encoder << "k" << array;
                ASSERT_EQ(encoder.container(), array);
                encoder << object;
                    ASSERT_EQ(encoder.container(), object);
                encoder << end;
                ASSERT_EQ(encoder.container(), array);
            encoder << end;
            ASSERT_EQ(encoder.container(), object);
        encoder << end;
        ASSERT_EQ(encoder.container(), end);
    }
}

TEST(Encode, general)
{
    Encoder encoder{};
    encoder << object;
        encoder << "Name"sv << "Salt's Crust"sv;
        encoder << "Founded"sv << Date{.year = 1964u, .month = 3u, .day = 17u};
        encoder << "Opens"sv << Time{.hour = 8u, .minute = 30u};
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
        encoder << "Last Updated"sv << utc << Timepoint{std::chrono::seconds{1056808751} + std::chrono::milliseconds{67}};
    encoder << end;

    ASSERT_EQ(encoder.finish(), R"({
    "Name": "Salt's Crust",
    "Founded": D1964-03-17,
    "Opens": T08:30:00,
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
    "Green Eggs and Ham": "I do not like them in a box\n"
                          "I do not like them with a fox\n"
                          "I do not like them in a house\n"
                          "I do not like them with a mouse\n"
                          "I do not like them here or there\n"
                          "I do not like them anywhere\n"
                          "I do not like green eggs and ham\n"
                          "I do not like them Sam I am\n",
    "Magic Numbers": [0x309,0o1411,0b1100001001],
    "Last Updated": D2003-06-28T13:59:11.067Z
})");
}
