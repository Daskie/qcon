#include <qcon.hpp>

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
using qcon::Datetime;
using qcon::decode;
using qcon::encode;

using qcon::makeObject;
using qcon::makeArray;

TEST(qcon, encodeDecodeString)
{
    { // Empty
        const std::string_view val{""sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(val, *decoded->string());
    }
    { // Typical
        std::string_view val{"abc"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(val, *decoded->string());
    }
    { // Printable characters
        std::string_view val{R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(val, *decoded->string());
    }
    { // Escape characters
        std::string_view val{"\b\f\n\r\t"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(val, *decoded->string());
    }
    { // Unicode
        std::string_view val{"\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u000B\u000E\u000F\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F\u007F"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->string());
        ASSERT_EQ(val, *decoded->string());
    }
}

TEST(qcon, encodeDecodeSignedInteger)
{
    { // Zero
        int val{0};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Typical
        int val{123};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Max 64
        s64 val{std::numeric_limits<s64>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Min 64
        s64 val{std::numeric_limits<s64>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Max 32
        s32 val{std::numeric_limits<s32>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Min 32
        s32 val{std::numeric_limits<s32>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Max 16
        s16 val{std::numeric_limits<s16>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Min 16
        s16 val{std::numeric_limits<s16>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Max 8
        s8 val{std::numeric_limits<s8>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
    { // Min 8
        s8 val{std::numeric_limits<s8>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
}

TEST(qcon, encodeDecodeUnsignedInteger)
{
    { // Zero
        unsigned int val{0u};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, u64(*decoded->integer()));
    }
    { // Typical
        unsigned int val{123u};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, u64(*decoded->integer()));
    }
    { // Max 64
        u64 val{std::numeric_limits<u64>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, u64(*decoded->integer()));
    }
    { // Max 32
        u32 val{std::numeric_limits<u32>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, u64(*decoded->integer()));
    }
    { // Max 16
        u16 val{std::numeric_limits<u16>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, u64(*decoded->integer()));
    }
    { // Max 8
        u8 val{std::numeric_limits<u8>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->integer());
        ASSERT_EQ(val, *decoded->integer());
    }
}

TEST(qcon, encodeDecodeFloater)
{
    u64 val64;
    u32 val32;

    { // Zero
        double val{0.0};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Typical
        double val{123.45};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Max integer 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'10000110011'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Max integer 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'10010110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Max 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'11111111110'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Max 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'11111110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Min normal 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'00000000001'0000000000000000000000000000000000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Min normal 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'00000001'00000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Min subnormal 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'00000000000'0000000000000000000000000000000000000000000000000001u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Min subnormal 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'00000000'00000000000000000000001u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Positive infinity
        double val{std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
    }
    { // Negative infinity
        double val{-std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->floater());
        ASSERT_EQ(val, *decoded->floater());
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

TEST(qcon, encodeDecodeBoolean)
{
    { // true
        const std::optional<std::string> encoded{encode(true)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->boolean());
        ASSERT_EQ(true, *decoded->boolean());
    }
    { // false
        const std::optional<std::string> encoded{encode(false)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->boolean());
        ASSERT_EQ(false, *decoded->boolean());
    }
}

TEST(qcon, encodeDecodeDatetime)
{
    { // Epoch
        const Datetime dt{};
        const std::optional<std::string> encoded{encode(dt)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->datetime());
        ASSERT_EQ(*decoded->datetime(), dt);
    }
    { // Current time
        const Datetime dt{std::chrono::system_clock::now()};
        const std::optional<std::string> encoded{encode(dt)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->datetime());
        ASSERT_EQ(*decoded->datetime(), dt);
    }
}

TEST(qcon, encodeDecodeNull)
{
    const std::optional<std::string> encoded{encode(nullptr)};
    ASSERT_TRUE(encoded);
    const std::optional<Value> decoded{decode(*encoded)};
    ASSERT_TRUE(decoded);
    ASSERT_TRUE(decoded->null());
}

TEST(qcon, valueConstruction)
{
    // Default
    ASSERT_EQ(Type::null, Value().type());

    // Object
    ASSERT_EQ(Type::object, Value(Object()).type());

    // Array
    ASSERT_EQ(Type::array, Value(Array()).type());

    // String
    ASSERT_EQ(Type::string, Value("abc"sv).type());
    ASSERT_EQ(Type::string, Value("abc"s).type());
    ASSERT_EQ(Type::string, Value("abc").type());
    ASSERT_EQ(Type::string, Value(const_cast<char *>("abc")).type());
    ASSERT_EQ(Type::string, Value('a').type());

    // Integer
    ASSERT_EQ(Type::integer, Value(s64(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<s64>::max()).positive());
    ASSERT_FALSE(Value(std::numeric_limits<s64>::min()).positive());
    ASSERT_EQ(Type::integer, Value(s32(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<s32>::max()).positive());
    ASSERT_FALSE(Value(std::numeric_limits<s32>::min()).positive());
    ASSERT_EQ(Type::integer, Value(s16(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<s16>::max()).positive());
    ASSERT_FALSE(Value(std::numeric_limits<s16>::min()).positive());
    ASSERT_EQ(Type::integer, Value(s8(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<s8>::max()).positive());
    ASSERT_FALSE(Value(std::numeric_limits<s8>::min()).positive());
    ASSERT_EQ(Type::integer, Value(u64(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<u64>::max()).positive());
    ASSERT_EQ(Type::integer, Value(u32(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<u32>::max()).positive());
    ASSERT_EQ(Type::integer, Value(u16(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<u16>::max()).positive());
    ASSERT_EQ(Type::integer, Value(u8(0)).type());
    ASSERT_TRUE(Value(std::numeric_limits<u8>::max()).positive());

    // Floater
    ASSERT_EQ(Type::floater, Value(0.0).type());
    ASSERT_TRUE(Value(1.0).positive());
    ASSERT_FALSE(Value(-1.0).positive());
    ASSERT_EQ(Type::floater, Value(0.0f).type());
    ASSERT_TRUE(Value(1.0f).positive());
    ASSERT_FALSE(Value(-1.0f).positive());

    // Boolean
    ASSERT_EQ(Type::boolean, Value(false).type());

    // Datetime
    ASSERT_EQ(Type::datetime, Value(Datetime{}).type());

    // Null
    ASSERT_EQ(Type::null, Value(nullptr).type());
}

TEST(qcon, valueMove)
{
    Value v1("abc"sv);
    ASSERT_TRUE(v1.string());
    ASSERT_EQ("abc"sv, *v1.string());

    Value v2(std::move(v1));
    ASSERT_TRUE(v1.null());
    ASSERT_TRUE(v2.string());
    ASSERT_EQ("abc"sv, *v2.string());

    v1 = std::move(v2);
    ASSERT_TRUE(v1.string());
    ASSERT_TRUE(v2.null());
    ASSERT_EQ("abc"sv, *v1.string());
}

TEST(qcon, valueAssignAndEquality)
{
    Value v{};

    const Object objRef{makeObject("a", 1, "b", "wow", "c", nullptr)};
    v = makeObject("a", 1, "b", "wow", "c", nullptr);
    ASSERT_EQ(Type::object, v.type());
    ASSERT_TRUE(v == objRef);
    ASSERT_FALSE(v != objRef);

    const Array arrRef{makeArray(0, "a", true)};
    v = makeArray(0, "a", true);
    ASSERT_EQ(Type::array, v.type());
    ASSERT_TRUE(v == arrRef);
    ASSERT_FALSE(v != arrRef);

    v = "hello"s;
    ASSERT_EQ(Type::string, v.type());
    ASSERT_TRUE(v == "hello"s);
    ASSERT_FALSE(v != "hello"s);

    v = "hellu"sv;
    ASSERT_EQ(Type::string, v.type());
    ASSERT_TRUE(v == "hellu"sv);
    ASSERT_FALSE(v != "hellu"sv);

    v = "helli";
    ASSERT_EQ(Type::string, v.type());
    ASSERT_TRUE(v == "helli");
    ASSERT_FALSE(v != "helli");

    v = const_cast<char *>("hella");
    ASSERT_EQ(Type::string, v.type());
    ASSERT_TRUE(v == const_cast<char *>("hella"));
    ASSERT_FALSE(v != const_cast<char *>("hella"));

    v = 'h';
    ASSERT_EQ(Type::string, v.type());
    ASSERT_TRUE(v == 'h');
    ASSERT_FALSE(v != 'h');

    v = s64(5);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == s64(5));
    ASSERT_FALSE(v != s64(5));
    v = std::numeric_limits<s64>::max();
    ASSERT_TRUE(v.positive());
    v = std::numeric_limits<s64>::min();
    ASSERT_FALSE(v.positive());

    v = s32(6);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == s32(6));
    ASSERT_FALSE(v != s32(6));
    v = std::numeric_limits<s32>::max();
    ASSERT_TRUE(v.positive());
    v = std::numeric_limits<s32>::min();
    ASSERT_FALSE(v.positive());

    v = s16(7);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == s16(7));
    ASSERT_FALSE(v != s16(7));
    v = std::numeric_limits<s16>::max();
    ASSERT_TRUE(v.positive());
    v = std::numeric_limits<s16>::min();
    ASSERT_FALSE(v.positive());

    v = s8(8);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == s8(8));
    ASSERT_FALSE(v != s8(8));
    v = std::numeric_limits<s8>::max();
    ASSERT_TRUE(v.positive());
    v = std::numeric_limits<s8>::min();
    ASSERT_FALSE(v.positive());

    v = u64(10u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == u64(10u));
    ASSERT_FALSE(v != u64(10u));
    v = std::numeric_limits<u64>::max();
    ASSERT_TRUE(v.positive());

    v = u32(11u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == u32(11u));
    ASSERT_FALSE(v != u32(11u));
    v = std::numeric_limits<u32>::max();
    ASSERT_TRUE(v.positive());

    v = u16(12u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == u16(12u));
    ASSERT_FALSE(v != u16(12u));
    v = std::numeric_limits<u16>::max();
    ASSERT_TRUE(v.positive());

    v = u8(13u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == u8(13u));
    ASSERT_FALSE(v != u8(13u));
    v = std::numeric_limits<u8>::max();
    ASSERT_TRUE(v.positive());

    v = 7.7;
    ASSERT_EQ(Type::floater, v.type());
    ASSERT_TRUE(v == 7.7);
    ASSERT_FALSE(v != 7.7);

    v = 7.7f;
    ASSERT_EQ(Type::floater, v.type());
    ASSERT_TRUE(v == 7.7f);
    ASSERT_FALSE(v != 7.7f);

    v = true;
    ASSERT_EQ(Type::boolean, v.type());
    ASSERT_TRUE(v == true);
    ASSERT_FALSE(v != true);

    v = Datetime{};
    ASSERT_EQ(Type::datetime, v.type());
    ASSERT_TRUE(v == Datetime{});
    ASSERT_FALSE(v != Datetime{});

    v = nullptr;
    ASSERT_EQ(Type::null, v.type());
    ASSERT_TRUE(v == nullptr);
    ASSERT_FALSE(v != nullptr);
}

TEST(qcon, swap)
{
    const Array arrRef{makeArray(0, "a", true)};
    Value v1{makeArray(0, "a", true)};
    ASSERT_EQ(Type::array, v1.type());
    ASSERT_EQ(arrRef, v1);

    Value v2{"hello"s};
    ASSERT_EQ(Type::string, v2.type());
    ASSERT_EQ("hello"s, v2);

    std::swap(v1, v2);

    ASSERT_EQ(Type::string, v1.type());
    ASSERT_EQ("hello"s, v1);

    ASSERT_EQ(Type::array, v2.type());
    ASSERT_EQ(arrRef, v2);
}

TEST(qcon, valueTypes)
{
    { // Object
        Value v{Object{}};
        ASSERT_EQ(Type::object, v.type());
        ASSERT_TRUE(v.object());
    }
    { // Array
        Value v{Array{}};
        ASSERT_EQ(Type::array, v.type());
        ASSERT_TRUE(v.array());
    }
    { // String
        Value v{"abc"sv};
        ASSERT_EQ(Type::string, v.type());
        ASSERT_TRUE(v.string());
    }
    { // Character
        Value v{'a'};
        ASSERT_EQ(Type::string, v.type());
        ASSERT_TRUE(v.string());
    }
    { // Signed integer
        Value v{123};
        ASSERT_EQ(Type::integer, v.type());
        ASSERT_TRUE(v.integer());
    }
    { // Unsigned integer
        Value v{123u};
        ASSERT_EQ(Type::integer, v.type());
        ASSERT_TRUE(v.integer());
    }
    { // Floater
        Value v{123.0};
        ASSERT_EQ(Type::floater, v.type());
        ASSERT_TRUE(v.floater());
    }
    { // Boolean
        Value v{false};
        ASSERT_EQ(Type::boolean, v.type());
        ASSERT_TRUE(v.boolean());
    }
    { // Datetime
        Value v{Datetime{}};
        ASSERT_EQ(Type::datetime, v.type());
        ASSERT_TRUE(v.datetime());
    }
    { // Null
        Value v{nullptr};
        ASSERT_EQ(Type::null, v.type());
        ASSERT_TRUE(v.null());
    }
}

TEST(qcon, wrongValueType)
{
    ASSERT_FALSE((Value{}.object()));
    ASSERT_FALSE((Value{}.array()));
    ASSERT_FALSE((Value{}.string()));
    ASSERT_FALSE((Value{}.integer()));
    ASSERT_FALSE((Value{}.floater()));
    ASSERT_FALSE((Value{}.boolean()));
    ASSERT_FALSE((Value{}.datetime()));
}

TEST(qcon, density)
{
    ASSERT_EQ(R"([
    1,
    2,
    3
])"s, encode(makeArray(1, 2, 3), qcon::multiline));
    ASSERT_EQ("[ 1, 2, 3 ]"s, encode(makeArray(1, 2, 3), qcon::uniline));
    ASSERT_EQ("[1,2,3]"s, encode(makeArray(1, 2, 3), qcon::nospace));
}

TEST(qcon, makeObject)
{
    { // Generic
        Object obj1{makeObject("a", 1, "b"s, 2.0, "c"sv, true)};
        Object obj2{makeObject("d", std::move(obj1))};
        ASSERT_EQ(1u, obj2.size());
        ASSERT_TRUE(obj2.contains("d"));

        const Object * innerObj{obj2["d"].object()};
        ASSERT_TRUE(innerObj);
        ASSERT_EQ(3u, innerObj->size());

        ASSERT_TRUE(innerObj->contains("a"));
        const s64 * aVal{innerObj->at("a").integer()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(1, *aVal);

        ASSERT_TRUE(innerObj->contains("b"));
        const double * bVal{innerObj->at("b").floater()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(2.0, *bVal);

        ASSERT_TRUE(innerObj->contains("c"));
        const bool * cVal{innerObj->at("c").boolean()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(true, *cVal);
    }
    { // Empty array
        Object obj{makeObject()};
        ASSERT_TRUE(obj.empty());
    }
}

TEST(qcon, makeArray)
{
    { // Generic
        Array arr1{makeArray(1, 2.0, true)};
        Array arr2{makeArray("ok", std::move(arr1))};
        ASSERT_EQ(2u, arr2.size());
        ASSERT_EQ(2u, arr2.capacity());

        const std::string * v1{arr2[0].string()};
        ASSERT_TRUE(v1);
        ASSERT_EQ("ok", *v1);

        const Array * innerArr{arr2[1].array()};
        ASSERT_TRUE(innerArr);
        ASSERT_EQ(3u, innerArr->size());
        ASSERT_EQ(3u, innerArr->capacity());

        const s64 * v2{(*innerArr)[0].integer()};
        ASSERT_TRUE(v2);
        ASSERT_EQ(1, *v2);

        const double * v3{(*innerArr)[1].floater()};
        ASSERT_TRUE(v3);
        ASSERT_EQ(2.0, *v3);

        const bool * v4{(*innerArr)[2].boolean()};
        ASSERT_TRUE(v4);
        ASSERT_EQ(true, *v4);
    }
    { // Empty array
        Array arr{makeArray()};
        ASSERT_TRUE(arr.empty());
        ASSERT_EQ(0u, arr.capacity());
    }
}

TEST(qcon, comments)
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
# Blah)"sv)};
        const Array & rootArr{*qcon.array()};
        ASSERT_EQ(2u, rootArr.size());
        const Array & innerArr{*rootArr.at(0).array()};
        ASSERT_EQ(2u, innerArr.size());
        const Object & innerObj{*rootArr.at(1).object()};
        ASSERT_EQ(2u, innerObj.size());
    }
}

TEST(qcon, numberEquality)
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

TEST(qcon, general)
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
    "Founded": D1964-03-17T13:59:11Z,
    "Green Eggs and Ham": "I do not like them in a box\nI do not like them with a fox\nI do not like them in a house\nI do not like them with a mouse\nI do not like them here or there\nI do not like them anywhere\nI do not like green eggs and ham\nI do not like them Sam I am\n",
    "Ha\x03r Name": "M\0\0n",
    "Magic Numbers": [
        777,
        777,
        777
    ],
    "Name": "Salt's Crust",
    "Profit Margin": null
})"s);
    const std::optional<Value> decoded{decode(qcon)};
    ASSERT_TRUE(decoded);
    const std::optional<std::string> encoded{encode(*decoded)};
    ASSERT_EQ(qcon, encoded);
}
