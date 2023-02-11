#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This standalone header provides a SAX decoder
/// See the README for more info and examples!
///

#include <cctype>

#include <charconv>
#include <format>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace qcon
{
    using uchar = unsigned char;

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
        int64_t integer;
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
        uint64_t _stack{};
        size_t _depth{};

        void _skipSpace();

        void _skipSpaceAndComments();

        bool _tryConsumeChar(char c);

        bool _tryConsumeChars(std::string_view str);

        [[nodiscard]] bool _consumeChar(char c);

        [[nodiscard]] bool _consumeCodePoint(char & c, int digits);

        [[nodiscard]] bool _consumeEscaped(char & c);

        [[nodiscard]] bool _consumeString(std::string & dst);

        [[nodiscard]] bool _consumeKey();

        [[nodiscard]] bool _consumeInteger(int sign, int base);

        [[nodiscard]] bool _consumeFloater(int sign);

        void _ingestNumber(int sign);
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
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
                    if (std::isdigit(*_pos))
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
                    if (std::isdigit(*_pos))
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
        for (; _pos < _end && std::isspace(uchar(*_pos)); ++_pos);
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
        if (size_t(_end - _pos) >= str.length())
        {
            for (size_t i{0u}; i < str.length(); ++i)
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

        uint32_t val;
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
                if (std::isprint(uchar(c)))
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
            else if (std::isprint(uchar(c)))
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

    inline bool Decoder::_consumeInteger(const int sign, const int base)
    {
        uint64_t val;

        const std::from_chars_result res{std::from_chars(_pos, _end, val, base)};

        // There was an issue parsing
        if (res.ec != std::errc{})
        {
            if (res.ec == std::errc::result_out_of_range)
            {
                string = "Integer too large"sv;
                return false;
            }
            // Some other issue
            else
            {
                string = "Invalid integer"sv;
                return false;
            }
        }

        if (sign >= 0)
        {
            integer = int64_t(val);
            boolean = true;
        }
        else
        {
            // The integer is too large to fit in an `int64_t` when negative
            if (val > uint64_t(std::numeric_limits<int64_t>::min()))
            {
                string = "Negative integer too large"sv;
                return false;
            }

            integer = -int64_t(val);
            boolean = false;
        }

        _pos = res.ptr;
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
                    _state = _consumeInteger(sign, 2) ? DecodeState::integer : DecodeState::error;
                    return;
                }
                else if (*_pos == 'o')
                {
                    ++_pos;
                    _state = _consumeInteger(sign, 8) ? DecodeState::integer : DecodeState::error;
                    return;
                }
                else if (*_pos == 'x')
                {
                    ++_pos;
                    _state = _consumeInteger(sign, 16) ? DecodeState::integer : DecodeState::error;
                    return;
                }
            }

            --_pos;
        }

        // Determine if integer or floater
        const char * integerEnd{_pos + 1};
        for (; integerEnd < _end && std::isdigit(*integerEnd); ++integerEnd);
        if (integerEnd >= _end)
        {
            _state = _consumeInteger(sign, 10) ? DecodeState::integer : DecodeState::error;
        }
        else if (*integerEnd == 'e' || *integerEnd == 'E')
        {
            _state = _consumeFloater(sign) ? DecodeState::floater : DecodeState::error;
        }
        else if (*integerEnd == '.')
        {
            if (integerEnd + 1 < _end && std::isdigit(integerEnd[1]))
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
            _state = _consumeInteger(sign, 10) ? DecodeState::integer : DecodeState::error;
        }
    }
}
