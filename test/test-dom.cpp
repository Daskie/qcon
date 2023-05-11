#include <qcon-dom.hpp>

#include <cmath>

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

using qcon::Type;
using qcon::Value;
using qcon::Object;
using qcon::Array;
using qcon::Date;
using qcon::Time;
using qcon::Datetime;
using qcon::Timepoint;
using qcon::decode;
using qcon::encode;

using qcon::makeObject;
using qcon::makeArray;

template <typename T>
std::ostream & operator<<(std::ostream & os, const std::optional<T> & v)
{
    if (v)
    {
        return os << *v;
    }
    else
    {
        return os << "Optional empty";
    }
}

std::ostream & operator<<(std::ostream & os, const qcon::Date & date)
{
    return os << date.year << '-' << u32(date.month) << '-' << u32(date.day);
}

std::ostream & operator<<(std::ostream & os, const qcon::Time & time)
{
    return os << u32(time.hour) << ':' << u32(time.minute) << ':' << u32(time.second) << '.' << time.subsecond;
}

std::ostream & operator<<(std::ostream & os, const qcon::Datetime & datetime)
{
    os << datetime.date << ' ' << datetime.time;
    switch (datetime.zone.format)
    {
        case qcon::localTime: break;
        case qcon::utc: os << 'Z'; break;
        case qcon::utcOffset: os << (datetime.zone.offset >= 0 ? '+' : '-') << std::abs(datetime.zone.offset); break;
    }
    return os;
}

