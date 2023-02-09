#pragma once

///
/// QC JSON 2.0.2
///
/// Quick and clean JSON5 header library for C++20
///
/// Austin Quick : 2019 - 2023
///
/// https://github.com/daskie/qc-json
///
/// This standalone header provides a SAX interface for decoding JSON5
///
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

#ifndef QC_JSON_COMMON
#define QC_JSON_COMMON

namespace qc::json
{
    using std::string;
    using std::string_view;
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    using uchar = unsigned char;

    ///
    /// Simple enum representing a json container type
    ///
    enum class Container : int8_t
    {
        none,
        object,
        array
    };

    ///
    /// Pass with an object or array to specify its density
    ///
    enum class Density : int8_t
    {
        unspecified = 0b000, /// Use that of the root or parent element
        multiline   = 0b001, /// Elements are put on new lines
        uniline     = 0b011, /// Elements are put on one line separated by spaces
        nospace     = 0b111  /// No whitespace is used whatsoever
    };
}

#endif // QC_JSON_COMMON

namespace qc::json
{
    struct DecodeResult
    {
        bool success{}; /// Whether the decode succeeded
        string errorMessage{}; /// Brief description of what went wrong
        size_t errorPosition{}; /// The index into the string where the error occurred
    };

    ///
    /// Decodes the JSON string
    ///
    /// A note on numbers:
    ///
    /// A number will be parsed and sent to the composer as either a `int64_t`, a `uint64_t`, or a `double`
    /// - `int64_t` if the number is an integer (a fractional component of zero is okay) and can fit in a `int64_t`
    /// - `uint64_t` if the number is a positive integer, can fit in a `uint64_t`, but cannot fit in a `int64_t`
    /// - `double` if the number has a non-zero fractional component, has an exponent, or is an integer that is too large to fit in a `int64_t` or `uint64_t`
    ///
    /// @param json the string to decode
    /// @param composer the contents of the JSON are decoded in order and passed to this to do something with
    /// @param initialState the initial state object to be passed to the composer
    ///
    template <typename Composer, typename State> [[nodiscard]] DecodeResult decode(string_view json, Composer & composer, State & initialState);
    template <typename Composer, typename State> [[nodiscard]] DecodeResult decode(string_view json, Composer & composer, State && initialState);

    ///
    /// An example composer whose operations are all no-ops
    ///
    /// Any custom composer must provide matching methods. This class may be extended for baseline no-ops
    ///
    template <typename State = nullptr_t>
    class DummyComposer
    {
      public:

