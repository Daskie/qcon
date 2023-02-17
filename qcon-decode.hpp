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
#include <chrono>
#include <format>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <qcon-common.hpp>

namespace qcon
{
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
        date,
        time,
        datetime,
        done
    };

    class Decoder
    {
      public:

        std::string key{};
        std::string string{};
        s64 integer{};
        double floater{};
        bool boolean{};
        bool positive{};
        Datetime datetime{};
        Date & date{datetime.date};
        Time & time{datetime.time};
        std::string errorMessage{};

        Decoder() = default;
        Decoder(std::string_view qson);

        void load(std::string_view qcon);

        DecodeState step();

        DecodeState state() const { return _state; }

        const char * position() const { return _pos; }

      private:

        DecodeState _state{DecodeState::done};
        const char * _start{};
        const char * _end{};
        const char * _pos{};
        u64 _stack{};
        unat _depth{};

        void _skipSpace();

        void _skipSpaceAndComments();

        int _tryConsumeSign();

        bool _tryConsumeChar(char c);

        bool _tryConsumeChars(std::string_view str);

        [[nodiscard]] bool _consumeChar(char c);

        [[nodiscard]] bool _consumeCodePoint(char & c, int digits);

        [[nodiscard]] bool _consumeEscaped(char & c);

        [[nodiscard]] bool _consumeString(std::string & dst);

        [[nodiscard]] bool _consumeKey();

        [[nodiscard]] bool _consumeBinary(u64 & v);

        [[nodiscard]] bool _consumeOctal(u64 & v);

        [[nodiscard]] bool _consumeDecimal(u64 & v);

        [[nodiscard]] bool _consumeHex(u64 & v);

        [[nodiscard]] bool _consumeFloater(int sign);

        template <int base> [[nodiscard]] bool _consumeInteger(int sign);

        void _ingestNumber(int sign);

        bool _tryConsumeDecimalDigits(u64 & v, unat digits);

        [[nodiscard]] bool _consumeDecimalDigits(u64 & v, unat digits);

        [[nodiscard]] bool _consumeDate();

        [[nodiscard]] bool _consumeTime();

        [[nodiscard]] bool _consumeTimezone();
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
                errorMessage = "Expected content"sv;
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
                    errorMessage = "Expected comma"sv;
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
                    errorMessage = "Expected comma"sv;
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
                errorMessage = "Extraneous content"sv;
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
                    errorMessage = "Exceeded max depth of 64"sv;
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
                    errorMessage = "Exceeded max depth of 64"sv;
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
                    else if (*_pos == 'i')
                    {
                        ++_pos;
                        if (_tryConsumeChars("nf"sv))
                        {
                            floater = std::numeric_limits<double>::infinity();
                            positive = true;
                            return _state = DecodeState::floater;
                        }
                        --_pos;
                    }
                }
                --_pos;
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
                    else if (*_pos == 'i')
                    {
                        ++_pos;
                        if (_tryConsumeChars("nf"sv))
                        {
                            floater = -std::numeric_limits<double>::infinity();
                            positive = false;
                            return _state = DecodeState::floater;
                        }
                        --_pos;
                    }
                }
                --_pos;
                break;
            }
            case 'i':
            {
                ++_pos;
                if (_tryConsumeChars("nf"sv))
                {
                    floater = std::numeric_limits<double>::infinity();
                    positive = true;
                    return _state = DecodeState::floater;
                }
                --_pos;
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
                --_pos;
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
                --_pos;
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
                    positive = true;
                    return _state = DecodeState::floater;
                }
                --_pos;
                break;
            }
            case 'D':
            {
                ++_pos;
                if (!_consumeDate())
                {
                    return _state = DecodeState::error;
                }

                if (_pos < _end && *_pos == 'T')
                {
                    ++_pos;
                    return _state = _consumeTime() ? DecodeState::datetime : DecodeState::error;
                }
                else
                {
                    return _state = DecodeState::date;
                }
            }
            case 'T':
            {
                ++_pos;
                return _state = _consumeTime() ? DecodeState::time : DecodeState::error;
            }
        }

        errorMessage = "Unknown value"sv;
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

    inline int Decoder::_tryConsumeSign()
    {
        int sign{0};

        if (_pos < _end)
        {
            sign = (*_pos == '+') - (*_pos == '-');

            if (sign)
            {
                ++_pos;
            }
        }

        return sign;
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
            errorMessage.clear();
            std::format_to(std::back_inserter(errorMessage), "Expected `{}`"sv, c);
            _state = DecodeState::error;
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeCodePoint(char & c, const int digits)
    {
        if (_end - _pos < digits)
        {
            errorMessage.clear();
            std::format_to(std::back_inserter(errorMessage), "Expected {} code point digits"sv, digits);
            return false;
        }

        u32 v;
        const std::from_chars_result res{std::from_chars(_pos, _pos + digits, v, 16)};
        if (res.ec != std::errc{})
        {
            errorMessage = "Invalid code point"sv;
            return false;
        }

        _pos += digits;

        c = char(v);
        return true;
    }

    inline bool Decoder::_consumeEscaped(char & c)
    {
        if (_pos >= _end)
        {
            errorMessage = "Expected escape sequence"sv;
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
                    --_pos;
                    errorMessage = "Invalid escape sequence"sv;
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
                errorMessage = "Expected end quote"sv;
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
                errorMessage = "Invalid string content"sv;
                return false;
            }
        }
    }

    inline bool Decoder::_consumeKey()
    {
        if (_pos >= _end || *_pos != '"')
        {
            errorMessage = "Expected key"sv;
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

    inline bool Decoder::_consumeBinary(u64 & v)
    {
        const char * start{_pos};

        v = 0u;

        for (u64 b; _pos < _end && (b = u64(*_pos - '0')) <= 1u; ++_pos)
        {
            // Check if would overflow
            if (v & (u64(0b1u) << 63))
            {
                errorMessage = "Binary integer too large";
                return false;
            }

            v = (v << 1) | b;
        }

        if (_pos == start)
        {
            errorMessage = "Missing binary digit";
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeOctal(u64 & v)
    {
        const char * start{_pos};

        v = 0u;

        for (u64 o; _pos < _end && (o = u64(*_pos - '0')) <= 7u; ++_pos)
        {
            // Check if would overflow
            if (v & (u64(0b111u) << 61))
            {
                errorMessage = "Octal integer too large";
                return false;
            }

            v = (v << 3) | o;
        }

        if (_pos == start)
        {
            errorMessage = "Missing octal digit";
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeDecimal(u64 & v)
    {
        static constexpr u64 riskyVal{std::numeric_limits<u64>::max() / 10u};
        static constexpr u64 riskyDigit{std::numeric_limits<u64>::max() % 10u};

        const char * start{_pos};

        v = 0u;

        for (u64 d; _pos < _end && (d = u64(*_pos - '0')) <= 9u; ++_pos)
        {
            // Check if would overflow
            if (v > riskyVal || v == riskyVal && d > riskyDigit) [[unlikely]]
            {
                errorMessage = "Decimal integer too large";
                return false;
            }

            v = v * 10u + d;
        }

        if (_pos == start)
        {
            errorMessage = "Missing decimal digit";
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeHex(u64 & v)
    {
        static constexpr std::array<u8, 256u> hexTable{_createHexTable()};

        const char * start{_pos};

        v = 0u;

        for (u64 h; _pos < _end && (h = hexTable[u8(*_pos)]) <= 15u; ++_pos)
        {
            // Check if would overflow
            if (v & (u64(0b1111u) << 60))
            {
                errorMessage = "Hex integer too large";
                return false;
            }

            v = (v << 4) | h;
        }

        if (_pos == start)
        {
            errorMessage = "Missing hex digit";
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeFloater(const int sign)
    {
        const std::from_chars_result res{std::from_chars(_pos, _end, floater)};

        // There was an issue parsing
        if (res.ec != std::errc{})
        {
            errorMessage = "Invalid floater"sv;
            return false;
        }

        if (sign >= 0)
        {
            positive = true;
        }
        else
        {
            floater = -floater;
            positive = false;
        }

        _pos = res.ptr;
        return true;
    }

    template <int base>
    inline bool Decoder::_consumeInteger(const int sign)
    {
        u64 v;

        bool res;
        if constexpr (base == 2)
        {
            res = _consumeBinary(v);
        }
        else if constexpr (base == 8)
        {
            res = _consumeOctal(v);
        }
        else if constexpr (base == 10)
        {
            res = _consumeDecimal(v);
        }
        else if constexpr (base == 16)
        {
            res = _consumeHex(v);
        }
        if (!res)
        {
            errorMessage = "Invalid integer";
            return false;
        }

        if (sign >= 0)
        {
            integer = s64(v);
            positive = true;
        }
        else
        {
            // The integer is too large to fit in an `s64` when negative
            if (v > u64(std::numeric_limits<s64>::min()))
            {
                errorMessage = "Negative integer too large"sv;
                return false;
            }

            integer = -s64(v);
            positive = false;
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
                    _state = _consumeInteger<2>(sign) ? DecodeState::integer : DecodeState::error;
                    return;
                }
                else if (*_pos == 'o')
                {
                    ++_pos;
                    _state = _consumeInteger<8>(sign) ? DecodeState::integer : DecodeState::error;
                    return;
                }
                else if (*_pos == 'x')
                {
                    ++_pos;
                    _state = _consumeInteger<16>(sign) ? DecodeState::integer : DecodeState::error;
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
            _state = _consumeInteger<10>(sign) ? DecodeState::integer : DecodeState::error;
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
                errorMessage = "Number must not have trailing decimal"sv;
                _state = DecodeState::error;
            }
        }
        else
        {
            _state = _consumeInteger<10>(sign) ? DecodeState::integer : DecodeState::error;
        }
    }

    inline bool Decoder::_tryConsumeDecimalDigits(u64 & v, unat digits)
    {
        // Assumed to never overflow

        if (_pos + digits > _end)
        {
            return false;
        }

        v = 0u;

        for (; digits; --digits, ++_pos)
        {
            const u64 d{u64(*_pos - '0')};

            if (d > 9u)
            {
                return false;
            }

            v = v * 10u + d;
        }

        return true;
    }

    inline bool Decoder::_consumeDecimalDigits(u64 & v, const unat digits)
    {
        if (_tryConsumeDecimalDigits(v, digits))
        {
            return true;
        }
        else
        {
            errorMessage.clear();
            std::format_to(std::back_inserter(errorMessage), "Expected {} decimal digits"sv, digits);
            return false;
        }
    }

    inline u8 _lastMonthDay(const u16 year, const u8 month)
    {
        static constexpr std::array<u8, 16u> monthDayCounts{31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u, 0u, 0u, 0u, 0u};

        if (month == 2u)
        {
            return 28u + std::chrono::year{year}.is_leap();
        }
        else
        {
            return monthDayCounts[(month - 1u) & 15u];
        }
    }

    // Utterly ignoring leap seconds with righteous conviction
    inline bool Decoder::_consumeDate()
    {
        // Already consumed `D`

        // Consume year
        u64 year;
        if (!_consumeDecimalDigits(year, 4u))
        {
            return false;
        }
        date.year = u16(year);

        if (!_consumeChar('-'))
        {
            return false;
        }

        // Consume month
        u64 month;
        if (!_consumeDecimalDigits(month, 2u))
        {
            return false;
        }
        if (month == 0u || month > 12u)
        {
            errorMessage = "Invalid month"sv;
            return false;
        }
        date.month = u8(month);

        if (!_consumeChar('-'))
        {
            return false;
        }

        // Consume day
        u64 day;
        if (!_consumeDecimalDigits(day, 2u))
        {
            return false;
        }
        if (day == 0u || day > _lastMonthDay(date.year, date.month))
        {
            errorMessage = "Invalid day"sv;
            return false;
        }
        date.day = u8(day);

        return true;
    }

    // Utterly ignoring leap seconds with righteous conviction
    inline bool Decoder::_consumeTime()
    {
        // Already consumed `T`

        // Consume hour
        u64 hour;
        if (!_consumeDecimalDigits(hour, 2u))
        {
            return false;
        }
        if (hour >= 24u)
        {
            _pos -= 2;
            errorMessage = "Invalid hour"sv;
            return false;
        }
        time.hour = u8(hour);

        if (!_consumeChar(':'))
        {
            return false;
        }

        // Consume minute
        u64 minute;
        if (!_consumeDecimalDigits(minute, 2u))
        {
            return false;
        }
        if (minute >= 60u)
        {
            _pos -= 2;
            errorMessage = "Invalid minute"sv;
            return false;
        }
        time.minute = u8(minute);

        if (!_consumeChar(':'))
        {
            return false;
        }

        // Consume second
        u64 second;
        if (!_consumeDecimalDigits(second, 2u))
        {
            return false;
        }
        if (second >= 60u)
        {
            _pos -= 2;
            errorMessage = "Invalid second"sv;
            return false;
        }
        time.second = u8(second);

        // Consume subseconds
        if (_tryConsumeChar('.'))
        {
            const char * const start{_pos};
            u64 subsecond;
            if (!_consumeDecimal(subsecond))
            {
                return false;
            }

            static constexpr std::array<u64, 20u> powersOf10{
                1u, 10u, 100u, 1'000u, 10'000u, 100'000u, 1'000'000u, 10'000'000u,
                100'000'000u, 1'000'000'000u, 10'000'000'000u, 100'000'000'000u, 1'000'000'000'000u,
                10'000'000'000'000u, 100'000'000'000'000u, 1'000'000'000'000'000u, 10'000'000'000'000'000u,
                100'000'000'000'000'000u, 1'000'000'000'000'000'000u, 10'000'000'000'000'000'000u};

            const unat digits{unat(_pos - start)};
            if (digits < 9u)
            {
                time.subsecond = u32(subsecond * powersOf10[9u - digits]);
            }
            else if (digits > 9u)
            {
                // Appropriate rounding
                const u64 divisor{powersOf10[digits - 9u]};
                time.subsecond = u32((subsecond + divisor / 2u) / divisor);
            }
            else
            {
                time.subsecond = u32(subsecond);
            }
        }
        else
        {
            time.subsecond = 0u;
        }

        return _consumeTimezone();
    }

    inline bool Decoder::_consumeTimezone()
    {
        // GMT time
        if (_tryConsumeChar('Z'))
        {
            time.zone.format = utc;
            time.zone.offset = 0u;
        }
        // Has timezone offset
        else if (int sign{_tryConsumeSign()}; sign)
        {
            u64 hour;
            if (!_consumeDecimalDigits(hour, 2u))
            {
                return false;
            }

            if (!_consumeChar(':'))
            {
                return false;
            }

            u64 minute;
            if (!_consumeDecimalDigits(minute, 2u))
            {
                return false;
            }
            if (minute > 59u)
            {
                _pos -= 2;
                errorMessage = "Invalid minute"sv;
                return false;
            }

            time.zone.format = utcOffset;
            time.zone.offset = s16(hour * 60u + minute);
            if (sign < 0) time.zone.offset = -time.zone.offset;
        }
        // No timezone, local time
        else
        {
            time.zone.format = localTime;
            time.zone.offset = 0u;
        }

        return true;
    }
}