TEST(Dom, encodeDecodeString)
{
    { // Empty
        const std::string_view val{""sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(*decoded->string(), val);
    }
    { // Typical
        std::string_view val{"abc"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(*decoded->string(), val);
    }
    { // Printable characters
        std::string_view val{R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(*decoded->string(), val);
    }
    { // Escape characters
        std::string_view val{"\b\f\n\r\t"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(*decoded->string(), val);
    }
    { // Unicode
        std::string_view val{"\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u000B\u000E\u000F\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F\u007F"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(*decoded->string(), val);
    }
}

TEST(Dom, encodeDecodeSignedInteger)
{
    { // Zero
        s32 val{0};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Typical
        s32 val{123};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Max 64
        s64 val{std::numeric_limits<s64>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Min 64
        s64 val{std::numeric_limits<s64>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Max 32
        s32 val{std::numeric_limits<s32>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Min 32
        s32 val{std::numeric_limits<s32>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Max 16
        s16 val{std::numeric_limits<s16>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Min 16
        s16 val{std::numeric_limits<s16>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Max 8
        s8 val{std::numeric_limits<s8>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
    { // Min 8
        s8 val{std::numeric_limits<s8>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
}

TEST(Dom, encodeDecodeUnsignedInteger)
{
    { // Zero
        u32 val{0u};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(u64(*decoded->integer()), val);
    }
    { // Typical
        u32 val{123u};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(u64(*decoded->integer()), val);
    }
    { // Max 64
        u64 val{std::numeric_limits<u64>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(u64(*decoded->integer()), val);
    }
    { // Max 32
        u32 val{std::numeric_limits<u32>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(u64(*decoded->integer()), val);
    }
    { // Max 16
        u16 val{std::numeric_limits<u16>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(u64(*decoded->integer()), val);
    }
    { // Max 8
        u8 val{std::numeric_limits<u8>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(*decoded->integer(), val);
    }
}

TEST(Dom, encodeDecodeFloater)
{
    { // Zero
        double val{0.0};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Typical
        double val{123.45};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Max integer 64
        double val{std::bit_cast<double>(0b0'10000110011'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Max integer 32
        double val{std::bit_cast<float>(0b0'10010110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Max 64
        double val{std::bit_cast<double>(0b0'11111111110'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Max 32
        double val{std::bit_cast<float>(0b0'11111110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Min normal 64
        double val{std::bit_cast<double>(0b0'00000000001'0000000000000000000000000000000000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Min normal 32
        double val{std::bit_cast<float>(0b0'00000001'00000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Min subnormal 64
        double val{std::bit_cast<double>(u64(0b0'00000000000'0000000000000000000000000000000000000000000000000001u))};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Min subnormal 32
        double val{std::bit_cast<float>(0b0'00000000'00000000000000000000001u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Positive infinity
        double val{std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // Negative infinity
        double val{-std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(*decoded->floater(), val);
    }
    { // NaN
        const std::optional<std::string> encoded{encode(std::numeric_limits<double>::quiet_NaN())};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_TRUE(std::isnan(*decoded->floater()));
    }
}

TEST(Dom, encodeDecodeBoolean)
{
    { // true
        const std::optional<std::string> encoded{encode(true)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->boolean());
        ASSERT_EQ(*decoded->boolean(), true);
    }
    { // false
        const std::optional<std::string> encoded{encode(false)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->boolean());
        ASSERT_EQ(*decoded->boolean(), false);
    }
}

TEST(Dom, encodeDecodeDate)
{
    { // General
        const Date date{.year = 2023u, .month = 2u, .day = 17u};
        const std::optional<std::string> encoded{encode(date)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->date());
        ASSERT_EQ(*decoded->date(), date);
    }
}

TEST(Dom, encodeDecodeTime)
{
    { // General
        const Time time{.hour = 12u, .minute = 34u, .second = 56u, .subsecond = 123456789u};
        const std::optional<std::string> encoded{encode(time)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->time());
        ASSERT_EQ(*decoded->time(), time);
    }
}

TEST(Dom, encodeDecodeDatetime)
{
    { // Default
        const Datetime dt{};
        const std::optional<std::string> encoded{encode(dt)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->datetime());
        ASSERT_EQ(decoded->datetime()->date, dt.date);
        ASSERT_EQ(decoded->datetime()->time, dt.time);
        ASSERT_EQ(decoded->datetime()->zone.format, dt.zone.format);
        ASSERT_EQ(decoded->datetime()->zone.offset, dt.zone.offset);
    }
    { // Current time local
        Datetime dt;
        ASSERT_TRUE(dt.fromTimepoint(std::chrono::system_clock::now(), qcon::localTime));
        const std::optional<std::string> encoded{encode(dt)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->datetime());
        ASSERT_EQ(decoded->datetime()->date, dt.date);
        ASSERT_EQ(decoded->datetime()->time, dt.time);
        ASSERT_EQ(decoded->datetime()->zone.format, dt.zone.format);
    }
    { // Current time UTC
        Datetime dt;
        ASSERT_TRUE(dt.fromTimepoint(std::chrono::system_clock::now(), qcon::utc));
        const std::optional<std::string> encoded{encode(dt)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->datetime());
        ASSERT_EQ(decoded->datetime()->date, dt.date);
        ASSERT_EQ(decoded->datetime()->time, dt.time);
        ASSERT_EQ(decoded->datetime()->zone.format, dt.zone.format);
        ASSERT_EQ(decoded->datetime()->zone.offset, dt.zone.offset);
    }
    { // Current time UTC offset
        Datetime dt;
        ASSERT_TRUE(dt.fromTimepoint(std::chrono::system_clock::now(), qcon::utcOffset));
        const std::optional<std::string> encoded{encode(dt)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->datetime());
        ASSERT_EQ(decoded->datetime()->date, dt.date);
        ASSERT_EQ(decoded->datetime()->time, dt.time);
        ASSERT_EQ(decoded->datetime()->zone.format, dt.zone.format);
        ASSERT_EQ(decoded->datetime()->zone.offset, dt.zone.offset);
    }
}

TEST(Dom, encodeDecodeNull)
{
    const std::optional<std::string> encoded{encode(nullptr)};
    ASSERT_TRUE(encoded);
    const std::optional<Value> decoded{decode(*encoded)};
    ASSERT_TRUE(decoded);
    ASSERT_TRUE(decoded->null());
}

TEST(Dom, valueConstruction)
{
    // Default
    ASSERT_EQ(Value{}.type(), Type::null);

    // Object
    ASSERT_EQ(Value{Object()}.type(), Type::object);

    // Array
    ASSERT_EQ(Value{Array()}.type(), Type::array);

    // String
    ASSERT_EQ(Value{"abc"sv}.type(), Type::string);
    ASSERT_EQ(Value{"abc"s}.type(), Type::string);
    ASSERT_EQ(Value{"abc"}.type(), Type::string);
    ASSERT_EQ(Value{const_cast<char *>("abc")}.type(), Type::string);
    ASSERT_EQ(Value{'a'}.type(), Type::string);

    // Integer
    ASSERT_EQ(Value{s64{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<s64>::max()}.positive());
    ASSERT_FALSE(Value{std::numeric_limits<s64>::min()}.positive());
    ASSERT_EQ(Value{s32{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<s32>::max()}.positive());
    ASSERT_FALSE(Value{std::numeric_limits<s32>::min()}.positive());
    ASSERT_EQ(Value{s16{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<s16>::max()}.positive());
    ASSERT_FALSE(Value{std::numeric_limits<s16>::min()}.positive());
    ASSERT_EQ(Value{s8{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<s8>::max()}.positive());
    ASSERT_FALSE(Value{std::numeric_limits<s8>::min()}.positive());
    ASSERT_EQ(Value{u64{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<u64>::max()}.positive());
    ASSERT_EQ(Value{u32{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<u32>::max()}.positive());
    ASSERT_EQ(Value{u16{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<u16>::max()}.positive());
    ASSERT_EQ(Value{u8{}}.type(), Type::integer);
    ASSERT_TRUE(Value{std::numeric_limits<u8>::max()}.positive());

    // Floater
    ASSERT_EQ(Value{0.0}.type(), Type::floater);
    ASSERT_TRUE(Value{1.0}.positive());
    ASSERT_FALSE(Value{-1.0}.positive());
    ASSERT_EQ(Value{0.0f}.type(), Type::floater);
    ASSERT_TRUE(Value{1.0f}.positive());
    ASSERT_FALSE(Value{-1.0f}.positive());

    // Boolean
    ASSERT_EQ(Value{false}.type(), Type::boolean);

    // Date
    ASSERT_EQ(Value{Date{}}.type(), Type::date);

    // Time
    ASSERT_EQ(Value{Time{}}.type(), Type::time);

    // Datetime
    ASSERT_EQ(Value{Datetime{}}.type(), Type::datetime);

    // Null
    ASSERT_EQ(Value{nullptr}.type(), Type::null);
}

TEST(Dom, valueMove)
{
    Value v1("abc"sv);
    ASSERT_TRUE(v1.string());
    ASSERT_EQ(*v1.string(), "abc"sv);

    Value v2(std::move(v1));
    ASSERT_TRUE(v1.null());
    ASSERT_TRUE(v2.string());
    ASSERT_EQ(*v2.string(), "abc"sv);

    v1 = std::move(v2);
    ASSERT_TRUE(v1.string());
    ASSERT_TRUE(v2.null());
    ASSERT_EQ(*v1.string(), "abc"sv);
}

TEST(Dom, valueAssignAndEquality)
{
    Value v{};

    { // Object
        const Object objRef(makeObject("a", 1, "b", "wow", "c", nullptr));
        v = makeObject("a", 1, "b", "wow", "c", nullptr);
        ASSERT_EQ(v.type(), Type::object);
        ASSERT_TRUE(v == objRef);
        ASSERT_FALSE(v != objRef);
    }
    { // Array
        const Array arrRef(makeArray(0, "a", true));
        v = makeArray(0, "a", true);
        ASSERT_EQ(v.type(), Type::array);
        ASSERT_TRUE(v == arrRef);
        ASSERT_FALSE(v != arrRef);
    }
    { // String
        v = "hello"s;
        ASSERT_EQ(v.type(), Type::string);
        ASSERT_TRUE(v == "hello"s);
        ASSERT_FALSE(v != "hello"s);

        v = "hellu"sv;
        ASSERT_EQ(v.type(), Type::string);
        ASSERT_TRUE(v == "hellu"sv);
        ASSERT_FALSE(v != "hellu"sv);

        v = "helli";
        ASSERT_EQ(v.type(), Type::string);
        ASSERT_TRUE(v == "helli");
        ASSERT_FALSE(v != "helli");

        v = const_cast<char *>("hella");
        ASSERT_EQ(v.type(), Type::string);
        ASSERT_TRUE(v == const_cast<char *>("hella"));
        ASSERT_FALSE(v != const_cast<char *>("hella"));

        v = 'h';
        ASSERT_EQ(v.type(), Type::string);
        ASSERT_TRUE(v == 'h');
        ASSERT_FALSE(v != 'h');
    }
    { // Integer
        v = s64(5);
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == s64(5));
        ASSERT_FALSE(v != s64(5));
        v = std::numeric_limits<s64>::max();
        ASSERT_TRUE(v.positive());
        v = std::numeric_limits<s64>::min();
        ASSERT_FALSE(v.positive());

        v = s32{6};
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == s32{6});
        ASSERT_FALSE(v != s32{6});
        v = std::numeric_limits<s32>::max();
        ASSERT_TRUE(v.positive());
        v = std::numeric_limits<s32>::min();
        ASSERT_FALSE(v.positive());

        v = s16(7);
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == s16(7));
        ASSERT_FALSE(v != s16(7));
        v = std::numeric_limits<s16>::max();
        ASSERT_TRUE(v.positive());
        v = std::numeric_limits<s16>::min();
        ASSERT_FALSE(v.positive());

        v = s8(8);
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == s8(8));
        ASSERT_FALSE(v != s8(8));
        v = std::numeric_limits<s8>::max();
        ASSERT_TRUE(v.positive());
        v = std::numeric_limits<s8>::min();
        ASSERT_FALSE(v.positive());

        v = u64(10u);
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == u64(10u));
        ASSERT_FALSE(v != u64(10u));
        v = std::numeric_limits<u64>::max();
        ASSERT_TRUE(v.positive());

        v = u32{11u};
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == u32{11u});
        ASSERT_FALSE(v != u32{11u});
        v = std::numeric_limits<u32>::max();
        ASSERT_TRUE(v.positive());

        v = u16(12u);
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == u16(12u));
        ASSERT_FALSE(v != u16(12u));
        v = std::numeric_limits<u16>::max();
        ASSERT_TRUE(v.positive());

        v = u8(13u);
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v == u8(13u));
        ASSERT_FALSE(v != u8(13u));
        v = std::numeric_limits<u8>::max();
        ASSERT_TRUE(v.positive());
    }
    { // Floater
        v = 7.7;
        ASSERT_EQ(v.type(), Type::floater);
        ASSERT_TRUE(v == 7.7);
        ASSERT_FALSE(v != 7.7);

        v = 7.7f;
        ASSERT_EQ(v.type(), Type::floater);
        ASSERT_TRUE(v == 7.7f);
        ASSERT_FALSE(v != 7.7f);
    }
    { // Boolean
        v = true;
        ASSERT_EQ(v.type(), Type::boolean);
        ASSERT_TRUE(v == true);
        ASSERT_FALSE(v != true);
    }
    { // Date
        v = Date{};
        ASSERT_EQ(v.type(), Type::date);
        ASSERT_TRUE(v == Date{});
        ASSERT_FALSE(v != Date{});
    }
    { // Time
        v = Time{};
        ASSERT_EQ(v.type(), Type::time);
        ASSERT_TRUE(v == Time{});
        ASSERT_FALSE(v != Time{});
    }
    { // Datetime
        v = Datetime{};
        ASSERT_EQ(v.type(), Type::datetime);
        ASSERT_TRUE(v == Datetime{});
        ASSERT_FALSE(v != Datetime{});
    }
    { // Null
        v = nullptr;
        ASSERT_EQ(v.type(), Type::null);
        ASSERT_TRUE(v == nullptr);
        ASSERT_FALSE(v != nullptr);
    }
}

TEST(Dom, swap)
{
    const Array arrRef(makeArray(0, "a", true));
    Value v1{makeArray(0, "a", true)};
    ASSERT_EQ(v1.type(), Type::array);
    ASSERT_EQ(v1, arrRef);

    Value v2{"hello"s};
    ASSERT_EQ(v2.type(), Type::string);
    ASSERT_EQ(v2, "hello"s);

    std::swap(v1, v2);

    ASSERT_EQ(v1.type(), Type::string);
    ASSERT_EQ(v1, "hello"s);

    ASSERT_EQ(v2.type(), Type::array);
    ASSERT_EQ(v2, arrRef);
}

TEST(Dom, valueTypes)
{
    { // Object
        Value v{Object{}};
        ASSERT_EQ(v.type(), Type::object);
        ASSERT_TRUE(v.object());
    }
    { // Array
        Value v{Array{}};
        ASSERT_EQ(v.type(), Type::array);
        ASSERT_TRUE(v.array());
    }
    { // String
        Value v{"abc"sv};
        ASSERT_EQ(v.type(), Type::string);
        ASSERT_TRUE(v.string());
    }
    { // Character
        Value v{'a'};
        ASSERT_EQ(v.type(), Type::string);
        ASSERT_TRUE(v.string());
    }
    { // Signed integer
        Value v{123};
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v.integer());
    }
    { // Unsigned integer
        Value v{123u};
        ASSERT_EQ(v.type(), Type::integer);
        ASSERT_TRUE(v.integer());
    }
    { // Floater
        Value v{123.0};
        ASSERT_EQ(v.type(), Type::floater);
        ASSERT_TRUE(v.floater());
    }
    { // Boolean
        Value v{false};
        ASSERT_EQ(v.type(), Type::boolean);
        ASSERT_TRUE(v.boolean());
    }
    { // Date
        Value v{Date{}};
        ASSERT_EQ(v.type(), Type::date);
        ASSERT_TRUE(v.date());
    }
    { // Time
        Value v{Time{}};
        ASSERT_EQ(v.type(), Type::time);
        ASSERT_TRUE(v.time());
    }
    { // Datetime
        Value v{Datetime{}};
        ASSERT_EQ(v.type(), Type::datetime);
        ASSERT_TRUE(v.datetime());
        ASSERT_TRUE(v.date());
        ASSERT_TRUE(v.time());
    }
    { // Null
        Value v{nullptr};
        ASSERT_EQ(v.type(), Type::null);
        ASSERT_TRUE(v.null());
    }
}

TEST(Dom, wrongValueType)
{
    ASSERT_FALSE((Value{}.object()));
    ASSERT_FALSE((Value{}.array()));
    ASSERT_FALSE((Value{}.string()));
    ASSERT_FALSE((Value{}.integer()));
    ASSERT_FALSE((Value{}.floater()));
    ASSERT_FALSE((Value{}.boolean()));
    ASSERT_FALSE((Value{}.date()));
    ASSERT_FALSE((Value{}.time()));
    ASSERT_FALSE((Value{}.datetime()));
}

TEST(Dom, density)
{
    ASSERT_EQ(encode(makeArray(1, 2, 3), qcon::multiline), R"([
    1,
    2,
    3
])"s);
    ASSERT_EQ(encode(makeArray(1, 2, 3), qcon::uniline), "[ 1, 2, 3 ]"s);
    ASSERT_EQ(encode(makeArray(1, 2, 3), qcon::nospace), "[1,2,3]"s);
}

TEST(Dom, makeObject)
{
    { // Generic
        Object obj1(makeObject("a", 1, "b"s, 2.0, "c"sv, true));
        Object obj2(makeObject("d", std::move(obj1)));
        ASSERT_EQ(obj2.size(), 1u);
        ASSERT_TRUE(obj2.contains("d"));

        const Object * innerObj{obj2["d"].object()};
        ASSERT_TRUE(innerObj);
        ASSERT_EQ(innerObj->size(), 3u);

        ASSERT_TRUE(innerObj->contains("a"));
        const s64 * aVal{innerObj->at("a").integer()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(*aVal, 1);

        ASSERT_TRUE(innerObj->contains("b"));
        const double * bVal{innerObj->at("b").floater()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(*bVal, 2.0);

        ASSERT_TRUE(innerObj->contains("c"));
        const bool * cVal{innerObj->at("c").boolean()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(*cVal, true);
    }
    { // Empty array
        Object obj(makeObject());
        ASSERT_TRUE(obj.empty());
    }
}

TEST(Dom, makeArray)
{
    { // Generic
        Array arr1(makeArray(1, 2.0, true));
        Array arr2(makeArray("ok", std::move(arr1)));
        ASSERT_EQ(arr2.size(), 2u);
        ASSERT_EQ(arr2.capacity(), 2u);

        const std::string * v1{arr2[0].string()};
        ASSERT_TRUE(v1);
        ASSERT_EQ(*v1, "ok");

        const Array * innerArr{arr2[1].array()};
        ASSERT_TRUE(innerArr);
        ASSERT_EQ(innerArr->size(), 3u);
        ASSERT_EQ(innerArr->capacity(), 3u);

        const s64 * v2{(*innerArr)[0].integer()};
        ASSERT_TRUE(v2);
        ASSERT_EQ(*v2, 1);

        const double * v3{(*innerArr)[1].floater()};
        ASSERT_TRUE(v3);
        ASSERT_EQ(*v3, 2.0);

        const bool * v4{(*innerArr)[2].boolean()};
        ASSERT_TRUE(v4);
        ASSERT_EQ(*v4, true);
    }
    { // Empty array
        Array arr(makeArray());
        ASSERT_TRUE(arr.empty());
        ASSERT_EQ(arr.capacity(), 0u);
    }
}

TEST(Dom, comments)
{
    { // Decode
        const Value qcon{*decode(R"(# AAAAA
# Blah
[ # Blah
    # Blah
    [ # Blah
        # Blah
        0, # Blah
        # Blah
        1 # Blah
        # Blah
    ], # Blah
    # Blah
    { # Blah
        # Blah
        "k1": # Blah
        # Blah
        "v1", # Blah
        # Blah
        "k2": # Blah
        # Blah
        "v2" # Blah
    # Blah
    } # Blah
    # Blah
] # Blah
# Blah)")};
        const Array & rootArr{*qcon.array()};
        ASSERT_EQ(rootArr.size(), 2u);
        const Array & innerArr{*rootArr.at(0).array()};
        ASSERT_EQ(innerArr.size(), 2u);
        const Object & innerObj{*rootArr.at(1).object()};
        ASSERT_EQ(innerObj.size(), 2u);
    }
}

TEST(Dom, numberEquality)
{
    { // Signed integer
        Value val{10};
        ASSERT_TRUE(val == 10);
        ASSERT_TRUE(val == 10u);
        ASSERT_FALSE(val != 10);
        ASSERT_FALSE(val != 10u);
        ASSERT_TRUE(10 == val);
        ASSERT_TRUE(10u == val);
        ASSERT_FALSE(10 != val);
        ASSERT_FALSE(10u != val);
        ASSERT_FALSE(val == 11);
        ASSERT_FALSE(val == 11u);
        ASSERT_TRUE(val != 11);
        ASSERT_TRUE(val != 11u);
        ASSERT_FALSE(11 == val);
        ASSERT_FALSE(11u == val);
        ASSERT_TRUE(11 != val);
        ASSERT_TRUE(11u != val);
    }
    { // Unsigned integer
        Value val{10u};
        ASSERT_TRUE(val == 10);
        ASSERT_TRUE(val == 10u);
        ASSERT_FALSE(val != 10);
        ASSERT_FALSE(val != 10u);
        ASSERT_TRUE(10 == val);
        ASSERT_TRUE(10u == val);
        ASSERT_FALSE(10 != val);
        ASSERT_FALSE(10u != val);
        ASSERT_FALSE(val == 11);
        ASSERT_FALSE(val == 11u);
        ASSERT_TRUE(val != 11);
        ASSERT_TRUE(val != 11u);
        ASSERT_FALSE(11 == val);
        ASSERT_FALSE(11u == val);
        ASSERT_TRUE(11 != val);
        ASSERT_TRUE(11u != val);
    }
    { // Floater
        Value val{10.0};
        ASSERT_TRUE(val == 10.0);
        ASSERT_FALSE(val != 10.0);
        ASSERT_TRUE(10.0 == val);
        ASSERT_FALSE(10.0 != val);
        ASSERT_FALSE(val == 11.0);
        ASSERT_TRUE(val != 11.0);
        ASSERT_FALSE(11.0 == val);
        ASSERT_TRUE(11.0 != val);
    }
    { // Special cases
        Value val{std::numeric_limits<u64>::max()};
        ASSERT_TRUE(val == -1);

        val = -1;
        ASSERT_TRUE(val == std::numeric_limits<u64>::max());

        val = std::numeric_limits<double>::infinity();
        ASSERT_TRUE(val == std::numeric_limits<double>::infinity());

        val = std::numeric_limits<double>::quiet_NaN();
        ASSERT_FALSE(val == std::numeric_limits<double>::quiet_NaN());
        ASSERT_TRUE(val != std::numeric_limits<double>::quiet_NaN());
    }
}

TEST(Dom, general)
{
    const std::string qcon(R"({
    "Dishes": [
        {
            "Gluten Free": false,
            "Ingredients": [
                "\"Salt\"",
                "Barnacles"
            ],
            "Name": "Basket o' Barnacles",
            "Price": 5.45
        },
        {
            "Gluten Free": true,
            "Ingredients": [
                "Tuna"
            ],
            "Name": "Two Tuna",
            "Price": -inf
        },
        {
            "Gluten Free": false,
            "Ingredients": [
                "\"Salt\"",
                "Octopus",
                "Crab"
            ],
            "Name": "18 Leg Bouquet",
            "Price": nan
        }
    ],
    "Employees": [
        {
            "Age": 69,
            "Name": "Ol' Joe Fisher",
            "Title": "Fisherman"
        },
        {
            "Age": 41,
            "Name": "Mark Rower",
            "Title": "Cook"
        },
        {
            "Age": 19,
            "Name": "Phineas",
            "Title": "Server Boy"
        }
    ],
    "Founded": D1964-03-17,
    "Green Eggs and Ham": "I do not like them in a box\n"
                          "I do not like them with a fox\n"
                          "I do not like them in a house\n"
                          "I do not like them with a mouse\n"
                          "I do not like them here or there\n"
                          "I do not like them anywhere\n"
                          "I do not like green eggs and ham\n"
                          "I do not like them Sam I am\n",
    "Ha\x03r Name": "M\0\0n",
    "Last Updated": D2003-06-28T13:59:11.067Z,
    "Magic Numbers": [
        777,
        777,
        777
    ],
    "Name": "Salt's Crust",
    "Opens": T08:30:00,
    "Profit Margin": null
})"s);
    const std::optional<Value> decoded{decode(qcon)};
    ASSERT_TRUE(decoded);
    const std::optional<std::string> encoded{encode(*decoded)};
    ASSERT_EQ(encoded, qcon);
}
