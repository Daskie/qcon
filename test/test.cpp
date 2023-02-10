#include <cmath>

#include <gtest/gtest.h>

#include <qcon.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;

using qcon::Value;
using qcon::Object;
using qcon::Array;
using qcon::decode;
using qcon::encode;
using qcon::Type;
using namespace qcon::tokens;
using qcon::Density;

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
        ASSERT_TRUE(decoded->isString());
        ASSERT_EQ(val, *decoded->asString());
    }
    { // Typical
        std::string_view val{"abc"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isString());
        ASSERT_EQ(val, *decoded->asString());
    }
    { // Printable characters
        std::string_view val{R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isString());
        ASSERT_EQ(val, *decoded->asString());
    }
    { // Escape characters
        std::string_view val{"\b\f\n\r\t"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isString());
        ASSERT_EQ(val, *decoded->asString());
    }
    { // Unicode
        std::string_view val{"\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u000B\u000E\u000F\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F\u007F"sv};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isString());
        ASSERT_EQ(val, *decoded->asString());
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
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Typical
        int val{123};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Max 64
        int64_t val{std::numeric_limits<int64_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Min 64
        int64_t val{std::numeric_limits<int64_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Max 32
        int32_t val{std::numeric_limits<int32_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Min 32
        int32_t val{std::numeric_limits<int32_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Max 16
        int16_t val{std::numeric_limits<int16_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Min 16
        int16_t val{std::numeric_limits<int16_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Max 8
        int8_t val{std::numeric_limits<int8_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
    { // Min 8
        int8_t val{std::numeric_limits<int8_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
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
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, uint64_t(*decoded->asInteger()));
    }
    { // Typical
        unsigned int val{123u};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, uint64_t(*decoded->asInteger()));
    }
    { // Max 64
        uint64_t val{std::numeric_limits<uint64_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, uint64_t(*decoded->asInteger()));
    }
    { // Max 32
        uint32_t val{std::numeric_limits<uint32_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, uint64_t(*decoded->asInteger()));
    }
    { // Max 16
        uint16_t val{std::numeric_limits<uint16_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, uint64_t(*decoded->asInteger()));
    }
    { // Max 8
        uint8_t val{std::numeric_limits<uint8_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isInteger());
        ASSERT_EQ(val, *decoded->asInteger());
    }
}

TEST(qcon, encodeDecodeFloater)
{
    uint64_t val64;
    uint32_t val32;

    { // Zero
        double val{0.0};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Typical
        double val{123.45};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Max integer 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'10000110011'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Max integer 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'10010110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Max 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'11111111110'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Max 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'11111110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Min normal 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'00000000001'0000000000000000000000000000000000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Min normal 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'00000001'00000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Min subnormal 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'00000000000'0000000000000000000000000000000000000000000000000001u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Min subnormal 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'00000000'00000000000000000000001u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Positive infinity
        double val{std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // Negative infinity
        double val{-std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_EQ(val, *decoded->asFloater());
    }
    { // NaN
        const std::optional<std::string> encoded{encode(std::numeric_limits<double>::quiet_NaN())};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isFloater());
        ASSERT_TRUE(std::isnan(*decoded->asFloater()));
    }
}

TEST(qcon, encodeDecodeBoolean)
{
    { // true
        const std::optional<std::string> encoded{encode(true)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isBoolean());
        ASSERT_EQ(true, *decoded->asBoolean());
    }
    { // false
        const std::optional<std::string> encoded{encode(false)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->isBoolean());
        ASSERT_EQ(false, *decoded->asBoolean());
    }
}

TEST(qcon, encodeDecodeNull)
{
    const std::optional<std::string> encoded{encode(nullptr)};
    ASSERT_TRUE(encoded);
    const std::optional<Value> decoded{decode(*encoded)};
    ASSERT_TRUE(decoded);
    ASSERT_TRUE(decoded->isNull());
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
    // Number
    ASSERT_EQ(Type::integer, Value(int64_t(0)).type());
    ASSERT_EQ(Type::integer, Value(int32_t(0)).type());
    ASSERT_EQ(Type::integer, Value(int16_t(0)).type());
    ASSERT_EQ(Type::integer, Value(int8_t(0)).type());
    ASSERT_EQ(Type::integer, Value(uint64_t(0)).type());
    ASSERT_EQ(Type::integer, Value(uint32_t(0)).type());
    ASSERT_EQ(Type::integer, Value(uint16_t(0)).type());
    ASSERT_EQ(Type::integer, Value(uint8_t(0)).type());
    ASSERT_EQ(Type::floater, Value(0.0).type());
    ASSERT_EQ(Type::floater, Value(0.0f).type());
    // Boolean
    ASSERT_EQ(Type::boolean, Value(false).type());
    // Null
    ASSERT_EQ(Type::null, Value(nullptr).type());
}

TEST(qcon, valueMove)
{
    Value v1("abc"sv);
    ASSERT_TRUE(v1.isString());
    ASSERT_EQ("abc"sv, *v1.asString());

    Value v2(std::move(v1));
    ASSERT_TRUE(v1.isNull());
    ASSERT_TRUE(v2.isString());
    ASSERT_EQ("abc"sv, *v2.asString());

    v1 = std::move(v2);
    ASSERT_TRUE(v1.isString());
    ASSERT_TRUE(v2.isNull());
    ASSERT_EQ("abc"sv, *v1.asString());
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

    v = int64_t(5);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == int64_t(5));
    ASSERT_FALSE(v != int64_t(5));

    v = int32_t(6);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == int32_t(6));
    ASSERT_FALSE(v != int32_t(6));

    v = int16_t(7);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == int16_t(7));
    ASSERT_FALSE(v != int16_t(7));

    v = int8_t(8);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == int8_t(8));
    ASSERT_FALSE(v != int8_t(8));

    v = uint64_t(10u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == uint64_t(10u));
    ASSERT_FALSE(v != uint64_t(10u));

    v = uint32_t(11u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == uint32_t(11u));
    ASSERT_FALSE(v != uint32_t(11u));

    v = uint16_t(12u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == uint16_t(12u));
    ASSERT_FALSE(v != uint16_t(12u));

    v = uint8_t(13u);
    ASSERT_EQ(Type::integer, v.type());
    ASSERT_TRUE(v == uint8_t(13u));
    ASSERT_FALSE(v != uint8_t(13u));

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
        Value v(Object{});
        ASSERT_EQ(Type::object, v.type());
        ASSERT_TRUE(v.isObject());
        ASSERT_TRUE(v.asObject());
    }
    { // Array
        Value v(Array{});
        ASSERT_EQ(Type::array, v.type());
        ASSERT_TRUE(v.isArray());
        ASSERT_TRUE(v.asArray());
    }
    { // String
        Value v("abc"sv);
        ASSERT_EQ(Type::string, v.type());
        ASSERT_TRUE(v.isString());
        ASSERT_TRUE(v.asString());
    }
    { // Character
        Value v('a');
        ASSERT_EQ(Type::string, v.type());
        ASSERT_TRUE(v.isString());
        ASSERT_TRUE(v.asString());
    }
    { // Signed integer
        Value v(123);
        ASSERT_EQ(Type::integer, v.type());
        ASSERT_TRUE(v.isInteger());
        ASSERT_TRUE(v.asInteger());
    }
    { // Unsigned integer
        Value v(123u);
        ASSERT_EQ(Type::integer, v.type());
        ASSERT_TRUE(v.isInteger());
        ASSERT_TRUE(v.asInteger());
    }
    { // Floater
        Value v(123.0);
        ASSERT_EQ(Type::floater, v.type());
        ASSERT_TRUE(v.isFloater());
        ASSERT_TRUE(v.asFloater());
    }
    { // Boolean
        Value v(false);
        ASSERT_EQ(Type::boolean, v.type());
        ASSERT_TRUE(v.isBoolean());
        ASSERT_TRUE(v.asBoolean());
    }
    { // Null
        Value v(nullptr);
        ASSERT_EQ(Type::null, v.type());
        ASSERT_TRUE(v.isNull());
    }
}

TEST(qcon, wrongValueType)
{
    // Safe
    ASSERT_FALSE((Value().asObject()));
    ASSERT_FALSE((Value().asArray()));
    ASSERT_FALSE((Value().asString()));
    ASSERT_FALSE((Value().asInteger()));
    ASSERT_FALSE((Value().asFloater()));
    ASSERT_FALSE((Value().asBoolean()));
}

TEST(qcon, density)
{
    ASSERT_EQ(R"([
    1,
    2,
    3
])"s, encode(makeArray(1, 2, 3), Density::multiline));
    ASSERT_EQ("[ 1, 2, 3 ]"s, encode(makeArray(1, 2, 3), Density::uniline));
    ASSERT_EQ("[1,2,3]"s, encode(makeArray(1, 2, 3), Density::nospace));
}