        State object(State & /*outerState*/) { return State{}; }
        State array(State & /*outerState*/) { return State{}; }
        void end(const Density /*density*/, State && /*innerState*/, State & /*outerState*/) {}
        void key(const string_view /*key*/, State & /*state*/) {}
        void val(const string_view /*val*/, State & /*state*/) {}
        void val(const int64_t /*val*/, State & /*state*/) {}
        void val(const uint64_t /*val*/, State & /*state*/) {}
        void val(const double /*val*/, State & /*state*/) {}
        void val(const bool /*val*/, State & /*state*/) {}
        void val(const std::nullptr_t, State & /*state*/) {}
        void comment(const string_view /*comment*/, State & /*state*/) {}
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qc::json
{
    inline Density & operator&=(Density & d1, const Density d2)
    {
        reinterpret_cast<uint8_t &>(d1) &= uint8_t(int8_t(d2));
        return d1;
    }

    inline Density & operator|=(Density & d1, const Density d2)
    {
        reinterpret_cast<uint8_t &>(d1) |= uint8_t(int8_t(d2));
        return d1;
    }

    // This functionality is wrapped in a class purely as a convenient way to keep track of state
    template <typename Composer, typename State>
    class _Decoder
    {
      private:

        void _error(string && message, const char * const position)
        {
            _results.success = false;
            _results.errorMessage = std::move(message);
            _results.errorPosition = size_t(position - _start);
        }

      public:

        _Decoder(const string_view str, Composer & composer) :
            _start{str.data()},
            _end{_start + str.length()},
            _pos{_start},
            _composer{composer}
        {
            _results.success = true;
        }

        const DecodeResult & operator()(State & initialState)
        {
            Density density;

            if (!_skipSpaceAndIngestComments(density, initialState))
            {
                return _results;
            }

            if (!_ingestValue(initialState))
            {
                return _results;
            }

            if (!_skipSpaceAndIngestComments(density, initialState))
            {
                return _results;
            }

            // Allow trailing comma
            if (_tryConsumeChar(','))
            {
                if (!_skipSpaceAndIngestComments(density, initialState))
                {
                    return _results;
                }
            }

            if (_pos != _end)
            {
                _error("Extraneous content", _pos);
            }

            return _results;
        }

      private:

        const char * const _start{nullptr};
        const char * const _end{nullptr};
        const char * _pos{nullptr};
        size_t _line{0u};
        size_t _column{0u};
        Composer & _composer;
        string _stringBuffer{};
        DecodeResult _results{};

        Density _skipWhitespace()
        {
            Density density{Density::nospace};

            while (_pos < _end)
            {
                if (std::isspace(uchar(*_pos)))
                {
                    if (*_pos == '\n') density &= Density::multiline;
                    else density &= Density::uniline;
                    ++_pos;
                }
                else
                {
                    break;
                }
            }

            return density;
        }

        Density _ingestLineComment(bool concat, State & state)
        {
            // We already know we have `//`
            _pos += 2;
            const char * commentStart{_pos};

            // Seek to end of line
            while (_pos < _end && *_pos != '\n')
            {
                ++_pos;
            }
            const char * commentEnd{_pos};

            // Trim space after `//`
            if (commentStart < commentEnd && *commentStart == ' ')
            {
                ++commentStart;
            }

            // Trim `\r` from end
            if (commentEnd[-1] == '\r')
            {
                --commentEnd;
            }

            // If this is a continuation, add it to the buffer
            if (concat)
            {
                _stringBuffer.push_back('\n');
                _stringBuffer.append(commentStart, commentEnd);
            }

            // Check for continuation on next line
            bool isContinuation{false};
            Density density{Density::nospace};

            // This comment ended with a newline (as opposed to the end of the json)
            if (_pos < _end)
            {
                // Skip newline
                ++_pos;
                density = Density::multiline;

                // There are no additional newlines
                if (_skipWhitespace() > Density::multiline)
                {
                    isContinuation = _pos + 2 < _end && _pos[0] == '/' && _pos[1] == '/';
                }
            }

            // There is more comment to come
            if (isContinuation)
            {
                if (!concat)
                {
                    _stringBuffer.assign(commentStart, commentEnd);
                }

                _ingestLineComment(true, state);

                if (!concat)
                {
                    _composer.comment(string_view{_stringBuffer}, state);
                }
            }
            // This is the end of the comment
            else
            {
                if (!concat)
                {
                    _composer.comment(string_view{commentStart, size_t(commentEnd - commentStart)}, state);
                }
            }

            return density;
        }

        [[nodiscard]] bool _ingestBlockComment(State & state)
        {
            // We already know we have `/*`
            _pos += 2;
            const char * commentStart{_pos};

            // Seek to `*/`
            while (_pos + 1 < _end && !(_pos[0] == '*' && _pos[1] == '/'))
            {
                ++_pos;
            }

            // `*/` not found
            if (_pos + 1 >= _end)
            {
                _error("Block comment is unterminated", commentStart - 2);
                return false;
            }

            const char * commentEnd{_pos};
            _pos += 2;

            // Trim space after `/*`
            if (*commentStart == ' ')
            {
                ++commentStart;
            }

            // Trim space before `*/`
            if (commentEnd > commentStart && commentEnd[-1] == ' ')
            {
                --commentEnd;
            }

            _composer.comment(string_view{commentStart, size_t(commentEnd - commentStart)}, state);
            return true;
        }

        [[nodiscard]] bool _skipSpaceAndIngestComments(Density & density, State & state)
        {
            // Skip whitespace
            density = _skipWhitespace();

            while (true)
            {
                // Check for comment
                if (_pos + 1 < _end && _pos[0] == '/')
                {
                    // Ingest line comment
                    if (_pos[1] == '/')
                    {
                        density &= _ingestLineComment(false, state);
                        // `_ingesetLineComment` skips trailing whitespace already
                        continue;
                    }
                    // Ingest block comment
                    else if (_pos[1] == '*')
                    {
                        if (!_ingestBlockComment(state))
                        {
                            return false;
                        }

                        // Skip whitespace
                        density &= _skipWhitespace();
                        continue;
                    }
                }

                break;
            }

            return true;
        }

        bool _tryConsumeChar(const char c)
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

        [[nodiscard]] bool _consumeChar(const char c)
        {
            if (!_tryConsumeChar(c))
            {
                _error(std::format("Expected `{}`", c), _pos);
                return false;
            }

            return true;
        }

        bool _tryConsumeChars(const string_view str)
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

        [[nodiscard]] bool _consumeChars(const string_view str)
        {
            if (!_tryConsumeChars(str))
            {
                _error(std::format("Expected `{}`", str), _pos);
                return false;
            }

            return true;
        }

        [[nodiscard]] bool _ingestValue(State & state)
        {
            if (_pos >= _end)
            {
                _error("Expected value", _pos);
                return false;
            }

            char c{*_pos};

            // First check for typical easy values

            switch (c)
            {
                case '{':
                {
                    return _ingestObject(state);
                }
                case '[':
                {
                    return _ingestArray(state);
                }
                case '"':
                {
                    return _ingestString('"', state);
                }
                case '\'':
                {
                    return _ingestString('\'', state);
                }
            }

            // Now determine whether there is a +/- sign to narrow it down to numbers or not

            const int sign{(c == '+') - (c == '-')};
            if (sign)
            {
                // There was a sign, so we'll keep track of that and increment our position
                ++_pos;
                if (_pos >= _end)
                {
                    _error("Expected number", _pos);
                    return false;
                }
                c = *_pos;
            }
            else
            {
                // There was no sign, so we can check the non-number keywords
                if (_tryConsumeChars("true"sv))
                {
                    _composer.val(true, state);
                    return true;
                }
                else if (_tryConsumeChars("false"sv))
                {
                    _composer.val(false, state);
                    return true;
                }
                else if (_tryConsumeChars("null"sv))
                {
                    _composer.val(nullptr, state);
                    return true;
                }
            }

            // At this point, we know it is a number (or invalid)

            if (std::isdigit(uchar(c)) || (c == '.' && _pos + 1 < _end && std::isdigit(_pos[1])))
            {
                return _ingestNumber(sign, state);
            }
            else if (_tryConsumeChars("nan"sv) || _tryConsumeChars("NaN"sv))
            {
                _composer.val(std::numeric_limits<double>::quiet_NaN(), state);
                return true;
            }
            else if (_tryConsumeChars("inf"sv) || _tryConsumeChars("Infinity"sv))
            {
                _composer.val(sign < 0 ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity(), state);
                return true;
            }

            // Nothing matched
            _error("Unknown value", _pos);
            return false;
        }

        [[nodiscard]] bool _ingestObject(State & outerState)
        {
            State innerState{_composer.object(outerState)};

            ++_pos; // We already know we have `{`

            Density density;
            if (!_skipSpaceAndIngestComments(density, innerState))
            {
                return false;
            }

            if (!_tryConsumeChar('}'))
            {
                while (true)
                {
                    // Parse key
                    if (_pos >= _end)
                    {
                        _error("Expected key", _pos);
                        return false;
                    }
                    const char c{*_pos};
                    string_view key;
                    if (c == '"' || c == '\'')
                    {
                        if (!_consumeString(key, c))
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if (!_consumeIdentifier(key))
                        {
                            return false;
                        }
                    }
                    _composer.key(key, innerState);

                    Density density_;
                    if (!_skipSpaceAndIngestComments(density_, innerState))
                    {
                        return false;
                    }
                    density &= density_;

                    if (!_consumeChar(':'))
                    {
                        return false;
                    }

                    if (!_skipSpaceAndIngestComments(density_, innerState))
                    {
                        return false;
                    }
                    density &= density_;

                    if (!_ingestValue(innerState))
                    {
                        return false;
                    }

                    if (!_skipSpaceAndIngestComments(density_, innerState))
                    {
                        return false;
                    }
                    density &= density_;

                    if (_tryConsumeChar('}'))
                    {
                        break;
                    }
                    else
                    {
                        if (!_consumeChar(','))
                        {
                            return false;
                        }

                        if (!_skipSpaceAndIngestComments(density_, innerState))
                        {
                            return false;
                        }
                        density &= density_;

                        // Allow trailing comma
                        if (_tryConsumeChar('}'))
                        {
                            break;
                        }
                    }
                }
            }

            _composer.end(density, std::move(innerState), outerState);
            return true;
        }

        [[nodiscard]] bool _ingestArray(State & outerState)
        {
            State innerState{_composer.array(outerState)};

            ++_pos; // We already know we have `[`

            Density density;
            if (!_skipSpaceAndIngestComments(density, innerState))
            {
                return false;
            }

            if (!_tryConsumeChar(']'))
            {
                while (true)
                {
                    if (!_ingestValue(innerState))
                    {
                        return false;
                    }

                    Density density_;
                    if (!_skipSpaceAndIngestComments(density_, innerState))
                    {
                        return false;
                    }
                    density &= density_;

                    if (_tryConsumeChar(']'))
                    {
                        break;
                    }
                    else
                    {
                        if (!_consumeChar(','))
                        {
                            return false;
                        }

                        if (!_skipSpaceAndIngestComments(density_, innerState))
                        {
                            return false;
                        }
                        density &= density_;

                        // Allow trailing comma
                        if (_tryConsumeChar(']'))
                        {
                            break;
                        }
                    }
                }
            }

            _composer.end(density, std::move(innerState), outerState);
            return true;
        }

        [[nodiscard]] bool _ingestString(const char quote, State & state)
        {
            string_view str;
            if (!_consumeString(str, quote))
            {
                return false;
            }
            _composer.val(str, state);
            return true;
        }

        [[nodiscard]] bool _consumeString(string_view & str, const char quote)
        {
            _stringBuffer.clear();

            ++_pos; // We already know we have `"` or `'`

            while (true)
            {
                if (_pos >= _end)
                {
                    _error("Expected end quote", _pos);
                    return false;
                }

                char c{*_pos};
                if (c == quote)
                {
                    ++_pos;
                    str = _stringBuffer;
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
                        _stringBuffer.push_back(c);
                    }
                }
                else if (std::isprint(uchar(c)))
                {
                    _stringBuffer.push_back(c);
                    ++_pos;
                }
                else
                {
                    _error("Invalid string content", _pos);
                    return false;
                }
            }
        }

        [[nodiscard]] bool _consumeEscaped(char & c)
        {
            if (_pos >= _end)
            {
                _error("Expected escape sequence", _pos);
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
                        _error("Invalid escape sequence", _pos - 1);
                        return false;
                    }
                }
            }
        }

