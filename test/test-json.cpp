#include <cmath>

#include <gtest/gtest.h>

#include <qc-json.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;

using qc::json::Value;
using qc::json::Object;
using qc::json::Array;
using qc::json::decode;
using qc::json::encode;
using qc::json::Type;
using namespace qc::json::tokens;
using qc::json::Density;

using qc::json::makeObject;
using qc::json::makeArray;

struct CustomVal { int x, y; };

bool operator==(const CustomVal & cv1, const CustomVal & cv2)
{
    return cv1.x == cv2.x && cv1.y == cv2.y;
}

template <>
struct qc::json::ValueTo<CustomVal>
{
    std::optional<CustomVal> operator()(const Value & val) const
    {
        const Array * const arr{val.asArray()};
        if (arr)
        {
            const std::optional<int> v1{(*arr)[0].get<int>()};
            const std::optional<int> v2{(*arr)[1].get<int>()};
            if (v1 && v2)
            {
                return CustomVal{*v1, *v2};
            }
        }
        return {};
    }
};

template <>
struct qc::json::ValueFrom<CustomVal>
{
    Value operator()(const CustomVal & v) const
    {
        return makeArray(v.x, v.y);
    }
};

TEST(json, encodeDecodeString)
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

TEST(json, encodeDecodeSignedInteger)
{
    { // Zero
        int val{0};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int>());
        ASSERT_EQ(val, *decoded->get<int>());
    }
    { // Typical
        int val{123};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int>());
        ASSERT_EQ(val, *decoded->get<int>());
    }
    { // Max 64
        int64_t val{std::numeric_limits<int64_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int64_t>());
        ASSERT_EQ(val, *decoded->get<int64_t>());
    }
    { // Min 64
        int64_t val{std::numeric_limits<int64_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int64_t>());
        ASSERT_EQ(val, *decoded->get<int64_t>());
    }
    { // Max 32
        int32_t val{std::numeric_limits<int32_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int32_t>());
        ASSERT_EQ(val, *decoded->get<int32_t>());
    }
    { // Min 32
        int32_t val{std::numeric_limits<int32_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int32_t>());
        ASSERT_EQ(val, *decoded->get<int32_t>());
    }
    { // Max 16
        int16_t val{std::numeric_limits<int16_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int16_t>());
        ASSERT_EQ(val, *decoded->get<int16_t>());
    }
    { // Min 16
        int16_t val{std::numeric_limits<int16_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int16_t>());
        ASSERT_EQ(val, *decoded->get<int16_t>());
    }
    { // Max 8
        int8_t val{std::numeric_limits<int8_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int8_t>());
        ASSERT_EQ(val, *decoded->get<int8_t>());
    }
    { // Min 8
        int8_t val{std::numeric_limits<int8_t>::min()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<int8_t>());
        ASSERT_EQ(val, *decoded->get<int8_t>());
    }
}

TEST(json, encodeDecodeUnsignedInteger)
{
    { // Zero
        unsigned int val{0u};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<unsigned int>());
        ASSERT_EQ(val, *decoded->get<unsigned int>());
    }
    { // Typical
        unsigned int val{123u};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<unsigned int>());
        ASSERT_EQ(val, *decoded->get<unsigned int>());
    }
    { // Max 64
        uint64_t val{std::numeric_limits<uint64_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<uint64_t>());
        ASSERT_EQ(val, *decoded->get<uint64_t>());
    }
    { // Max 32
        uint32_t val{std::numeric_limits<uint32_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<uint32_t>());
        ASSERT_EQ(val, *decoded->get<uint32_t>());
    }
    { // Max 16
        uint16_t val{std::numeric_limits<uint16_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<uint16_t>());
        ASSERT_EQ(val, *decoded->get<uint16_t>());
    }
    { // Max 8
        uint8_t val{std::numeric_limits<uint8_t>::max()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<uint8_t>());
        ASSERT_EQ(val, *decoded->get<uint8_t>());
    }
}

