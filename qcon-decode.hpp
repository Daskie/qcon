#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This standalone header provides a SAX decoder
/// See the README for more info and examples!
///

#include <cctype>

#include <array>
#include <bit>
#include <charconv>
#include <format>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace qcon
{
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

    enum class DecodeState
    {
        error,
        object,
        array,
        end,
        string,
        integer,
        floater,
        boolean,
        null,
        done
    };

    class Decoder
    {
      public:

        std::string key;
        std::string string;
        s64 integer;
        double floater;
        bool boolean;

        Decoder() = default;
        Decoder(std::string_view qson);

        void load(std::string_view qcon);

        DecodeState step();

      private:

        DecodeState _state{DecodeState::done};
        const char * _start{};
        const char * _end{};
        const char * _pos{};
        u64 _stack{};
        unat _depth{};

        void _skipSpace();

        void _skipSpaceAndComments();

        bool _tryConsumeChar(char c);

        bool _tryConsumeChars(std::string_view str);

        [[nodiscard]] bool _consumeChar(char c);

        [[nodiscard]] bool _consumeCodePoint(char & c, int digits);

        [[nodiscard]] bool _consumeEscaped(char & c);

        [[nodiscard]] bool _consumeString(std::string & dst);

        [[nodiscard]] bool _consumeKey();

        [[nodiscard]] bool _consumeBinary(u64 & val, bool wasZero);

        [[nodiscard]] bool _consumeOctal(u64 & val, bool wasZero);

        [[nodiscard]] bool _consumeHex(u64 & val, bool wasZero);

        [[nodiscard]] bool _consumeDecimal(u64 & val, const char * end);

        [[nodiscard]] bool _consumeFloater(int sign);

        template <int base> [[nodiscard]] bool _consumeInteger(int sign, const char * end);

        void _ingestNumber(int sign);
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
    inline constexpr bool _isDigit(const char c)
    {
        return unat(c - '0') < 10u;
    }

    inline Decoder::Decoder(const std::string_view qson)
    {
        load(qson);
    }

    inline void Decoder::load(const std::string_view qcon)
    {
        _state = DecodeState::done;
        _start = qcon.data();
        _end = _start + qcon.size();
        _pos = _start;
    }

    inline DecodeState Decoder::step()
    {
        if (_state == DecodeState::error)
        {
            return _state;
        }

        const bool content{_pos != _start};

        _skipSpaceAndComments();

        // At end of QCON
        if (_pos >= _end)
        {
            if (_depth == 0u && content)
            {
                return _state = DecodeState::done;
            }
            else
            {
                string = "Expected content"sv;
                return _state = DecodeState::error;
            }
        }

        // In container
        if (_depth)
        {
            // In object
            if (_stack & 1u)
            {
                // Expect comma after value
                bool missingComma{false};
                if (_state != DecodeState::object)
                {
                    if (_tryConsumeChar(','))
                    {
                        _skipSpaceAndComments();
                    }
                    else
                    {
                        missingComma = true;
                    }
                }

                // Check for end
                if (*_pos == '}')
                {
                    ++_pos;
                    _stack >>= 1;
                    --_depth;
                    return _state = DecodeState::end;
                }

                if (missingComma)
                {
                    string = "Expected comma"sv;
                    return _state = DecodeState::error;
                }

                // Consume key
                if (!_consumeKey())
                {
                    return _state = DecodeState::error;
                }
                _skipSpaceAndComments();
            }
            // In array
            else
            {
                // Expect comma after value
                bool missingComma{false};
                if (_state != DecodeState::array)
                {
                    if (_tryConsumeChar(','))
                    {
                        _skipSpaceAndComments();
                    }
                    else
                    {
                        missingComma = true;
                    }
                }

                // Check for end
                if (*_pos == ']')
                {
                    ++_pos;
                    _stack >>= 1;
                    --_depth;
                    return _state = DecodeState::end;
                }

                if (missingComma)
                {
                    string = "Expected comma"sv;
                    return _state = DecodeState::error;
                }
            }
        }
        // Root
        else
        {
            // Can only be one value at root level
            if (content)
            {
                string = "Extraneous content"sv;
                return _state = DecodeState::error;
            }
        }

        const char c{*_pos};
        switch (c)
        {
            case '{':
            {
                if (_depth >= 64u)
                {
                    string = "Exceeded max depth of 64"sv;
                    return _state = DecodeState::error;
                }

                ++_pos;
                _stack <<= 1;
                _stack |= 1u;
                ++_depth;
                return _state = DecodeState::object;
            }
            case '[':
            {
                if (_depth >= 64u)
                {
                    string = "Exceeded max depth of 64"sv;
                    return _state = DecodeState::error;
                }

                ++_pos;
                _stack <<= 1;
                ++_depth;
                return _state = DecodeState::array;
            }
            case '"':
            {
                return _state = _consumeString(string) ? DecodeState::string : DecodeState::error;
            }
            case '0': [[fallthrough]];
            case '1': [[fallthrough]];
            case '2': [[fallthrough]];
            case '3': [[fallthrough]];
            case '4': [[fallthrough]];
            case '5': [[fallthrough]];
            case '6': [[fallthrough]];
            case '7': [[fallthrough]];
            case '8': [[fallthrough]];
            case '9':
            {
                _ingestNumber(0);
                return _state;
            }
            case '+':
            {
                ++_pos;
                if (_pos < _end)
                {
                    if (_isDigit(*_pos))
                    {
                        _ingestNumber(1);
                        return _state;
                    }
                    else if (*_pos == 'i' || *_pos == 'I')
                    {
                        ++_pos;
                        if (_tryConsumeChars("nf"sv))
                        {
                            _tryConsumeChars("inity"sv);
                            floater = std::numeric_limits<double>::infinity();
                            boolean = true;
                            return _state = DecodeState::floater;
                        }
                    }
                }
                break;
            }
            case '-':
            {
                ++_pos;
                if (_pos < _end)
                {
                    if (_isDigit(*_pos))
                    {
                        _ingestNumber(-1);
                        return _state;
                    }
                    else if (*_pos == 'i' || *_pos == 'I')
                    {
                        ++_pos;
                        if (_tryConsumeChars("nf"sv))
                        {
                            _tryConsumeChars("inity"sv);
                            floater = -std::numeric_limits<double>::infinity();
                            boolean = false;
                            return _state = DecodeState::floater;
                        }
                    }
                }
                break;
            }
            case 'i': [[fallthrough]];
            case 'I':
            {
                ++_pos;
                if (_tryConsumeChars("nf"sv))
                {
                    _tryConsumeChars("inity"sv);
                    floater = std::numeric_limits<double>::infinity();
                    boolean = true;
                    return _state = DecodeState::floater;
                }
                break;
            }
            case 't':
            {
                ++_pos;
                if (_tryConsumeChars("rue"sv))
                {
                    boolean = true;
                    return _state = DecodeState::boolean;
                }
                break;
            }
            case 'f':
            {
                ++_pos;
                if (_tryConsumeChars("alse"sv))
                {
                    boolean = false;
                    return _state = DecodeState::boolean;
                }
                break;
            }
            case 'n':
            {
                ++_pos;
                if (_tryConsumeChars("ull"sv)) {
                    return _state = DecodeState::null;
                }
                else if (_tryConsumeChars("an"sv))
                {
                    floater = std::numeric_limits<double>::quiet_NaN();
                    boolean = true;
                    return _state = DecodeState::floater;
                }
                break;
            }
            case 'N':
            {
                ++_pos;
                if (_tryConsumeChars("aN"sv))
                {
                    floater = std::numeric_limits<double>::quiet_NaN();
                    boolean = true;
                    return _state = DecodeState::floater;
                }
                break;
            }
        }

        string = "Unknown value"sv;
        return _state = DecodeState::error;
    }

    inline void Decoder::_skipSpace()
    {
        for (; _pos < _end && std::isspace(u8(*_pos)); ++_pos);
    }

    inline void Decoder::_skipSpaceAndComments()
    {
        _skipSpace();

        // Skip comments and space
        while (_pos < _end && *_pos == '#')
        {
            // Skip `#`
            ++_pos;

            // Skip rest of line
            for (; _pos < _end && *_pos != '\n'; ++_pos);

            _skipSpace();
        }
    }

    inline bool Decoder::_tryConsumeChar(const char c)
    {
        if (_pos < _end && *_pos == c)
        {
            ++_pos;
            return true;
        }
        else
        {
            return false;
        }
    }

    inline bool Decoder::_tryConsumeChars(const std::string_view str)
    {
        if (unat(_end - _pos) >= str.length())
        {
            for (unat i{0u}; i < str.length(); ++i)
            {
                if (_pos[i] != str[i])
                {
                    return false;
                }
            }
            _pos += str.length();
            return true;
        }
        else
        {
            return false;
        }
    }

    inline bool Decoder::_consumeChar(const char c)
    {
        if (!_tryConsumeChar(c))
        {
            string.clear();
            std::format_to(std::back_inserter(string), "Expected `{}`"sv, c);
            _state = DecodeState::error;
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeCodePoint(char & c, const int digits)
    {
        if (_end - _pos < digits)
        {
            string.clear();
            std::format_to(std::back_inserter(string), "Expected {} code point digits"sv, digits);
            return false;
        }

        u32 val;
        const std::from_chars_result res{std::from_chars(_pos, _pos + digits, val, 16)};
        if (res.ec != std::errc{})
        {
            string = "Invalid code point"sv;
            return false;
        }

        _pos += digits;

        c = char(val);
        return true;
    }

    inline bool Decoder::_consumeEscaped(char & c)
    {
        if (_pos >= _end)
        {
            string = "Expected escape sequence"sv;
            return false;
        }

        c = *_pos;
        ++_pos;

        switch (c)
        {
            case '0': { c = '\0'; return true; }
            case 'b': { c = '\b'; return true; }
            case 't': { c = '\t'; return true; }
            case 'n': { c = '\n'; return true; }
            case 'v': { c = '\v'; return true; }
            case 'f': { c = '\f'; return true; }
            case 'r': { c = '\r'; return true; }
            case 'x': { return _consumeCodePoint(c, 2); }
            case 'u': { return _consumeCodePoint(c, 4); }
            case 'U': { return _consumeCodePoint(c, 8); }
            default:
            {
                if (std::isprint(u8(c)))
                {
                    return true;
                }
                else
                {
                    string = "Invalid escape sequence"sv;
                    return false;
                }
            }
        }
    }

    inline bool Decoder::_consumeString(std::string & dst)
    {
        dst.clear();

        ++_pos; // We already know we have `"`

        while (true)
        {
            if (_pos >= _end)
            {
                string = "Expected end quote"sv;
                return false;
            }

            char c{*_pos};
            if (c == '"')
            {
                ++_pos;
                return true;
            }
            else if (c == '\\')
            {
                ++_pos;

                // Check for escaped newline
                if (*_pos == '\n')
                {
                    ++_pos;
                }
                else if (*_pos == '\r' && _pos + 1 < _end && _pos[1] == '\n')
                {
                    _pos += 2;
                }
                else
                {
                    if (!_consumeEscaped(c))
                    {
                        return false;
                    }
                    dst.push_back(c);
                }
            }
            else if (std::isprint(u8(c)))
            {
                dst.push_back(c);
                ++_pos;
            }
            else
            {
                string = "Invalid string content"sv;
                return false;
            }
        }
    }

    inline bool Decoder::_consumeKey()
    {
        if (_pos >= _end || *_pos != '"')
        {
            string = "Expected key"sv;
            return false;
        }

        if (!_consumeString(key))
        {
            return false;
        }

        _skipSpaceAndComments();

        if (!_consumeChar(':'))
        {
            return false;
        }

        return true;
    }

    inline consteval std::array<u8, 256u> _createHexTable()
    {
        std::array<u8, 256u> table;

        // Fill table with 255
        for (u8 & v : table)
        {
            v = 255u;
        }

        // Set '0' - '9'
        for (unat v{0u}; v < 10u; ++v)
        {
            table[unat('0') + v] = u8(v);
        }

        // Set 'A' - 'F' and 'a' - 'f'
        for (unat v{0u}; v < 6u; ++v)
        {
            table[unat('A') + v] = u8(10u + v);
            table[unat('a') + v] = u8(10u + v);
        }

        return table;
    }

    inline bool Decoder::_consumeBinary(u64 & val, const bool wasZero)
    {
        const char * start{_pos};

        u64 b;
        if (_pos >= _end || (b = u64(*_pos - '0')) > 1u)
        {
            if (wasZero)
            {
                val = 0u;
                return true;
            }
            else
            {
                string = "Invalid binary digit";
                return false;
            }
        }
        val = b;

        while (++_pos < _end && (b = u64(*_pos - '0')) <= 1u)
        {
            val = (val << 1) | b;
        }

        const unat length{unat(_pos - start)};
        if (length <= 64u)
        {
            return true;
        }
        else
        {
            string = "Binary integer too large";
            return false;
        }
    }

    inline bool Decoder::_consumeOctal(u64 & val, const bool wasZero)
    {
        const char * start{_pos};

        u64 o;
        if (_pos >= _end || (o = u64(*_pos - '0')) > 7u)
        {
            if (wasZero)
            {
                val = 0u;
                return true;
            }
            else
            {
                string = "Invalid octal digit";
                return false;
            }
        }
        val = o;

        const unat headBits{64u - unat(std::countl_zero(o))};

        while (++_pos < _end && (o = u64(*_pos - '0')) <= 7u)
        {
            val = (val << 3) | o;
        }

        const unat bits{unat(_pos - start - 1) * 3u + headBits};
        if (bits <= 64u)
        {
            return true;
        }
        else
        {
            string = "Octal integer too large";
            return false;
        }
    }

    inline bool Decoder::_consumeHex(u64 & val, const bool wasZero)
    {
        static constexpr std::array<u8, 256u> hexTable{_createHexTable()};

        const char * start{_pos};

        u64 h;
        if (_pos >= _end || (h = hexTable[u8(*_pos)]) > 15u)
        {
            if (wasZero)
            {
                val = 0u;
                return true;
            }
            else
            {
                string = "Invalid hex digit";
                return false;
            }
        }
        val = h;

        while (++_pos < _end && (h = hexTable[u8(*_pos)]) <= 15u)
        {
            val = (val << 4) | h;
        }

        const unat length{unat(_pos - start)};
        if (length <= 16u)
        {
            return true;
        }
        else
        {
            string = "Hex integer too large";
            return false;
        }
    }

    inline bool Decoder::_consumeDecimal(u64 & val, const char * const end)
    {
        static constexpr u64 riskyVal{std::numeric_limits<u64>::max() / 10u};
        static constexpr u64 riskyDigit{std::numeric_limits<u64>::max() % 10u};

        val = 0u;

        for (u64 d; _pos < end && (d = u64(*_pos - '0')) <= 9u; ++_pos)
        {
            // Check if would overflow
            if (val > riskyVal || val == riskyVal && d > riskyDigit) [[unlikely]]
            {
                string = "Decimal integer too large";
                return false;
            }

            val = val * 10u + d;
        }

        return true;
    }

    inline bool Decoder::_consumeFloater(const int sign)
    {
        const std::from_chars_result res{std::from_chars(_pos, _end, floater)};

        // There was an issue parsing
        if (res.ec != std::errc{})
        {
            string = "Invalid floater"sv;
            return false;
        }

        if (sign >= 0)
        {
            boolean = true;
        }
        else
        {
            floater = -floater;
            boolean = false;
        }

        _pos = res.ptr;
        return true;
    }

    template <int base>
    inline bool Decoder::_consumeInteger(const int sign, const char * end)
    {
        u64 val;

        // Skip leading zeroes
        bool wasZero{false};
        while (_pos < _end && *_pos == '0')
        {
            ++_pos;
            wasZero = true;
        }

        bool res;
        if constexpr (base == 2)
        {
            res = _consumeBinary(val, wasZero);
        }
        else if constexpr (base == 8)
        {
            res = _consumeOctal(val, wasZero);
        }
        else if constexpr (base == 16)
        {
            res = _consumeHex(val, wasZero);
        }
        else if constexpr (base == 10)
        {
            res = _consumeDecimal(val, end);
        }
        if (!res)
        {
            string = "Invalid integer";
            return false;
        }

        if (sign >= 0)
        {
            integer = s64(val);
            boolean = true;
        }
        else
        {
            // The integer is too large to fit in an `s64` when negative
            if (val > u64(std::numeric_limits<s64>::min()))
            {
                string = "Negative integer too large"sv;
                return false;
            }

            integer = -s64(val);
            boolean = false;
        }

        return true;
    }

    inline void Decoder::_ingestNumber(const int sign)
    {
        // Check if hex/octal/binary
        if (*_pos == '0')
        {
            ++_pos;

            if (_pos < _end)
            {
                if (*_pos == 'b')
                {
                    ++_pos;
                    _state = _consumeInteger<2>(sign, nullptr) ? DecodeState::integer : DecodeState::error;
                    return;
                }
                else if (*_pos == 'o')
                {
                    ++_pos;
                    _state = _consumeInteger<8>(sign, nullptr) ? DecodeState::integer : DecodeState::error;
                    return;
                }
                else if (*_pos == 'x')
                {
                    ++_pos;
                    _state = _consumeInteger<16>(sign, nullptr) ? DecodeState::integer : DecodeState::error;
                    return;
                }
            }

            --_pos;
        }

        // Determine if integer or floater
        const char * integerEnd{_pos + 1};
        for (; integerEnd < _end && _isDigit(*integerEnd); ++integerEnd);
        if (integerEnd >= _end)
        {
            _state = _consumeInteger<10>(sign, integerEnd) ? DecodeState::integer : DecodeState::error;
        }
        else if (*integerEnd == 'e' || *integerEnd == 'E')
        {
            _state = _consumeFloater(sign) ? DecodeState::floater : DecodeState::error;
        }
        else if (*integerEnd == '.')
        {
            if (integerEnd + 1 < _end && _isDigit(integerEnd[1]))
            {
                _state = _consumeFloater(sign) ? DecodeState::floater : DecodeState::error;
            }
            else
            {
                string = "Number must not have trailing decimal"sv;
                _state = DecodeState::error;
            }
        }
        else
        {
            _state = _consumeInteger<10>(sign, integerEnd) ? DecodeState::integer : DecodeState::error;
        }
    }
}