        [[nodiscard]] bool _consumeCodePoint(char & c, const int digits)
        {
            if (_end - _pos < digits)
            {
                _error(std::format("Expected {} code point digits", digits), _pos);
                return false;
            }

            uint32_t val;
            const std::from_chars_result res{std::from_chars(_pos, _pos + digits, val, 16)};
            if (res.ec != std::errc{})
            {
                _error("Invalid code point", _pos);
                return false;
            }

            _pos += digits;

            c = char(val);
            return true;
        }

        [[nodiscard]] bool _consumeIdentifier(string_view & str)
        {
            _stringBuffer.clear();

            // Ensure identifier is at least one character long
            char c{*_pos};
            if (std::isalnum(uchar(c)) || c == '_')
            {
                _stringBuffer.push_back(c);
                ++_pos;
            }
            else
            {
                _error("Expected identifier", _pos);
                return false;
            }

            while (true)
            {
                if (_pos >= _end)
                {
                    str = _stringBuffer;
                    return true;
                }

                c = *_pos;
                if (std::isalnum(uchar(c)) || c == '_')
                {
                    _stringBuffer.push_back(c);
                    ++_pos;
                }
                else
                {
                    str = _stringBuffer;
                    return true;
                }
            }
        }

        // Returns the string length of the number, including trailing decimal point & zeroes, or `0` if it's not an integer
        size_t _isInteger() const
        {
            const char * pos{_pos};
            // Skip all leading digits
            while (pos < _end && std::isdigit(uchar(*pos))) ++pos;
            // If that's it, we're an integer
            if (pos >= _end)
            {
                return size_t(pos - _pos);
            }
            // If instead there is a decimal point...
            else if (*pos == '.')
            {
                ++pos;
                // Skip all zeroes
                while (pos < _end && *pos == '0') ++pos;
                // If there's a digit or an exponent, we must be a floater
                if (pos < _end && (std::isdigit(uchar(*pos)) || *pos == 'e' || *pos == 'E'))
                {
                    return 0;
                }
                // Otherwise, we're an integer
                else
                {
                    return size_t(pos - _pos);
                }
            }
            // If instead there is an exponent, we must be a floater
            else if (*pos == 'e' || *pos == 'E')
            {
                return 0;
            }
            // Otherwise, that's the end of the number, and we're an integer
            else
            {
                return size_t(pos - _pos);
            }
        }