TEST(json, encodeDecodeFloater)
{
    uint64_t val64;
    uint32_t val32;

    { // Zero
        double val{0.0};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Typical
        double val{123.45};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Max integer 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'10000110011'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Max integer 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'10010110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Max 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'11111111110'1111111111111111111111111111111111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Max 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'11111110'11111111111111111111111u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Min normal 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'00000000001'0000000000000000000000000000000000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Min normal 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'00000001'00000000000000000000000u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Min subnormal 64
        double val{reinterpret_cast<const double &>(val64 = 0b0'00000000000'0000000000000000000000000000000000000000000000000001u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Min subnormal 32
        double val{reinterpret_cast<const float &>(val32 = 0b0'00000000'00000000000000000000001u)};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Positive infinity
        double val{std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // Negative infinity
        double val{-std::numeric_limits<double>::infinity()};
        const std::optional<std::string> encoded{encode(val)};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_EQ(val, *decoded->get<double>());
    }
    { // NaN
        const std::optional<std::string> encoded{encode(std::numeric_limits<double>::quiet_NaN())};
        ASSERT_TRUE(encoded);
        const std::optional<Value> decoded{decode(*encoded)};
        ASSERT_TRUE(decoded);
        ASSERT_TRUE(decoded->is<double>());
        ASSERT_TRUE(std::isnan(*decoded->get<double>()));
    }
}

TEST(json, encodeDecodeBoolean)
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

TEST(json, encodeDecodeNull)
{
    const std::optional<std::string> encoded{encode(nullptr)};
    ASSERT_TRUE(encoded);
    const std::optional<Value> decoded{decode(*encoded)};
    ASSERT_TRUE(decoded);
    ASSERT_TRUE(decoded->isNull());
}

TEST(json, encodeDecodeCustom)
{
    CustomVal val{1, 2};
    const std::optional<std::string> encoded{encode(val)};
    ASSERT_TRUE(encoded);
    const std::optional<Value> decoded{decode(*encoded)};
    ASSERT_TRUE(decoded);
    const std::optional<CustomVal> decodedVal{decoded->get<CustomVal>()};
    ASSERT_TRUE(decodedVal);
    ASSERT_EQ(val, *decodedVal);
}

TEST(json, valueToFromCustom)
{
    CustomVal c{1, 2};

    Value cVal{c};
    ASSERT_EQ(Type::array, cVal.type());
    const Array * arr{cVal.asArray()};
    ASSERT_TRUE(arr);
    ASSERT_EQ(2u, arr->size());
    ASSERT_EQ(1, (*arr)[0]);
    ASSERT_EQ(2, (*arr)[1]);

    cVal = nullptr;
    ASSERT_EQ(Type::null, cVal.type());

    cVal = c;
    ASSERT_EQ(Type::array, cVal.type());
    arr = cVal.asArray();
    ASSERT_TRUE(arr);
    ASSERT_EQ(2u, arr->size());
    ASSERT_EQ(1, (*arr)[0]);
    ASSERT_EQ(2, (*arr)[1]);

    const std::optional<CustomVal> c2{cVal.get<CustomVal>()};
    ASSERT_TRUE(c2);
    ASSERT_EQ(c, *c2);

    ASSERT_EQ(c, cVal);
}

TEST(json, valueConstruction)
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
    ASSERT_EQ(Type::unsigner, Value(uint64_t(0)).type());
    ASSERT_EQ(Type::unsigner, Value(uint32_t(0)).type());
    ASSERT_EQ(Type::unsigner, Value(uint16_t(0)).type());
    ASSERT_EQ(Type::unsigner, Value(uint8_t(0)).type());
    ASSERT_EQ(Type::floater, Value(0.0).type());
    ASSERT_EQ(Type::floater, Value(0.0f).type());
    // Boolean
    ASSERT_EQ(Type::boolean, Value(false).type());
    // Null
    ASSERT_EQ(Type::null, Value(nullptr).type());
}

TEST(json, valueMove)
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

TEST(json, valueAssignAndEquality)
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
    ASSERT_EQ(Type::unsigner, v.type());
    ASSERT_TRUE(v == uint64_t(10u));
    ASSERT_FALSE(v != uint64_t(10u));

    v = uint32_t(11u);
    ASSERT_EQ(Type::unsigner, v.type());
    ASSERT_TRUE(v == uint32_t(11u));
    ASSERT_FALSE(v != uint32_t(11u));

    v = uint16_t(12u);
    ASSERT_EQ(Type::unsigner, v.type());
    ASSERT_TRUE(v == uint16_t(12u));
    ASSERT_FALSE(v != uint16_t(12u));

    v = uint8_t(13u);
    ASSERT_EQ(Type::unsigner, v.type());
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

TEST(json, swap)
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

TEST(json, valueTypes)
{
    { // Object
        Value v(Object{}, Density::uniline);
        ASSERT_EQ(Type::object, v.type());
        ASSERT_EQ(Density::uniline, v.density());
        ASSERT_TRUE(v.isObject());
        ASSERT_TRUE(v.is<Object>());
        ASSERT_TRUE(v.asObject());
    }
    { // Array
        Value v(Array{}, Density::multiline);
        ASSERT_EQ(Type::array, v.type());
        ASSERT_EQ(Density::multiline, v.density());
        ASSERT_TRUE(v.isArray());
        ASSERT_TRUE(v.is<Array>());
        ASSERT_TRUE(v.asArray());
    }
    { // String
        Value v("abc"sv);
        ASSERT_EQ(Type::string, v.type());
        ASSERT_EQ(Density::unspecified, v.density());
        ASSERT_TRUE(v.isString());
        ASSERT_TRUE(v.is<std::string>());
        ASSERT_TRUE(v.is<std::string_view>());
        ASSERT_TRUE(v.is<const char *>());
        ASSERT_TRUE(v.is<char *>());
        ASSERT_FALSE(v.is<char>());
        ASSERT_TRUE(v.asString());
        ASSERT_TRUE(v.get<std::string_view>());
        ASSERT_TRUE(v.get<const char *>());
    }
    { // Character
        Value v('a');
        ASSERT_EQ(Type::string, v.type());
        ASSERT_EQ(Density::unspecified, v.density());
        ASSERT_TRUE(v.isString());
        ASSERT_TRUE(v.is<std::string>());
        ASSERT_TRUE(v.is<std::string_view>());
        ASSERT_TRUE(v.is<const char *>());
        ASSERT_TRUE(v.is<char *>());
        ASSERT_TRUE(v.is<char>());
        ASSERT_TRUE(v.asString());
        ASSERT_TRUE(v.get<std::string_view>());
        ASSERT_TRUE(v.get<const char *>());
        ASSERT_TRUE(v.get<char>());
    }
    { // Signed integer
        Value v(123);
        ASSERT_EQ(Type::integer, v.type());
        ASSERT_EQ(Density::unspecified, v.density());
        ASSERT_TRUE(v.isNumber());
        ASSERT_TRUE(v.isInteger());
        ASSERT_TRUE(v.is<int>());
        ASSERT_TRUE(v.asInteger());
        ASSERT_TRUE(v.get<int>());
    }
    { // Unsigned integer
        Value v(123u);
        ASSERT_EQ(Type::unsigner, v.type());
        ASSERT_EQ(Density::unspecified, v.density());
        ASSERT_TRUE(v.isNumber());
        ASSERT_TRUE(v.isUnsigner());
        ASSERT_TRUE(v.is<unsigned int>());
        ASSERT_TRUE(v.asUnsigner());
        ASSERT_TRUE(v.get<int>());
    }
    { // Floater
        Value v(123.0);
        ASSERT_EQ(Type::floater, v.type());
        ASSERT_EQ(Density::unspecified, v.density());
        ASSERT_TRUE(v.isNumber());
        ASSERT_TRUE(v.isFloater());
        ASSERT_TRUE(v.is<float>());
        ASSERT_TRUE(v.asFloater());
        ASSERT_TRUE(v.get<float>());
    }
    { // Boolean
        Value v(false);
        ASSERT_EQ(Type::boolean, v.type());
        ASSERT_EQ(Density::unspecified, v.density());
        ASSERT_TRUE(v.isBoolean());
        ASSERT_TRUE(v.is<bool>());
        ASSERT_TRUE(v.asBoolean());
        ASSERT_TRUE(v.get<bool>());
    }
    { // Null
        Value v(nullptr);
        ASSERT_EQ(Type::null, v.type());
        ASSERT_EQ(Density::unspecified, v.density());
        ASSERT_TRUE(v.isNull());
        ASSERT_TRUE(v.get<nullptr_t>());
    }
}

template <typename T>
void testNumber(T v, bool isS64, bool isS32, bool isS16, bool isS08, bool isU64, bool isU32, bool isU16, bool isU08, bool isF64, bool isF32)
{
    Value val(v);

    ASSERT_EQ(isS64, val.is< int64_t>());
    ASSERT_EQ(isS32, val.is< int32_t>());
    ASSERT_EQ(isS16, val.is< int16_t>());
    ASSERT_EQ(isS08, val.is<  int8_t>());
    ASSERT_EQ(isU64, val.is<uint64_t>());
    ASSERT_EQ(isU32, val.is<uint32_t>());
    ASSERT_EQ(isU16, val.is<uint16_t>());
    ASSERT_EQ(isU08, val.is< uint8_t>());
    ASSERT_EQ(isF64, val.is<  double>());
    ASSERT_EQ(isF32, val.is<   float>());

    if (isS64) ASSERT_EQ( int64_t(v), *val.get< int64_t>()); else ASSERT_FALSE(val.get< int64_t>());
    if (isS32) ASSERT_EQ( int32_t(v), *val.get< int32_t>()); else ASSERT_FALSE(val.get< int32_t>());
    if (isS16) ASSERT_EQ( int16_t(v), *val.get< int16_t>()); else ASSERT_FALSE(val.get< int16_t>());
    if (isS08) ASSERT_EQ(  int8_t(v), *val.get<  int8_t>()); else ASSERT_FALSE(val.get<  int8_t>());
    if (isU64) ASSERT_EQ(uint64_t(v), *val.get<uint64_t>()); else ASSERT_FALSE(val.get<uint64_t>());
    if (isU32) ASSERT_EQ(uint32_t(v), *val.get<uint32_t>()); else ASSERT_FALSE(val.get<uint32_t>());
    if (isU16) ASSERT_EQ(uint16_t(v), *val.get<uint16_t>()); else ASSERT_FALSE(val.get<uint16_t>());
    if (isU08) ASSERT_EQ( uint8_t(v), *val.get< uint8_t>()); else ASSERT_FALSE(val.get< uint8_t>());
    if (isF64) ASSERT_EQ(  double(v), *val.get<  double>()); else ASSERT_FALSE(val.get<  double>());
    if (isF32) ASSERT_EQ(   float(v), *val.get<   float>()); else ASSERT_FALSE(val.get<   float>());
}

TEST(json, valueNumbers)
{
    // Zero, given as signed integer
    testNumber(0, true, true, true, true, true, true, true, true, true, true);
    // Zero, given as unsigned integer
    testNumber(0u, true, true, true, true, true, true, true, true, true, true);
    // Zero, given as floater
    testNumber(0.0, true, true, true, true, true, true, true, true, true, true);
    // Positive integer, given as signed integer
    testNumber(127, true, true, true, true, true, true, true, true, true, true);
    // Positive integer, given as unsigned integer
    testNumber(127u, true, true, true, true, true, true, true, true, true, true);
    // Positive integer, given as floater
    testNumber(127.0, true, true, true, true, true, true, true, true, true, true);
    // Positive integer too big for int8_t, given as signed integer
    testNumber(128, true, true, true, false, true, true, true, true, true, true);
    // Positive integer too big for int8_t, given as unsigned integer
    testNumber(128u, true, true, true, false, true, true, true, true, true, true);
    // Positive integer too big for int8_t, given as floater
    testNumber(128.0, true, true, true, false, true, true, true, true, true, true);
    // Positive integer too big for uint8_t, given as signed integer
    testNumber(256, true, true, true, false, true, true, true, false, true, true);
    // Positive integer too big for uint8_t, given as unsigned integer
    testNumber(256u, true, true, true, false, true, true, true, false, true, true);
    // Positive integer too big for uint8_t, given as floater
    testNumber(256.0, true, true, true, false, true, true, true, false, true, true);
    // Positive integer too big for int16_t, given as signed integer
    testNumber(32768, true, true, false, false, true, true, true, false, true, true);
    // Positive integer too big for int16_t, given as unsigned integer
    testNumber(32768u, true, true, false, false, true, true, true, false, true, true);
    // Positive integer too big for int16_t, given as floater
    testNumber(32768.0, true, true, false, false, true, true, true, false, true, true);
    // Positive integer too big for uint16_t, given as signed integer
    testNumber(65536, true, true, false, false, true, true, false, false, true, true);
    // Positive integer too big for uint16_t, given as unsigned integer
    testNumber(65536u, true, true, false, false, true, true, false, false, true, true);
    // Positive integer too big for uint16_t, given as floater
    testNumber(65536.0, true, true, false, false, true, true, false, false, true, true);
    // Positive integer too big for int32_t, given as signed integer
    testNumber(2147483648LL, true, false, false, false, true, true, false, false, true, true);
    // Positive integer too big for int32_t, given as unsigned integer
    testNumber(2147483648u, true, false, false, false, true, true, false, false, true, true);
    // Positive integer too big for int32_t, given as floater
    testNumber(2147483648.0, true, false, false, false, true, true, false, false, true, true);
    // Positive integer too big for uint32_t, given as signed integer
    testNumber(4294967296, true, false, false, false, true, false, false, false, true, true);
    // Positive integer too big for uint32_t, given as unsigned integer
    testNumber(4294967296u, true, false, false, false, true, false, false, false, true, true);
    // Positive integer too big for uint32_t, given as floater
    testNumber(4294967296.0, true, false, false, false, true, false, false, false, true, true);
    // Positive integer too big for int64_t, given as unsigned integer
    testNumber(9223372036854775808u, false, false, false, false, true, false, false, false, true, true);
    // Positive integer too big for int64_t, given as floater
    testNumber(9223372036854775808.0, false, false, false, false, true, false, false, false, true, true);
    // Positive integer too big for uint64_t, given as floater
    testNumber(20000000000000000000.0, false, false, false, false, false, false, false, false, true, true);
    // Negative integer, given as signed integer
    testNumber(-128, true, true, true, true, false, false, false, false, true, true);
    // Negative integer, given as floater
    testNumber(-128.0, true, true, true, true, false, false, false, false, true, true);
    // Negative integer too small for int8_t, given as signed integer
    testNumber(-129, true, true, true, false, false, false, false, false, true, true);
    // Negative integer too small for int8_t, given as floater
    testNumber(-129.0, true, true, true, false, false, false, false, false, true, true);
    // Negative integer too small for int16_t, given as signed integer
    testNumber(-32769, true, true, false, false, false, false, false, false, true, true);
    // Negative integer too small for int16_t, given as floater
    testNumber(-32769.0, true, true, false, false, false, false, false, false, true, true);
    // Negative integer too small for int32_t, given as signed integer
    testNumber(-2147483649LL, true, false, false, false, false, false, false, false, true, true);
    // Negative integer too small for int32_t, given as floater
    testNumber(-2147483649.0, true, false, false, false, false, false, false, false, true, true);
    // Negative integer too small for int64_t, given as floater
    testNumber(-10000000000000000000.0, false, false, false, false, false, false, false, false, true, true);
    // Floating point number
    testNumber(123.4, false, false, false, false, false, false, false, false, true, true);
}

TEST(json, wrongValueType)
{
    // Safe
    ASSERT_FALSE((Value().asObject()));
    ASSERT_FALSE((Value().asArray()));
    ASSERT_FALSE((Value().asString()));
    ASSERT_FALSE((Value().get<std::string>()));
    ASSERT_FALSE((Value().get<std::string_view>()));
    ASSERT_FALSE((Value().get<const char *>()));
    ASSERT_FALSE((Value().get<char>()));
    ASSERT_FALSE((Value().asInteger()));
    ASSERT_FALSE((Value().asUnsigner()));
    ASSERT_FALSE((Value().asFloater()));
    ASSERT_FALSE((Value().get<int64_t>()));
    ASSERT_FALSE((Value().get<int32_t>()));
    ASSERT_FALSE((Value().get<int16_t>()));
    ASSERT_FALSE((Value().get<int8_t>()));
    ASSERT_FALSE((Value().get<uint64_t>()));
    ASSERT_FALSE((Value().get<uint32_t>()));
    ASSERT_FALSE((Value().get<uint16_t>()));
    ASSERT_FALSE((Value().get<uint8_t>()));
    ASSERT_FALSE((Value().get<double>()));
    ASSERT_FALSE((Value().get<float>()));
    ASSERT_FALSE((Value().asBoolean()));
    ASSERT_FALSE((Value().get<bool>()));
    ASSERT_FALSE((Value(1).get<nullptr_t>()));
}

TEST(json, density)
{
    ASSERT_EQ(R"([
    1,
    2,
    3
])"s, encode(makeArray(1, 2, 3), Density::multiline));
    ASSERT_EQ("[ 1, 2, 3 ]"s, encode(makeArray(1, 2, 3), Density::uniline));
    ASSERT_EQ("[1,2,3]"s, encode(makeArray(1, 2, 3), Density::nospace));
}

TEST(json, makeObject)
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

TEST(json, makeArray)
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

TEST(json, comments)
{
    { // Encode object
        Value json{makeObject("a", 1, "b", 2, "c", 3)};
        json.setComment("Yada yada");
        Object & obj{*json.asObject()};
        obj.at("a").setComment("How fascinating...");
        obj.at("c").setComment("Wow,\nso\n  incredible");
        ASSERT_EQ(R"(// Yada yada
{
    // How fascinating...
    "a": 1,
    "b": 2,
    // Wow,
    // so
    //   incredible
    "c": 3
})"s, qc::json::encode(json, Density::multiline));
        ASSERT_EQ(R"(/* Yada yada */ { /* How fascinating... */ "a": 1, "b": 2, /* Wow,
so
  incredible */ "c": 3 })"s, qc::json::encode(json, Density::uniline));
        ASSERT_EQ(R"(/*Yada yada*/{/*How fascinating...*/"a":1,"b":2,/*Wow,
so
  incredible*/"c":3})"s, qc::json::encode(json, Density::nospace));
    }
    { // Encode array
        Value json{makeArray(1, 2, 3)};
        json.setComment("Yada yada");
        Array & arr{*json.asArray()};
        arr[0].setComment("How fascinating...");
        arr[2].setComment("Wow,\nso\n  incredible");
        ASSERT_EQ(R"(// Yada yada
[
    // How fascinating...
    1,
    2,
    // Wow,
    // so
    //   incredible
    3
])"s, qc::json::encode(json, Density::multiline));
        ASSERT_EQ(R"(/* Yada yada */ [ /* How fascinating... */ 1, 2, /* Wow,
so
  incredible */ 3 ])"s, qc::json::encode(json, Density::uniline));
        ASSERT_EQ(R"(/*Yada yada*/[/*How fascinating...*/1,2,/*Wow,
so
  incredible*/3])"s, qc::json::encode(json, Density::nospace));
    }
    { // Decode
        const Value json{*decode(R"(// AAAAA
// BBBBB
/* CCCCC */
[
    /* DDDDD */
    // EEEEE
    [ /* FFFFF */ /* GGGGG */ 0, /* HHHHH */ 1, /* IIIII */ ], // JJJJJ
    { /* KKKKK */ /* LLLLL */ "k1": /* MMMMM */ "v1", /* NNNNN */ /* OOOOO */ "k2": "v2" /* PPPPP */ } // QQQQQ
    /* RRRRR */
] // SSSSS)"sv)};
        ASSERT_EQ("CCCCC", *json.comment());
        const Array & rootArr{*json.asArray()};
        ASSERT_EQ(2u, rootArr.size());
        ASSERT_EQ("EEEEE", *rootArr.at(0).comment());
        const Array & innerArr{*rootArr.at(0).asArray()};
        ASSERT_EQ(2u, innerArr.size());
        ASSERT_EQ("GGGGG", *innerArr.at(0).comment());
        ASSERT_EQ("HHHHH", *innerArr.at(1).comment());
        ASSERT_EQ("JJJJJ", *rootArr.at(1).comment());
        const Object & innerObj{*rootArr.at(1).asObject()};
        ASSERT_EQ(2u, innerObj.size());
        ASSERT_EQ("MMMMM", *innerObj.at("k1").comment());
        ASSERT_EQ("OOOOO", *innerObj.at("k2").comment());
    }
}

TEST(json, encoderOptions)
{
    ASSERT_EQ(R"({
  k: [
    'v'
  ]
})", encode(makeObject("k", makeArray("v")), Density::multiline, 2u, true, true));
}

