#include <cmath>

#include <concepts>
#include <deque>
#include <format>
#include <variant>

#include <gtest/gtest.h>

#include <qcon-decode.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;

using qcon::decode;
using qcon::DecodeResult;

static qcon::DummyComposer dummyComposer{};

class ExpectantComposer
{
  public:

    struct Object {};
    struct Array {};
    struct End {};
    struct Key { std::string_view k; };
    struct String { std::string_view v; };
    struct Integer { int64_t v; bool positive; };
    struct Floater { double v; };
    struct Boolean { bool v; };
    struct Null {};

    friend bool operator==(const Object &, const Object &) { return true; }
    friend bool operator==(const Array &, const Array &) { return true; }
    friend bool operator==(const End &, const End &) { return true; }
    friend bool operator==(const Key & a, const Key & b) { return a.k == b.k; }
    friend bool operator==(const String & a, const String & b) { return a.v == b.v; }
    friend bool operator==(const Integer & a, const Integer & b) { return a.v == b.v; }
    friend bool operator==(const Floater & a, const Floater & b) { return a.v == b.v || (std::isnan(a.v) && std::isnan(b.v)); }
    friend bool operator==(const Boolean & a, const Boolean & b) { return a.v == b.v; }
    friend bool operator==(const Null &, const Null &) { return true; }

    using Element = std::variant<Object, Array, End, Key, String, Integer, Floater, Boolean, Null>;

    std::nullptr_t object(std::nullptr_t) { assertNextIs(Object{}); return nullptr; }
    std::nullptr_t array(std::nullptr_t) { assertNextIs(Array{}); return nullptr; }
    void end(std::nullptr_t, std::nullptr_t) { assertNextIs(End{}); }
    void key(std::string_view k, std::nullptr_t) { assertNextIs(Key{k}); }
    void val(std::string_view v, std::nullptr_t) { assertNextIs(String{v}); }
    void val(int64_t v, bool positive, std::nullptr_t) { assertNextIs(Integer{v, positive}); }
    void val(double v, std::nullptr_t) { assertNextIs(Floater{v}); }
    void val(bool v, std::nullptr_t) { assertNextIs(Boolean{v}); }
    void val(std::nullptr_t, std::nullptr_t) { assertNextIs(Null{}); }

    ExpectantComposer & expectObject() { m_sequence.emplace_back(Object{}); return *this; }
    ExpectantComposer & expectArray() { m_sequence.emplace_back(Array{}); return *this; }
    ExpectantComposer & expectEnd() { m_sequence.emplace_back(End{}); return *this; }
    ExpectantComposer & expectKey(std::string_view k) { m_sequence.emplace_back(Key{k}); return *this; }
    ExpectantComposer & expectString(std::string_view v) { m_sequence.emplace_back(String{v}); return *this; }
    ExpectantComposer & expectInteger(std::signed_integral auto v) { m_sequence.emplace_back(Integer{v, v >= 0}); return *this; }
    ExpectantComposer & expectInteger(std::unsigned_integral auto v) { m_sequence.emplace_back(Integer{int64_t(v), true}); return *this; }
    ExpectantComposer & expectFloater(double v) { m_sequence.emplace_back(Floater{v}); return *this; }
    ExpectantComposer & expectBoolean(bool v) { m_sequence.emplace_back(Boolean{v}); return *this; }
    ExpectantComposer & expectNull() { m_sequence.emplace_back(Null{}); return *this; }

    bool isDone() const { return m_sequence.empty(); }

  private:

    std::deque<Element> m_sequence;

    void assertNextIs(const Element & e)
    {
        ASSERT_FALSE(m_sequence.empty());
        ASSERT_EQ(m_sequence.front(), e);
        m_sequence.pop_front();
    }
};