        [[nodiscard]] bool _ingestNumber(const int sign, State & state)
        {
            // Check if hex/octal/binary
            if (*_pos == '0' && _pos + 1 < _end)
            {
                int base{0};
                switch (_pos[1])
                {
                    case 'x': case 'X': base = 16; break;
                    case 'o': case 'O': base =  8; break;
                    case 'b': case 'B': base =  2; break;
                }

                if (base)
                {
                    if (sign)
                    {
                        _error("Hex, octal, and binary numbers must not be signed", _pos);
                        return false;
                    }
                    _pos += 2;
                    return _ingestHexOctalBinary(base, state);
                }
            }

            // Determine if integer or floater
            if (size_t length{_isInteger()}; length)
            {
                if (sign < 0)
                {
                    return _ingestInteger<true>(length, state);
                }
                else
                {
                    return _ingestInteger<false>(length, state);
                }
            }
            else
            {
                return _ingestFloater(sign < 0, state);
            }
        }

        [[nodiscard]] bool _ingestHexOctalBinary(const int base, State & state)
        {
            uint64_t val;
            const std::from_chars_result res{std::from_chars(_pos, _end, val, base)};

            // There was an issue parsing
            if (res.ec != std::errc{})
            {
                _error(base == 2 ? "Invalid binary" : base == 8 ? "Invalid octal" : "Invalid hex", _pos);
                return false;
            }

            _pos = res.ptr;

            _composer.val(val, state);
            return true;
        }