TEST(qcon, makeObject)
{
    { // Generic
        Object obj1{makeObject("a", 1, "b"s, 2.0, "c"sv, true)};
        Object obj2{makeObject("d", std::move(obj1))};
        ASSERT_EQ(1u, obj2.size());
        ASSERT_TRUE(obj2.contains("d"));

        const Object * innerObj{obj2["d"].asObject()};
        ASSERT_TRUE(innerObj);
        ASSERT_EQ(3u, innerObj->size());

        ASSERT_TRUE(innerObj->contains("a"));
        const int64_t * aVal{innerObj->at("a").asInteger()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(1, *aVal);

        ASSERT_TRUE(innerObj->contains("b"));
        const double * bVal{innerObj->at("b").asFloater()};
        ASSERT_TRUE(aVal);
        ASSERT_EQ(2.0, *bVal);

        ASSERT_TRUE(innerObj->contains("c"));
        const bool * cVal{innerObj->at("c").asBoolean()};
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

        const std::string * v1{arr2[0].asString()};
        ASSERT_TRUE(v1);
        ASSERT_EQ("ok", *v1);

        const Array * innerArr{arr2[1].asArray()};
        ASSERT_TRUE(innerArr);
        ASSERT_EQ(3u, innerArr->size());
        ASSERT_EQ(3u, innerArr->capacity());

        const int64_t * v2{(*innerArr)[0].asInteger()};
        ASSERT_TRUE(v2);
        ASSERT_EQ(1, *v2);

        const double * v3{(*innerArr)[1].asFloater()};
        ASSERT_TRUE(v3);
        ASSERT_EQ(2.0, *v3);

        const bool * v4{(*innerArr)[2].asBoolean()};
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
        const Array & rootArr{*qcon.asArray()};
        ASSERT_EQ(2u, rootArr.size());
        const Array & innerArr{*rootArr.at(0).asArray()};
        ASSERT_EQ(2u, innerArr.size());
        const Object & innerObj{*rootArr.at(1).asObject()};
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
        Value val{std::numeric_limits<uint64_t>::max()};
        ASSERT_TRUE(val == -1);

        val = -1;
        ASSERT_TRUE(val == std::numeric_limits<uint64_t>::max());

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
    "Founded": 1964,
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
    ASSERT_EQ(qcon, encode(*decode(qcon)));
}