std::ostream & operator<<(std::ostream & os, const ExpectantComposer::Element & v)
{
    if (std::holds_alternative<ExpectantComposer::Object>(v)) return os << "Object";
    if (std::holds_alternative<ExpectantComposer::Array>(v)) return os << "Array";
    if (std::holds_alternative<ExpectantComposer::End>(v)) return os << "End";
    if (std::holds_alternative<ExpectantComposer::Key>(v)) return os << "Key `" << std::get<ExpectantComposer::Key>(v).k << "`";
    if (std::holds_alternative<ExpectantComposer::String>(v)) return os << "String `" << std::get<ExpectantComposer::String>(v).v << "`";
    if (std::holds_alternative<ExpectantComposer::Integer>(v)) { const ExpectantComposer::Integer & i{std::get<ExpectantComposer::Integer>(v)}; return i.positive ? (os << "Integer `" << uint64_t(i.v)) : (os << "Integer `" << i.v) << "`"; }
    if (std::holds_alternative<ExpectantComposer::Floater>(v)) return os << "Floater `" << std::get<ExpectantComposer::Floater>(v).v << "`";
    if (std::holds_alternative<ExpectantComposer::Boolean>(v)) return os << "Boolean `" << (std::get<ExpectantComposer::Boolean>(v).v ? "true" : "false") << "`";
    if (std::holds_alternative<ExpectantComposer::Null>(v)) return os << "Null";
    return os << "Unknown Element";
}