        template <bool negative>
        [[nodiscard]] bool _ingestInteger(const size_t length, State & state)
        {
            std::conditional_t<negative, int64_t, uint64_t> val;

            // Edge case that `.0` should evaluate to the integer `0`
            if (*_pos == '.')
            {
                val = 0;
            }
            else
            {
                const std::from_chars_result res{std::from_chars(_pos - negative, _end, val)};

                // There was an issue parsing
                if (res.ec != std::errc{})
                {
                    // If too large, parse as a floater instead
                    if (res.ec == std::errc::result_out_of_range)
                    {
                        return _ingestFloater(negative, state);
                    }
                    // Some other issue
                    else
                    {
                        _error("Invalid integer", _pos);
                        return false;
                    }
                }
            }

            _pos += length;

            // If unsigned and the most significant bit is not set, we default to reporting it as signed
            if constexpr (!negative)
            {
                if (!(val & 0x8000000000000000u))
                {
                    _composer.val(int64_t(val), state);
                    return true;
                }
            }

            _composer.val(val, state);
            return true;
        }

        [[nodiscard]] bool _ingestFloater(const bool negative, State & state)
        {
            double val;
            const std::from_chars_result res{std::from_chars(_pos - negative, _end, val)};

            // There was an issue parsing
            if (res.ec != std::errc{})
            {
                _error("Invalid floater", _pos);
                return false;
            }

            _pos = res.ptr;
            _composer.val(val, state);
            return true;
        }
    };

    template <typename Composer, typename State> concept _ComposerHasObjectMethod = requires (Composer composer, State state) { State{composer.object(state)}; };
    template <typename Composer, typename State> concept _ComposerHasArrayMethod = requires (Composer composer, State state) { State{composer.array(state)}; };
    template <typename Composer, typename State> concept _ComposerHasEndMethod = requires (Composer composer, const Density density, State innerState, State outerState) { composer.end(density, std::move(innerState), outerState); };
    template <typename Composer, typename State> concept _ComposerHasKeyMethod = requires (Composer composer, const string_view key, State state) { composer.key(key, state); };
    template <typename Composer, typename State> concept _ComposerHasStringValMethod = requires (Composer composer, const string_view val, State state) { composer.val(val, state); };
    template <typename Composer, typename State> concept _ComposerHasSignedIntegerValMethod = requires (Composer composer, const int64_t val, State state) { composer.val(val, state); };
    template <typename Composer, typename State> concept _ComposerHasUnsignedIntegerValMethod = requires (Composer composer, const uint64_t val, State state) { composer.val(val, state); };
    template <typename Composer, typename State> concept _ComposerHasFloaterValMethod = requires (Composer composer, const double val, State state) { composer.val(val, state); };
    template <typename Composer, typename State> concept _ComposerHasBooleanValMethod = requires (Composer composer, const bool val, State state) { composer.val(val, state); };
    template <typename Composer, typename State> concept _ComposerHasNullValMethod = requires (Composer composer, State state) { composer.val(nullptr, state); };
    template <typename Composer, typename State> concept _ComposerHasCommentMethod = requires (Composer composer, const string_view comment, State state) { composer.comment(comment, state); };

    template <typename Composer, typename State>
    inline DecodeResult decode(const string_view json, Composer & composer, State & initialState)
    {
        // Much more understandable compile errors than just letting the template code fly
        static_assert(_ComposerHasObjectMethod<Composer, State>);
        static_assert(_ComposerHasArrayMethod<Composer, State>);
        static_assert(_ComposerHasEndMethod<Composer, State>);
        static_assert(_ComposerHasKeyMethod<Composer, State>);
        static_assert(_ComposerHasStringValMethod<Composer, State>);
        static_assert(_ComposerHasSignedIntegerValMethod<Composer, State>);
        static_assert(_ComposerHasUnsignedIntegerValMethod<Composer, State>);
        static_assert(_ComposerHasFloaterValMethod<Composer, State>);
        static_assert(_ComposerHasBooleanValMethod<Composer, State>);
        static_assert(_ComposerHasNullValMethod<Composer, State>);
        static_assert(_ComposerHasCommentMethod<Composer, State>);

        return _Decoder<Composer, State>{json, composer}(initialState);
    }

    template <typename Composer, typename State>
    inline DecodeResult decode(string_view json, Composer & composer, State && initialState)
    {
        return decode(json, composer, initialState);
    }
}