TEST(json, numberEquality)
{
    { // Signed integer
        Value val{10};
        ASSERT_TRUE(val == 10);
        ASSERT_TRUE(val == 10u);
        ASSERT_TRUE(val == 10.0);
        ASSERT_FALSE(val != 10);
        ASSERT_FALSE(val != 10u);
        ASSERT_FALSE(val != 10.0);
        ASSERT_TRUE(10 == val);
        ASSERT_TRUE(10u == val);
        ASSERT_TRUE(10.0 == val);
        ASSERT_FALSE(10 != val);
        ASSERT_FALSE(10u != val);
        ASSERT_FALSE(10.0 != val);
        ASSERT_FALSE(val == 11);
        ASSERT_FALSE(val == 11u);
        ASSERT_FALSE(val == 11.0);
        ASSERT_TRUE(val != 11);
        ASSERT_TRUE(val != 11u);
        ASSERT_TRUE(val != 11.0);
        ASSERT_FALSE(11 == val);
        ASSERT_FALSE(11u == val);
        ASSERT_FALSE(11.0 == val);
        ASSERT_TRUE(11 != val);
        ASSERT_TRUE(11u != val);
        ASSERT_TRUE(11.0 != val);
    }
    { // Unsigned integer
        Value val{10u};
        ASSERT_TRUE(val == 10);
        ASSERT_TRUE(val == 10u);
        ASSERT_TRUE(val == 10.0);
        ASSERT_FALSE(val != 10);
        ASSERT_FALSE(val != 10u);
        ASSERT_FALSE(val != 10.0);
        ASSERT_TRUE(10 == val);
        ASSERT_TRUE(10u == val);
        ASSERT_TRUE(10.0 == val);
        ASSERT_FALSE(10 != val);
        ASSERT_FALSE(10u != val);
        ASSERT_FALSE(10.0 != val);
        ASSERT_FALSE(val == 11);
        ASSERT_FALSE(val == 11u);
        ASSERT_FALSE(val == 11.0);
        ASSERT_TRUE(val != 11);
        ASSERT_TRUE(val != 11u);
        ASSERT_TRUE(val != 11.0);
        ASSERT_FALSE(11 == val);
        ASSERT_FALSE(11u == val);
        ASSERT_FALSE(11.0 == val);
        ASSERT_TRUE(11 != val);
        ASSERT_TRUE(11u != val);
        ASSERT_TRUE(11.0 != val);
    }
    { // Floater
        Value val{10.0};
        ASSERT_TRUE(val == 10);
        ASSERT_TRUE(val == 10u);
        ASSERT_TRUE(val == 10.0);
        ASSERT_FALSE(val != 10);
        ASSERT_FALSE(val != 10u);
        ASSERT_FALSE(val != 10.0);
        ASSERT_TRUE(10 == val);
        ASSERT_TRUE(10u == val);
        ASSERT_TRUE(10.0 == val);
        ASSERT_FALSE(10 != val);
        ASSERT_FALSE(10u != val);
        ASSERT_FALSE(10.0 != val);
        ASSERT_FALSE(val == 11);
        ASSERT_FALSE(val == 11u);
        ASSERT_FALSE(val == 11.0);
        ASSERT_TRUE(val != 11);
        ASSERT_TRUE(val != 11u);
        ASSERT_TRUE(val != 11.0);
        ASSERT_FALSE(11 == val);
        ASSERT_FALSE(11u == val);
        ASSERT_FALSE(11.0 == val);
        ASSERT_TRUE(11 != val);
        ASSERT_TRUE(11u != val);
        ASSERT_TRUE(11.0 != val);
    }
    { // Special cases
        Value val{std::numeric_limits<uint64_t>::max()};
        ASSERT_FALSE(val == -1);

        val = -1;
        ASSERT_FALSE(val == std::numeric_limits<uint64_t>::max());

        val = std::numeric_limits<int64_t>::max();
        ASSERT_FALSE(val == double(std::numeric_limits<int64_t>::max()));

        // TODO: Figure out why this doesn't work on x86
        if constexpr (sizeof(size_t) >= 8)
        {
            val = std::numeric_limits<uint64_t>::max();
            ASSERT_FALSE(val == double(std::numeric_limits<uint64_t>::max()));
        }

        val = std::numeric_limits<double>::infinity();
        ASSERT_TRUE(val == std::numeric_limits<double>::infinity());
        ASSERT_FALSE(val == std::numeric_limits<int64_t>::max());
        ASSERT_FALSE(val == std::numeric_limits<uint64_t>::max());

        val = std::numeric_limits<double>::quiet_NaN();
        ASSERT_FALSE(val == std::numeric_limits<double>::quiet_NaN());
        ASSERT_TRUE(val != std::numeric_limits<double>::quiet_NaN());
    }
}