TEST(decode, object)
{
    { // Empty
        ExpectantComposer composer{};
        composer.expectObject().expectEnd();
        ASSERT_TRUE(decode(R"({})"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Single key
        ExpectantComposer composer{};
        composer.expectObject().expectKey("a"sv).expectNull().expectEnd();
        ASSERT_TRUE(decode(R"({ "a": null })"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Multiple keys
        ExpectantComposer composer{};
        composer.expectObject().expectKey("a"sv).expectNull().expectKey("b"sv).expectNull().expectKey("c"sv).expectNull().expectEnd();
        ASSERT_TRUE(decode(R"({ "a": null, "b": null, "c": null })"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // No space
        ExpectantComposer composer{};
        composer.expectObject().expectKey("a"sv).expectNull().expectKey("b"sv).expectNull().expectEnd();
        ASSERT_TRUE(decode(R"({"a":null,"b":null})"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Weird spacing
        ExpectantComposer composer{};
        composer.expectObject().expectKey("a"sv).expectNull().expectKey("b"sv).expectNull().expectEnd();
        ASSERT_TRUE(decode(R"({"a" :null ,"b" :null})"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Empty key
        ExpectantComposer composer{};
        composer.expectObject().expectKey(""sv).expectNull().expectEnd();
        ASSERT_TRUE(decode(R"({ "": null })"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // No colon after key
        ASSERT_FALSE(decode(R"({ "a" 0 })", dummyComposer, nullptr).success);
    }
    { // Key with single quotes
        ASSERT_FALSE(decode(R"({ 'a': 0 })", dummyComposer, nullptr).success);
    }
    { // Key without quotes
        ASSERT_FALSE(decode(R"({ a: 0 })", dummyComposer, nullptr).success);
    }
    { // Missing value
        ASSERT_FALSE(decode(R"({ "a": })", dummyComposer, nullptr).success);
    }
    { // No comma between elements
        ASSERT_FALSE(decode(R"({ "a": 0 "b": 1 })", dummyComposer, nullptr).success);
    }
    { // Empty entry
        ASSERT_FALSE(decode(R"({ "a": 0, , "b": 1 })", dummyComposer, nullptr).success);
    }
    { // Cut off
        ASSERT_FALSE(decode(R"({)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({")", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a")", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":0,)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":0,")", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":0,"b)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":0,"b")", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":0,"b":)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"a":0,"b":1)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"("a":0,"b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(a":0,"b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(":0,"b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(:0,"b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0,"b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(,"b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"("b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(b":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(":1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(:1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1})", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(})", dummyComposer, nullptr).success);
    }
}

TEST(decode, array)
{
    { // Empty
        ExpectantComposer composer{};
        composer.expectArray().expectEnd();
        ASSERT_TRUE(decode(R"([])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Single element
        ExpectantComposer composer{};
        composer.expectArray().expectNull().expectEnd();
        ASSERT_TRUE(decode(R"([ null ])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Multiple elements
        ExpectantComposer composer{};
        composer.expectArray().expectNull().expectNull().expectNull().expectEnd();
        ASSERT_TRUE(decode(R"([ null, null, null ])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // No space
        ExpectantComposer composer{};
        composer.expectArray().expectNull().expectNull().expectEnd();
        ASSERT_TRUE(decode(R"([null,null])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Weird spacing
        ExpectantComposer composer{};
        composer.expectArray().expectNull().expectNull().expectEnd();
        ASSERT_TRUE(decode(R"([null ,null])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // No comma between elements
        ASSERT_FALSE(decode(R"([ 0 1 ])", dummyComposer, nullptr).success);
    }
    { // Empty entry
        ASSERT_FALSE(decode(R"([ 0, , 1 ])", dummyComposer, nullptr).success);
    }
    { // Cut off
        ASSERT_FALSE(decode(R"([)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([0,)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([0,1)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0,1])", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(,1])", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1])", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(])", dummyComposer, nullptr).success);
    }
}

TEST(decode, string)
{
    { // Empty string
        ExpectantComposer composer{};
        composer.expectString(""sv);
        ASSERT_TRUE(decode(R"("")"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // All printable
        ExpectantComposer composer{};
        composer.expectString(R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv);
        const bool success{decode(R"(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")"sv, composer, nullptr).success};
        ASSERT_TRUE(success);
        ASSERT_TRUE(composer.isDone());
    }
    { // All non-printable
        std::string decodeStr{R"(" ")"};
        for (int i{0}; i < 256; ++i)
        {
            if (!std::isprint(i))
            {
                decodeStr[1] = char(i);
                ASSERT_FALSE(decode(decodeStr, dummyComposer, nullptr).success);
            }
        }
    }
    { // Escape characters
        ExpectantComposer composer{};
        composer.expectString("\0\b\t\n\v\f\r"sv);
        ASSERT_TRUE(decode(R"("\0\b\t\n\v\f\r")"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Missing escape sequence
        const std::string_view brokenSeq{R"("\\\")"};
        ASSERT_FALSE(decode(brokenSeq, dummyComposer, nullptr).success);
        const std::string_view brokenSeqInArray{R"([ "\\\" ])"};
        ASSERT_FALSE(decode(brokenSeqInArray, dummyComposer, nullptr).success);
    }
    { // Unknown escape sequence
        ASSERT_FALSE(decode("\"\\\0\"", dummyComposer, nullptr).success);
    }
    { // 'x' code point
        ExpectantComposer composer{};
        std::string expectedStr(256, '\0');
        std::string decodeStr(1 + 256 * 4 + 1, '\0');
        decodeStr.front() = '"';
        decodeStr.back() = '"';
        for (size_t i{0u}; i < 256u; ++i)
        {
            expectedStr[i] = char(i);
            std::format_to_n(&decodeStr[1u + 4u * i], 4, "\\x{:02X}"sv, i);
        }
        composer.expectString(expectedStr);
        ASSERT_TRUE(decode(decodeStr, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // 'u' code point
        ExpectantComposer composer{};
        std::string expectedStr(256, '\0');
        std::string decodeStr(1 + 256 * 6 + 1, '\0');
        decodeStr.front() = '"';
        decodeStr.back() = '"';
        for (size_t i{0u}; i < 256u; ++i)
        {
            expectedStr[i] = char(i);
            std::format_to_n(&decodeStr[1u + 6u * i], 6, "\\u{:04X}"sv, i);
        }
        composer.expectString(expectedStr);
        ASSERT_TRUE(decode(decodeStr, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // 'U' code point
        ExpectantComposer composer{};
        composer.expectString("\u0077\u00FF\u00FF");
        const bool success{decode(R"("\U00000077\U000000FF\UFFFFFFFF")", composer, nullptr).success};
        ASSERT_TRUE(success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Uppercase and lowercase code point hex digits
        ExpectantComposer composer{};
        composer.expectString("\u00AA\u00BB\u00CC\u00DD");
        ASSERT_TRUE(decode(R"("\u00aa\u00BB\u00cC\u00Dd")", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Incorrect number of code point digits
        // Raw strings, `\x`/`\u`, and macros don't play nice together
        ASSERT_FALSE(decode("\"\\x\"", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode("\"\\x1\"", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode("\"\\u\"", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode("\"\\u1\"", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode("\"\\u11\"", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode("\"\\u111\"", dummyComposer, nullptr).success);
    }
    { // Missing end quote
        ASSERT_FALSE(decode(R"("abc)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([ "abc ])", dummyComposer, nullptr).success);
    }
    { // Escaped newlines
        ExpectantComposer composer{};
        composer.expectString("abc");
        ASSERT_TRUE(decode("\"a\\\nb\\\nc\"", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectString("abc");
        ASSERT_TRUE(decode("\"a\\\r\nb\\\r\nc\"", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectString("");
        ASSERT_TRUE(decode("\"\\\n\\\n\\\n\"", composer, nullptr).success);
        composer.expectString("");
        ASSERT_TRUE(decode("\"\\\r\n\\\n\\\r\n\"", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Single quotes
        ASSERT_FALSE(decode(R"('abc')", dummyComposer, nullptr).success);
    }
}

TEST(decode, decimal)
{
    { // Zero
        ExpectantComposer composer{};
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Normal
        ExpectantComposer composer{};
        composer.expectInteger(123);
        ASSERT_TRUE(decode(R"(123)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Positive
        ExpectantComposer composer{};
        composer.expectInteger(123);
        ASSERT_TRUE(decode(R"(+123)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Negative
        ExpectantComposer composer{};
        composer.expectInteger(-123);
        ASSERT_TRUE(decode(R"(-123)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-9223372036854775808)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::max());
        ASSERT_TRUE(decode(R"(+9223372036854775807)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max unsigned
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+18446744073709551615)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(123);
        ASSERT_TRUE(decode(R"(0123)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(00)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(+00)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(-00)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-000000009223372036854775808)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::max());
        ASSERT_TRUE(decode(R"(+000000009223372036854775807)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max unsigned with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+0000000018446744073709551615)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Invalid minus sign
        ASSERT_FALSE(decode(R"(-)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([ - ])", dummyComposer, nullptr).success);
    }
    { // Invalid plus sign
        ASSERT_FALSE(decode(R"(+)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([ + ])", dummyComposer, nullptr).success);
    }
    { // Multiple signs
        ASSERT_FALSE(decode(R"(++0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(--0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(+-0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(-+0)", dummyComposer, nullptr).success);
    }
}

TEST(decode, hex)
{
    { // Zero
        ExpectantComposer composer{};
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0x0)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Lowercase
        ExpectantComposer composer{};
        composer.expectInteger(26);
        ASSERT_TRUE(decode(R"(0x1a)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Uppercase
        ExpectantComposer composer{};
        composer.expectInteger(26);
        ASSERT_TRUE(decode(R"(0x1A)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Positive
        ExpectantComposer composer{};
        composer.expectInteger(26);
        ASSERT_TRUE(decode(R"(+0x1A)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Negative
        ExpectantComposer composer{};
        composer.expectInteger(-26);
        ASSERT_TRUE(decode(R"(-0x1A)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-0x8000000000000000)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+0xFFFFFFFFFFFFFFFF)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(26);
        ASSERT_TRUE(decode(R"(0x001A)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-0x000000008000000000000000)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+0x00000000FFFFFFFFFFFFFFFF)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Invalid digit
        ASSERT_FALSE(decode(R"(0x1G)", dummyComposer, nullptr).success);
    }
    { // Uppercase X
        ASSERT_FALSE(decode(R"(0X1A)", dummyComposer, nullptr).success);
    }
    { // Prefix leading zero
        ASSERT_FALSE(decode(R"(00x1A)", dummyComposer, nullptr).success);
    }
    { // Decimal
        ASSERT_FALSE(decode(R"(0x1A.)", dummyComposer, nullptr).success);
    }
    { // Too big
        ASSERT_FALSE(decode(R"(0x10000000000000000)", dummyComposer, nullptr).success);
    }
}

TEST(decode, octal)
{
    { // Zero
        ExpectantComposer composer{};
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0o0)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // No sign
        ExpectantComposer composer{};
        composer.expectInteger(10);
        ASSERT_TRUE(decode(R"(0o12)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Positive
        ExpectantComposer composer{};
        composer.expectInteger(10);
        ASSERT_TRUE(decode(R"(+0o12)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Negative
        ExpectantComposer composer{};
        composer.expectInteger(-10);
        ASSERT_TRUE(decode(R"(-0o12)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-0o1000000000000000000000)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+0o1777777777777777777777)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(10);
        ASSERT_TRUE(decode(R"(0o0012)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-0o000000001000000000000000000000)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+0o000000001777777777777777777777)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Invalid digit
        ASSERT_FALSE(decode(R"(0o18)", dummyComposer, nullptr).success);
    }
    { // Uppercase O
        ASSERT_FALSE(decode(R"(0O12)", dummyComposer, nullptr).success);
    }
    { // Prefix leading zero
        ASSERT_FALSE(decode(R"(00x12)", dummyComposer, nullptr).success);
    }
    { // Decimal
        ASSERT_FALSE(decode(R"(0x12.)", dummyComposer, nullptr).success);
    }
    { // Too big
        ASSERT_FALSE(decode(R"(0o2000000000000000000000)", dummyComposer, nullptr).success);
    }
}

TEST(decode, binary)
{
    { // Zero
        ExpectantComposer composer{};
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0b0)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // No sign
        ExpectantComposer composer{};
        composer.expectInteger(5);
        ASSERT_TRUE(decode(R"(0b101)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Positive
        ExpectantComposer composer{};
        composer.expectInteger(5);
        ASSERT_TRUE(decode(R"(+0b101)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Negative
        ExpectantComposer composer{};
        composer.expectInteger(-5);
        ASSERT_TRUE(decode(R"(-0b101)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-0b1000000000000000000000000000000000000000000000000000000000000000)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+0b1111111111111111111111111111111111111111111111111111111111111111)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(5);
        ASSERT_TRUE(decode(R"(0b00101)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Min with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<int64_t>::min());
        ASSERT_TRUE(decode(R"(-0b000000001000000000000000000000000000000000000000000000000000000000000000)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max with leading zeroes
        ExpectantComposer composer{};
        composer.expectInteger(std::numeric_limits<uint64_t>::max());
        ASSERT_TRUE(decode(R"(+0b000000001111111111111111111111111111111111111111111111111111111111111111)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Invalid digit
        ASSERT_FALSE(decode(R"(0b121)", dummyComposer, nullptr).success);
    }
    { // Uppercase B
        ASSERT_FALSE(decode(R"(0B101)", dummyComposer, nullptr).success);
    }
    { // Prefix leading zero
        ASSERT_FALSE(decode(R"(00b101)", dummyComposer, nullptr).success);
    }
    { // Decimal
        ASSERT_FALSE(decode(R"(0b101.)", dummyComposer, nullptr).success);
    }
    { // Too big
        ASSERT_FALSE(decode(R"(0b10000000000000000000000000000000000000000000000000000000000000000)", dummyComposer, nullptr).success);
    }
}

TEST(decode, floater)
{
    { // Zero
        ExpectantComposer composer{};
        composer.expectFloater(0.0);
        ASSERT_TRUE(decode(R"(0.0)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // No sign
        ExpectantComposer composer{};
        composer.expectFloater(123.456);
        ASSERT_TRUE(decode(R"(123.456)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Positive
        ExpectantComposer composer{};
        composer.expectFloater(123.456);
        ASSERT_TRUE(decode(R"(+123.456)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Negative
        ExpectantComposer composer{};
        composer.expectFloater(-123.456);
        ASSERT_TRUE(decode(R"(-123.456)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Exponent lowercase
        ExpectantComposer composer{};
        composer.expectFloater(123.456e17);
        ASSERT_TRUE(decode(R"(123.456e17)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Exponent uppercase
        ExpectantComposer composer{};
        composer.expectFloater(123.456e17);
        ASSERT_TRUE(decode(R"(123.456E17)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Positive exponent
        ExpectantComposer composer{};
        composer.expectFloater(123.456e17);
        ASSERT_TRUE(decode(R"(123.456e+17)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Negative exponent
        ExpectantComposer composer{};
        composer.expectFloater(-123.456e-17);
        ASSERT_TRUE(decode(R"(-123.456e-17)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Exponent without fraction
        ExpectantComposer composer{};
        composer.expectFloater(123.0e34);
        ASSERT_TRUE(decode(R"(123e34)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Max integer
        ExpectantComposer composer{};
        composer.expectFloater(9007199254740991.0);
        ASSERT_TRUE(decode(R"(9007199254740991.0e0)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Leading and trailing decimal
        ASSERT_FALSE(decode(R"(.0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(+.0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(-.0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0.)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(+0.)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(-0.)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1.e0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(.)", dummyComposer, nullptr).success);
    }
    { // Leading zeroes
        ExpectantComposer composer{};
        composer.expectFloater(1.2);
        ASSERT_TRUE(decode(R"(01.2)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(0.2);
        ASSERT_TRUE(decode(R"(00.2)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(1e02);
        ASSERT_TRUE(decode(R"(1e02)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(1e+02);
        ASSERT_TRUE(decode(R"(1e+02)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(1e-02);
        ASSERT_TRUE(decode(R"(1e-02)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(1e00);
        ASSERT_TRUE(decode(R"(1e00)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Valid infinity
        ExpectantComposer composer{};
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(inf)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(-std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(-inf)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(+inf)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(Inf)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(-std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(-Inf)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(+Inf)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(infinity)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(-std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(-infinity)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(+infinity)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(Infinity)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(-std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(-Infinity)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(decode(R"(+Infinity)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Invalid infinity
        ASSERT_FALSE(decode(R"(iNf)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(inF)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(INF)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infi)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infin)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infini)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infinit)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(iNfinity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(inFinity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infInity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infiNity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infinIty)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infiniTy)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infinitY)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(Infi)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(Infin)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(Infini)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(Infinit)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(INfinity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(InFinity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(InfInity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(InfiNity)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(InfinIty)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(InfiniTy)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(InfinitY)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(INFINITY)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infstuff)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(infinitystuff)", dummyComposer, nullptr).success);
    }
    { // Valid NaN
        ExpectantComposer composer{};
        composer.expectFloater(std::numeric_limits<double>::quiet_NaN());
        ASSERT_TRUE(decode(R"(nan)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectFloater(std::numeric_limits<double>::quiet_NaN());
        ASSERT_TRUE(decode(R"(NaN)", composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Invalid NaN
        ASSERT_FALSE(decode(R"(Nan)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(nAn)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(naN)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(NAN)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(nanstuff)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(NaNstuff)", dummyComposer, nullptr).success);
    }
    { // Exponent decimal point
        ASSERT_FALSE(decode(R"(1.0e1.0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1.0e1.)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1e1.0)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1e1.)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1e.1)", dummyComposer, nullptr).success);
    }
    { // Dangling exponent
        ASSERT_FALSE(decode(R"(0e)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0e+)", dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0e-)", dummyComposer, nullptr).success);
    }
    { // Magnitude too large
        ASSERT_FALSE(decode(R"(1e1000)", dummyComposer, nullptr).success);
    }
    { // Magnitude too small
        ASSERT_FALSE(decode(R"(1e-1000)", dummyComposer, nullptr).success);
    }
}

TEST(decode, boolean)
{
    { // True
        ExpectantComposer composer{};
        composer.expectBoolean(true);
        ASSERT_TRUE(decode(R"(true)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // False
        ExpectantComposer composer{};
        composer.expectBoolean(false);
        ASSERT_TRUE(decode(R"(false)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
}

TEST(decode, null)
{
    ExpectantComposer composer{};
    composer.expectNull();
    ASSERT_TRUE(decode(R"(null)"sv, composer, nullptr).success);
    ASSERT_TRUE(composer.isDone());
}

TEST(decode, noSpace)
{
    ExpectantComposer composer{};
    composer.expectObject().expectKey("a"sv).expectArray().expectString("abc"sv).expectInteger(-123).expectFloater(-123.456e-78).expectBoolean(true).expectNull().expectEnd().expectEnd();
    ASSERT_TRUE(decode(R"({"a":["abc",-123,-123.456e-78,true,null]})"sv, composer, nullptr).success);
    ASSERT_TRUE(composer.isDone());
}

TEST(decode, extraneousSpace)
{
    ExpectantComposer composer{};
    composer.expectObject().expectEnd();
    ASSERT_TRUE(decode(" \t\n\r\v{} \t\n\r\v"sv, composer, nullptr).success);
    ASSERT_TRUE(composer.isDone());
}

TEST(decode, trailingComma)
{
    { // Valid
        ExpectantComposer composer{};
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0,)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0, )"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0 ,)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectArray().expectInteger(0).expectEnd();
        ASSERT_TRUE(decode(R"([0,])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectArray().expectInteger(0).expectEnd();
        ASSERT_TRUE(decode(R"([0, ])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectArray().expectInteger(0).expectEnd();
        ASSERT_TRUE(decode(R"([0 ,])"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectObject().expectKey("k"sv).expectInteger(0).expectEnd();
        ASSERT_TRUE(decode(R"({"k":0,})"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectObject().expectKey("k"sv).expectInteger(0).expectEnd();
        ASSERT_TRUE(decode(R"({"k":0, })"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectObject().expectKey("k"sv).expectInteger(0).expectEnd();
        ASSERT_TRUE(decode(R"({"k":0 ,})"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Invalid
        ASSERT_FALSE(decode(R"(,)"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"( ,)"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(, )"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0,,)"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0 ,,)"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0, ,)"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(0,, )"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([0,,])"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([0 ,,])"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([0, ,])"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"([0,, ])"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"k":0,,})"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"k":0 ,,})"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"k":0, ,})"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"({"k":0,, })"sv, dummyComposer, nullptr).success);
    }
}

TEST(decode, comments)
{
    { // Single comment
        ExpectantComposer composer{};
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0 # AAAAA)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectInteger(0);
        ASSERT_TRUE(decode(R"(0 # AAAAA # BBBBB 1)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Multiple comments
        ExpectantComposer composer{};
        composer.expectInteger(0);
        ASSERT_TRUE(decode("# AAAAA\n#  BBBBB \n #CCCCC\n\n# DD DD\n0"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Comment in string
        ExpectantComposer composer{};
        composer.expectString("# AAAAA"sv);
        ASSERT_TRUE(decode(R"("# AAAAA")"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Comments in array
        ExpectantComposer composer{};
        composer.expectArray();
            composer.expectInteger(0);
            composer.expectInteger(1);
        composer.expectEnd();
        ASSERT_TRUE(decode(
R"([ # AAAAA
    0, # BBBBB
    1 # CCCCC
] # DDDDD)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Comments in object
        ExpectantComposer composer{};
        composer.expectObject();
            composer.expectKey("0"sv).expectInteger(0);
            composer.expectKey("1"sv).expectInteger(1);
        composer.expectEnd();
        ASSERT_TRUE(decode(
R"({ # AAAAA
    "0": 0, # BBBBB
    "1": 1 # CCCCC
} # DDDDD)"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // CRLF
        ExpectantComposer composer{};
        composer.expectArray().expectInteger(0).expectInteger(1).expectEnd();
        ASSERT_TRUE(decode("[ # AAAAA\r\n    0, # BBBBB\n    1 # CCCCC\r\n] # DDDDD \n"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
        composer.expectArray().expectInteger(1).expectEnd();
        ASSERT_TRUE(decode("[ # AAAAA\r 0, # BBBBB\n    1 # CCCCC\r\n] # DDDDD \n"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Weirdness
        ExpectantComposer composer{};

        composer.expectInteger(0);
        ASSERT_TRUE(decode("#\n0"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("# \n0"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("##\n0"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("# #\n0"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("#\n#\n0"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("0#"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("0# "sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("0#\n"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());

        composer.expectInteger(0);
        ASSERT_TRUE(decode("0#\n#"sv, composer, nullptr).success);
        ASSERT_TRUE(composer.isDone());
    }
    { // Nothing but comments
        ASSERT_FALSE(decode("# AAAAA\n# CCCCC\n"sv, dummyComposer, nullptr).success);
    }
}

TEST(decode, misc)
{
    { // Empty
        ASSERT_FALSE(decode(R"()"sv, dummyComposer, nullptr).success);
    }
    { // Only Space
        ASSERT_FALSE(decode(R"(   )"sv, dummyComposer, nullptr).success);
    }
    { // Unknown value
        ASSERT_FALSE(decode(R"(v)"sv, dummyComposer, nullptr).success);
    }
    { // Multiple root values
        ASSERT_FALSE(decode(R"(1 2)"sv, dummyComposer, nullptr).success);
        ASSERT_FALSE(decode(R"(1, 2)"sv, dummyComposer, nullptr).success);
    }
    { // Lone decimal
        ASSERT_FALSE(decode(R"(.)"sv, dummyComposer, nullptr).success);
    }
}

TEST(decode, general)
{
    ExpectantComposer composer{};
    composer.expectObject();
        composer.expectKey("Name"sv).expectString("Salt's Crust"sv);
        composer.expectKey("Founded"sv).expectInteger(1964);
        composer.expectKey("Employees"sv).expectArray();
            composer.expectObject().expectKey("Name"sv).expectString("Ol' Joe Fisher"sv).expectKey("Title"sv).expectString("Fisherman"sv).expectKey("Age"sv).expectInteger(69).expectEnd();
            composer.expectObject().expectKey("Name"sv).expectString("Mark Rower"sv).expectKey("Title"sv).expectString("Cook"sv).expectKey("Age"sv).expectInteger(41).expectEnd();
            composer.expectObject().expectKey("Name"sv).expectString("Phineas"sv).expectKey("Title"sv).expectString("Server Boy"sv).expectKey("Age"sv).expectInteger(19).expectEnd();
        composer.expectEnd();
        composer.expectKey("Dishes"sv).expectArray();
            composer.expectObject();
                composer.expectKey("Name"sv).expectString("Basket o' Barnacles"sv);
                composer.expectKey("Price"sv).expectFloater(5.45);
                composer.expectKey("Ingredients"sv).expectArray().expectString("\"Salt\""sv).expectString("Barnacles"sv).expectEnd();
                composer.expectKey("Gluten Free"sv).expectBoolean(false);
            composer.expectEnd();
            composer.expectObject();
                composer.expectKey("Name"sv).expectString("Two Tuna"sv);
                composer.expectKey("Price"sv).expectFloater(-std::numeric_limits<double>::infinity());
                composer.expectKey("Ingredients"sv).expectArray().expectString("Tuna"sv).expectEnd();
                composer.expectKey("Gluten Free"sv).expectBoolean(true);
            composer.expectEnd();
            composer.expectObject();
                composer.expectKey("Name"sv).expectString("18 Leg Bouquet"sv);
                composer.expectKey("Price"sv).expectFloater(std::numeric_limits<double>::quiet_NaN());
                composer.expectKey("Ingredients"sv).expectArray().expectString("\"Salt\""sv).expectString("Octopus"sv).expectString("Crab"sv).expectEnd();
                composer.expectKey("Gluten Free"sv).expectBoolean(false);
            composer.expectEnd();
        composer.expectEnd();
        composer.expectKey("Profit Margin"sv).expectNull();
        composer.expectKey("Ha\x03r Name"sv).expectString("M\0\0n"sv);
        composer.expectKey("Green Eggs and Ham"sv).expectString(
R"(I do not like them in a box
I do not like them with a fox
I do not like them in a house
I do not like them with a mouse
I do not like them here or there
I do not like them anywhere
I do not like green eggs and ham
I do not like them Sam I am
)");
        composer.expectKey("Magic Numbers"sv).expectArray().expectInteger(777).expectInteger(777).expectInteger(777).expectEnd();
    composer.expectEnd();
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
    const DecodeResult result{decode(qcon, composer, nullptr)};
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(composer.isDone());
}