TEST(json, general)
{
    std::string json(R"(// Third quarter summary document
// Protected information, do not propagate!
{
    "Dishes": [
        {
            "Gluten Free": false,
            "Ingredients": [ "\"Salt\"", "Barnacles" ],
            "Name": "Basket o' Barnacles",
            "Price": 5.45
        },
        {
            "Gluten Free": true,
            "Ingredients": [ /* It's actually cod lmao */ "Tuna" ],
            "Name": "Two Tuna",
            "Price": -inf
        },
        {
            "Gluten Free": false,
            "Ingredients": [ "\"Salt\"", "Octopus", "Crab" ],
            "Name": "18 Leg Bouquet",
            "Price": nan
        }
    ],
    // Not necessarily up to date
    "Employees": [
        { "Age": 69, "Name": "Ol' Joe Fisher", "Title": "Fisherman" },
        { "Age": 41, "Name": "Mark Rower", "Title": "Cook" },
        { "Age": 19, "Name": "Phineas", "Title": "Server Boy" }
    ],
    "Founded": 1964,
    "Green Eggs and Ham": "I do not like them in a box\nI do not like them with a fox\nI do not like them in a house\nI do not like them with a mouse\nI do not like them here or there\nI do not like them anywhere\nI do not like green eggs and ham\nI do not like them Sam I am\n",
    "Ha\x03r Name": "M\0\0n",
    // What could they mean?!
    "Magic Numbers": [777,777,777],
    "Name": "Salt's Crust",
    // Pay no heed
    "Profit Margin": null
})"s);
    ASSERT_EQ(json, encode(*decode(json)));
}
