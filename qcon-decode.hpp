#pragma once

///
/// QCON 0.1.1
/// https://github.com/daskie/qcon
/// This header provides a SAX QCON decoder
/// See the README for more info
///

#include <array>
#include <charconv>
#include <chrono>
#include <format>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <qcon-common.hpp>

namespace qcon
{
    ///
    /// Represents the current state of the decoder
    ///
    enum class DecodeState
    {
        error,    /// An error has occurred
        ready,    /// The QSON was just loaded
        object,   /// An object was started
        array,    /// An array was started
        end,      /// An object or array was ended
        key,      /// A key was just decoded
        string,   /// A string value was decoded
        integer,  /// An integer value was decoded
        floater,  /// A floater value was decoded
        boolean,  /// A boolean value was decoded
        date,     /// A date value was decoded
        time,     /// A time value was decoded
        datetime, /// A datetime value was decoded
        null     /// A null value was decoded
    };

    ///
    /// Class to facilitate QCON SAX decoding
    /// A QCON string is loaded and decoded values may be extracted in sequence
    ///
    class Decoder
    {
      public:

        std::string key{};    /// If an object element was just decoded, holds its key; unspecified otherwise; may be moved from
        std::string string{}; /// If a string was just decoded, holds its value; unspecified otherwise; may be moved from
        s64 integer{};        /// If an integer was just decoded, holds its value; unspecified otherwise
        double floater{};     /// If a floater was just decoded, holds its value; unspecified otherwise
        bool positive{};      /// If a number was just decoded, indicates whether it was positive; unspecified otherwise
        bool boolean{};       /// If a boolean was just decoded, holds its value; unspecified otherwise
        Datetime datetime{};  /// If a datetime was just decoded, holds its value; unspecified otherwise
        Date & date{datetime.date}; /// If a date was just decoded, holds its value; unspecified otherwise; alias for `datetime.date`
        Time & time{datetime.time}; /// If a time was just decoded, holds its value; unspecified otherwise; alias for `datetime.time`
        std::string errorMessage{}; /// Holds a brief description of the most recent error; may be moved from

        Decoder();

        Decoder(const Decoder &) = delete;
        Decoder(Decoder && other);

        Decoder & operator=(const Decoder &) = delete;
        Decoder & operator=(Decoder && other);

        ///
        /// Constructs a decoder and loads the given QSON string
        /// The QSON string *must* be null terminated (optimization allowing most range checks to be eliminated)
        /// Equivalent to `Decoder d{}; d.load(qcon)`
        /// @param qson encoded QSON to load
        ///
        Decoder(const char * qson);
        Decoder(const std::string & qson) : Decoder{qson.c_str()} {}
        Decoder(std::string &&) = delete; // Prevent binding to temporary
        Decoder(std::string_view) = delete; /// QCON string must be null terminated; pass c-string instead

        ///
        /// @return whether the decoding has been thusfar successful
        ///
        explicit operator bool() const { return _state != DecodeState::error; }

        ///
        /// Loads the given QSON string, overriding any existing state
        /// The QSON string *must* be null terminated (optimization allowing most range checks to be eliminated)
        /// @param qson encoded QSON to load
        ///
        void load(const char * qson);
        void load(const std::string & qson) { load(qson.c_str()); }
        void load(std::string &&) = delete; /// Prevent binding to temporary
        void load(std::string_view) = delete; /// QCON string must be null terminated; pass c-string instead

        ///
        /// Decode the next QCON unit, which could be a value, key-value pair, container start, or container end
        /// Calling this after reaching the end of the QCON will yield an error
        /// Once in the `error` state, will stay in the `error` state until a new QCON string is loaded
        /// @return current state; `error` if there was an error
        ///
        DecodeState step();

        ///
        /// If at root, returns whether the value has yet to be consumed
        /// If at the end of a container, consumes the end brace/bracket and returns false
        /// If not at the end of a container, returns true
        /// If state is `error`, returns false
        /// @return whether there are more elements in the current container
        ///
        [[nodiscard]] bool more();

        ///
        /// @return whether the full QCON was successfully decoded
        ///
        [[nodiscard]] bool finished() const;

        ///
        /// Decode a value of the given type into the provided destination variable
        /// If the current state is `error`, does nothing
        /// If the decode is successful, progresses the internal state like `step()`
        /// If the decode is unsuccessful, sets the state to `error`
        /// May be used instead of, or in combination with, `step()`
        /// @return this
        ///
        Decoder & operator>>(Container container);
        Decoder & operator>>(std::string & v);
        Decoder & operator>>(std::string_view &) = delete;
        Decoder & operator>>(char *) = delete;
        Decoder & operator>>(const char *) = delete;
        Decoder & operator>>(char &); /// Expects a single character string
        Decoder & operator>>(s64 & v);
        Decoder & operator>>(u64 & v);
        Decoder & operator>>(s32 & v);
        Decoder & operator>>(u32 & v);
        Decoder & operator>>(s16 & v);
        Decoder & operator>>(u16 & v);
        Decoder & operator>>(s8 & v);
        Decoder & operator>>(u8 & v);
        Decoder & operator>>(double & v);
        Decoder & operator>>(float & v);
        Decoder & operator>>(bool & v);
        Decoder & operator>>(Date & v);
        Decoder & operator>>(Time & v);
        Decoder & operator>>(Datetime & v);
        Decoder & operator>>(Timepoint & v);
        Decoder & operator>>(nullptr_t);

        ///
        /// @return current state
        ///
        [[nodiscard]] DecodeState state() const { return _state; }

        ///
        /// @return current position within the QCON string
        ///
        const char * position() const { return _pos; }

      private:

        DecodeState _state;
        const char * _qcon;
        const char * _pos;
        u64 _stack;
        unat _depth;
        const char * _cachedEnd; // Only used for floating point `from_chars`
        bool _hadComma;

        void _reset();

        void _skipSpace();

        void _skipSpaceAndComments();

        [[nodiscard]] bool _preValueStreamCheck();

        [[nodiscard]] bool _preKeyStreamCheck();

        void _postValue(DecodeState newState);
        void _postValue(bool condition, DecodeState newState);

        int _tryConsumeSign();

        bool _tryConsumeChar(char c);

        bool _tryConsumeChars(std::string_view str);

        template <bool checkOverflow> bool _tryConsumeBinaryDigit(u64 & dst);

        template <bool checkOverflow> bool _tryConsumeOctalDigit(u64 & dst);

        template <bool checkOverflow> bool _tryConsumeDecimalDigit(u64 & dst);

        template <bool checkOverflow> bool _tryConsumeHexDigit(u64 & dst);

        bool _tryConsumeDecimalDigits(unat digits, u64 & dst);

        bool _tryConsumeHexDigits(unat digits, u64 & dst);

        [[nodiscard]] bool _consumeChar(char c);

        [[nodiscard]] bool _consumeChars(std::string_view str);

        [[nodiscard]] bool _consumeDecimalDigits(unat digits, u64 & dst);

        [[nodiscard]] bool _consumeHexDigits(unat digits, u64 & dst);

        [[nodiscard]] bool _consumeCodePoint(unat digits, std::string & dst);

        [[nodiscard]] bool _consumeEscaped(std::string & dst);

        [[nodiscard]] bool _consumeString(std::string & dst);

        [[nodiscard]] bool _consumeKey(std::string & dst);

        [[nodiscard]] bool _consumeBinaryInteger(u64 & dst);

        [[nodiscard]] bool _consumeOctalInteger(u64 & dst);

        [[nodiscard]] bool _consumeDecimalInteger(u64 & dst);

        [[nodiscard]] bool _consumeHexInteger(u64 & dst);

        [[nodiscard]] bool _consumeInteger(s64 & dst);

        [[nodiscard]] bool _consumeFloater(double & dst);

        [[nodiscard]] bool _consumeDate(Date & dst);

        [[nodiscard]] bool _consumeTime(Time & dst);

        [[nodiscard]] bool _consumeTimezone(Timezone & dst);

        void _ingestStart(Container container);

        void _ingestEnd();

        void _ingestNumber();

        void _ingestValue();

        template <typename T> void _streamSmallerSignedInteger(T & v);

        template <typename T> void _streamSmallerUnsignedInteger(T & v);
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
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

    [[nodiscard]] inline bool _isSpace(const char c)
    {
        switch (c)
        {
            case '\t': [[fallthrough]];
            case '\n': [[fallthrough]];
            case '\v': [[fallthrough]];
            case '\f': [[fallthrough]];
            case '\r': [[fallthrough]];
            case ' ': return true;
            default: return false;
        }
    }

    [[nodiscard]] inline bool _isFloater(const char * str)
    {
        while (_isDigit(*str)) ++str;
        if (*str == '.')
        {
            return _isDigit(str[1]);
        }
        else if (*str == 'e' || *str == 'E')
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    inline Decoder::Decoder()
    {
        _reset();
    }

    inline Decoder::Decoder(Decoder && other) :
        key{std::move(other.key)},
        string{std::move(other.string)},
        errorMessage{std::move(other.errorMessage)},

        _state{other._state},
        _qcon{other._qcon},
        _pos{other._pos},
        _stack{other._stack},
        _depth{other._depth},
        _cachedEnd{other._cachedEnd},
        _hadComma{other._hadComma}
    {
        other._reset();
    }

    inline Decoder & Decoder::operator=(Decoder && other)
    {
        key = std::move(other.key);
        string = std::move(other.string);
        errorMessage = std::move(other.errorMessage);

        _state = other._state;
        _qcon = other._qcon;
        _pos = other._pos;
        _stack = other._stack;
        _depth = other._depth;
        _cachedEnd = other._cachedEnd;
        _hadComma = other._hadComma;

        other._reset();
    }

    inline Decoder::Decoder(const char * const qcon)
    {
        load(qcon);
    }

    inline void Decoder::load(const char * const qcon)
    {
        _reset();

        _state = DecodeState::ready;
        _qcon = qcon;
        _pos = _qcon;

        _skipSpaceAndComments();

        // QCON missing value
        if (!*_pos)
        {
            errorMessage = "Expected value"sv;
            _state = DecodeState::error;
        }
    }

    inline DecodeState Decoder::step()
    {
        // Preserve error state
        if (_state == DecodeState::error)
        {
            return _state;
        }

        // In container
        if (_depth)
        {
            const bool inObject{bool(_stack & 1u)};
            if (inObject)
            {
                if (_state != DecodeState::key)
                {
                    // Check for end
                    if (*_pos == '}')
                    {
                        _ingestEnd();
                        return _state;
                    }
                    // Otherwise ingest key
                    else
                    {
                        // Ensure there was a comma between elements
                        if (_state != DecodeState::object && !_hadComma)
                        {
                            errorMessage = "Missing comma between object elements"sv;
                            return _state = DecodeState::error;
                        }

                        if (_consumeKey(key))
                        {
                            _skipSpaceAndComments();
                            return _state = DecodeState::key;
                        }
                        else
                        {
                            return _state = DecodeState::error;
                        }
                    }
                }
            }
            else
            {
                // Check for end
                if (*_pos == ']')
                {
                    _ingestEnd();
                    return _state;
                }

                // Ensure there was a comma between elements
                if (_state != DecodeState::array && !_hadComma)
                {
                    errorMessage = "Missing comma between array elements"sv;
                    return _state = DecodeState::error;
                }
            }
        }

        _ingestValue();
        return _state;
    }

    inline bool Decoder::more()
    {
        // Preserve error state
        if (_state == DecodeState::error)
        {
            return false;
        }

        if (_depth)
        {
            if (_state == DecodeState::key)
            {
                return true;
            }

            // Check if we have end brace/bracket
            const bool inObject{bool(_stack & 1u)};
            if (*_pos == (inObject ? '}' : ']'))
            {
                _ingestEnd();
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return _state == DecodeState::ready;
        }
    }

    inline bool Decoder::finished() const
    {
        return _state != DecodeState::error && _state != DecodeState::ready && !_depth && !*_pos;
    }

    inline Decoder & Decoder::operator>>(const Container container)
    {
        // Stream container end
        if (container == end)
        {
            if (more())
            {
                errorMessage = "There are more elements in the container"sv;
                _state = DecodeState::error;
            }

            return *this;
        }
        // Stream container start
        else
        {
            if (!_preValueStreamCheck())
            {
                return *this;
            }

            if (_consumeChar(container == Container::object ? '{' : '['))
            {
                _ingestStart(container);
            }
            else
            {
                _state = DecodeState::error;
            }

            return *this;
        }
    }

    inline Decoder & Decoder::operator>>(std::string & v)
    {
        // Stream key
        const bool inObject{bool(_stack & 1u)};
        if (inObject && _state != DecodeState::key)
        {
            if (!_preKeyStreamCheck())
            {
                return *this;
            }

            if (_consumeKey(v))
            {
                _state = DecodeState::key;
                _skipSpaceAndComments();
            }
            else
            {
                _state = DecodeState::error;
            }

            return *this;
        }
        // Stream value
        else
        {
            if (!_preValueStreamCheck())
            {
                return *this;
            }

            if (_consumeChar('"'))
            {
                _postValue(_consumeString(v), DecodeState::string);
            }
            else
            {
                _state = DecodeState::error;
            }

            return *this;
        }
    }

    inline Decoder & Decoder::operator>>(char & v)
    {
        const bool inObject{bool(_stack & 1u)};
        const bool isKey{inObject && _state != DecodeState::key};
        std::string & dst{isKey ? key : string};

        if (*this >> dst)
        {
            if (dst.size() == 1u)
            {
                v = dst.front();
            }
            else
            {
                errorMessage = "Expected single character string"sv;
                _state = DecodeState::error;
            }
        }

        return *this;
    }

    inline Decoder & Decoder::operator>>(s64 & v)
    {
        if (!_preValueStreamCheck())
        {
            return *this;
        }

        positive = _tryConsumeSign() >= 0;

        if (_isDigit(*_pos) && !_isFloater(_pos + 1))
        {
            _postValue(_consumeInteger(v), DecodeState::integer);
        }
        else
        {
            errorMessage = "Expected integer"sv;
            _state = DecodeState::error;
        }

        return *this;
    }

    inline Decoder & Decoder::operator>>(u64 & v)
    {
        s64 tmp;
        if (!(*this >> tmp))
        {
            return *this;
        }

        // Ensure value is positive
        if (positive)
        {
            v = u64(tmp);
        }
        else
        {
            errorMessage = "Cannot decode negative value into unsigned integer"sv;
            _state = DecodeState::error;
        }

        return *this;
    }

    template <typename T>
    inline void Decoder::_streamSmallerSignedInteger(T & v)
    {
        s64 v64;
        if (!(*this >> v64))
        {
            return;
        }

        // Verify range
        if (v64 >= std::numeric_limits<T>::min() && v64 <= std::numeric_limits<T>::max())
        {
            v = T(v64);
        }
        else
        {
            errorMessage = "Signed integer too large";
            _state = DecodeState::error;
        }
    }

    template <typename T>
    inline void Decoder::_streamSmallerUnsignedInteger(T & v)
    {
        u64 v64;
        *this >> v64;

        // Verify range
        if (v64 <= std::numeric_limits<T>::max())
        {
            v = T(v64);
        }
        else
        {
            errorMessage = "Unsigned integer too large";
            _state = DecodeState::error;
        }
    }

    inline Decoder & Decoder::operator>>(s32 & v)
    {
        _streamSmallerSignedInteger(v);
        return *this;
    }

    inline Decoder & Decoder::operator>>(u32 & v)
    {
        _streamSmallerUnsignedInteger(v);
        return *this;
    }

    inline Decoder & Decoder::operator>>(s16 & v)
    {
        _streamSmallerSignedInteger(v);
        return *this;
    }

    inline Decoder & Decoder::operator>>(u16 & v)
    {
        _streamSmallerUnsignedInteger(v);
        return *this;
    }

    inline Decoder & Decoder::operator>>(s8 & v)
    {
        _streamSmallerSignedInteger(v);
        return *this;
    }

    inline Decoder & Decoder::operator>>(u8 & v)
    {
        _streamSmallerUnsignedInteger(v);
        return *this;
    }

    inline Decoder & Decoder::operator>>(double & v)
    {
        if (!_preValueStreamCheck())
        {
            return *this;
        }

        positive = _tryConsumeSign() >= 0;

        if (_isDigit(*_pos) && _isFloater(_pos + 1))
        {
            _postValue(_consumeFloater(v), DecodeState::floater);
        }
        else if (_tryConsumeChars("inf"sv))
        {
            v = positive ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
            _postValue(DecodeState::floater);
        }
        else if (_tryConsumeChars("nan"sv))
        {
            v = std::numeric_limits<double>::quiet_NaN();
            _postValue(DecodeState::floater);
        }
        else
        {
            errorMessage = "Expected floater"sv;
            _state = DecodeState::error;
        }

        return *this;
    }

    inline Decoder & Decoder::operator>>(float & v)
    {
        double tmp;
        if (*this >> tmp)
        {
            v = float(tmp);
        }

        return *this;
    }

    inline Decoder & Decoder::operator>>(bool & v)
    {
        if (!_preValueStreamCheck())
        {
            return *this;
        }

        if (_tryConsumeChars("true"))
        {
            v = true;
            _postValue(DecodeState::boolean);
        }
        else if (_tryConsumeChars("false"))
        {
            v = false;
            _postValue(DecodeState::boolean);
        }
        else
        {
            errorMessage = "Expected boolean"sv;
            _state = DecodeState::error;
        }

        return *this;
    }

    inline Decoder & Decoder::operator>>(Date & v)
    {
        if (!_preValueStreamCheck())
        {
            return *this;
        }

        _postValue(_consumeChar('D') && _consumeDate(v), DecodeState::date);

        return *this;
    }

    inline Decoder & Decoder::operator>>(Time & v)
    {
        if (!_preValueStreamCheck())
        {
            return *this;
        }

        _postValue(_consumeChar('T') && _consumeTime(v), DecodeState::time);

        return *this;
    }

    inline Decoder & Decoder::operator>>(Datetime & v)
    {
        if (!_preValueStreamCheck())
        {
            return *this;
        }

        _postValue(_consumeChar('D') && _consumeDate(v.date) && _consumeChar('T') && _consumeTime(v.time) && _consumeTimezone(v.zone), DecodeState::datetime);

        return *this;
    }

    inline Decoder & Decoder::operator>>(Timepoint & v)
    {
        if (*this >> datetime)
        {
            v = datetime.toTimepoint();
        }

        return *this;
    }

    inline Decoder & Decoder::operator>>(nullptr_t)
    {
        if (!_preValueStreamCheck())
        {
            return *this;
        }

        _postValue(_consumeChars("null"sv), DecodeState::null);

        return *this;
    }

    inline void Decoder::_reset()
    {
        _state = DecodeState::error;
        _qcon = nullptr;
        _pos = nullptr;
        _stack = 0u;
        _depth = 0u;
        _cachedEnd = nullptr;
        _hadComma = false;
    }

    inline void Decoder::_skipSpace()
    {
        while (_isSpace(*_pos)) ++_pos;
    }

    inline void Decoder::_skipSpaceAndComments()
    {
        _skipSpace();

        // Skip comments and space
        while (*_pos == '#')
        {
            // Skip `#`
            ++_pos;

            // Skip rest of line
            while (*_pos && *_pos != '\n') ++_pos;

            _skipSpace();
        }
    }

    inline bool Decoder::_preValueStreamCheck()
    {
        // Preserve error state
        if (_state == DecodeState::error)
        {
            return false;
        }

        if (_depth)
        {
            const bool inObject{bool(_stack & 1u)};
            if (inObject)
            {
                // Ensure we have key if in object
                if (_state != DecodeState::key)
                {
                    errorMessage = "Expected key"sv;
                    _state = DecodeState::error;
                    return false;
                }
            }
            else
            {
                // Ensure there is a comma between array elements
                if (_state != DecodeState::array && !_hadComma)
                {
                    errorMessage = "Expected comma"sv;
                    _state = DecodeState::error;
                    return false;
                }
            }
        }
        else
        {
            // Ensure only one value at root level
            if (_state != DecodeState::ready)
            {
                errorMessage = "Root may only have a single value"sv;
                _state = DecodeState::error;
                return false;
            }
        }

        return true;
    }

    inline bool Decoder::_preKeyStreamCheck()
    {
        // Already know we're in object and don't already have key

        // Preserve error state
        if (_state == DecodeState::error)
        {
            return false;
        }

        // Ensure there is a comma between elements
        if (_state != DecodeState::object && !_hadComma)
        {
            errorMessage = "Expected comma"sv;
            _state = DecodeState::error;
            return false;
        }

        return true;
    }

    inline void Decoder::_postValue(const DecodeState newState)
    {
        _state = newState;

        _skipSpaceAndComments();

        if (_depth)
        {
            // Ingest comma
            _hadComma = _tryConsumeChar(',');
            if (_hadComma)
            {
                _skipSpaceAndComments();
            }
        }
        else
        {
            // Ensure there is nothing else at root level
            if (*_pos)
            {
                errorMessage = "Extraneous root content"sv;
                _state = DecodeState::error;
            }
        }
    }

    inline void Decoder::_postValue(const bool condition, const DecodeState newState)
    {
        if (condition)
        {
            _postValue(newState);
        }
        else
        {
            _state = DecodeState::error;
        }
    }

    inline int Decoder::_tryConsumeSign()
    {
        const int sign{(*_pos == '+') - (*_pos == '-')};

        if (sign)
        {
            ++_pos;
        }

        return sign;
    }

    inline bool Decoder::_tryConsumeChar(const char c)
    {
        if (*_pos == c)
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
        const char * const start{_pos};

        for (const char c : str)
        {
            if (*_pos != c)
            {
                _pos = start;
                return false;
            }

            ++_pos;
        }

        return true;
    }

    template <bool checkOverflow> inline bool Decoder::_tryConsumeBinaryDigit(u64 & dst)
    {
        const u64 b{u64(*_pos - '0')};

        if (b >= 2u)
        {
            return false;
        }

        // Check if would overflow
        if constexpr (checkOverflow)
        {
            if (dst & (u64(0b1u) << 63))
            {
                return false;
            }
        }

        dst = (dst << 1) | b;

        ++_pos;
        return true;
    }

    template <bool checkOverflow> inline bool Decoder::_tryConsumeOctalDigit(u64 & dst)
    {
        const u64 o{u64(*_pos - '0')};

        if (o >= 8u)
        {
            return false;
        }

        // Check if would overflow
        if constexpr (checkOverflow)
        {
            if (dst & (u64(0b111u) << 61))
            {
                return false;
            }
        }

        dst = (dst << 3) | o;

        ++_pos;
        return true;
    }

    template <bool checkOverflow> inline bool Decoder::_tryConsumeDecimalDigit(u64 & dst)
    {
        static constexpr u64 riskyVal{std::numeric_limits<u64>::max() / 10u};
        static constexpr u64 riskyDigit{std::numeric_limits<u64>::max() % 10u};

        const u64 d{u64(*_pos - '0')};

        if (d >= 10u)
        {
            return false;
        }

        // Check if would overflow
        if constexpr (checkOverflow)
        {
            if (dst > riskyVal || dst == riskyVal && d > riskyDigit)
            {
                return false;
            }
        }

        dst = dst * 10u + d;

        ++_pos;
        return true;
    }

    template <bool checkOverflow> inline bool Decoder::_tryConsumeHexDigit(u64 & dst)
    {
        static constexpr std::array<u8, 256u> hexTable{_createHexTable()};

        const u64 h{hexTable[u8(*_pos)]};

        if (h >= 16u)
        {
            return false;
        }

        // Check if would overflow
        if constexpr (checkOverflow)
        {
            if (dst & (u64(0b1111u) << 60))
            {
                return false;
            }
        }

        dst = (dst << 4) | h;

        ++_pos;
        return true;
    }

    // Assumed to not overflow
    inline bool Decoder::_tryConsumeDecimalDigits(unat digits, u64 & dst)
    {
        const char * const start{_pos};

        dst = 0u;

        for (; digits; --digits)
        {
            if (!_tryConsumeDecimalDigit<false>(dst))
            {
                _pos = start;
                return false;
            }
        }

        return true;
    }

    // Assumed to not overflow
    inline bool Decoder::_tryConsumeHexDigits(unat digits, u64 & dst)
    {
        const char * const start{_pos};

        dst = 0u;

        for (; digits; --digits)
        {
            if (!_tryConsumeHexDigit<false>(dst))
            {
                _pos = start;
                return false;
            }
        }

        return true;
    }

    inline bool Decoder::_consumeChar(const char c)
    {
        if (!_tryConsumeChar(c))
        {
            errorMessage.clear();
            std::format_to(std::back_inserter(errorMessage), "Expected `{}`"sv, c);
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeChars(const std::string_view str)
    {
        if (!_tryConsumeChars(str))
        {
            errorMessage.clear();
            std::format_to(std::back_inserter(errorMessage), "Expected `{}`"sv, str);
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeDecimalDigits(const unat digits, u64 & dst)
    {
        if (_tryConsumeDecimalDigits(digits, dst))
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

    inline bool Decoder::_consumeHexDigits(const unat digits, u64 & dst)
    {
        if (_tryConsumeHexDigits(digits, dst))
        {
            return true;
        }
        else
        {
            errorMessage.clear();
            std::format_to(std::back_inserter(errorMessage), "Expected {} hex digits"sv, digits);
            return false;
        }
    }

    inline bool Decoder::_consumeCodePoint(const unat digits, std::string & dst)
    {
        u64 v;
        if (!_consumeHexDigits(digits, v))
        {
            return false;
        }

        const u32 codepoint{u32(v)};

        // Convert codepoint to UTF-8
        // AAABBBBBBCCCCCCDDDDDD -> 11110AAA 10BBBBBB 10CCCCCC 10DDDDDD
        //      AAAABBBBBBCCCCCC -> 1110AAAA 10BBBBBB 10CCCCCC
        //           AAAAABBBBBB -> 110AAAAA 10BBBBBB
        //               AAAAAAA -> 0AAAAAAA
        if (codepoint >= (1u << 21))
        {
            errorMessage = "Codepoint too large"sv;
            return false;
        }
        else if (codepoint >= (1u << 16))
        {
            dst.push_back(char(u8((0b11110'000u | (codepoint >> 18)))));
            dst.push_back(char(u8((0b10'000000u | ((codepoint >> 12) & 0b111111u)))));
            dst.push_back(char(u8((0b10'000000u | ((codepoint >> 6) & 0b111111u)))));
            dst.push_back(char(u8((0b10'000000u | (codepoint & 0b111111u)))));
        }
        else if (codepoint >= (1u << 11))
        {
            dst.push_back(char(u8((0b1110'0000u | (codepoint >> 12)))));
            dst.push_back(char(u8((0b10'000000u | ((codepoint >> 6) & 0b111111u)))));
            dst.push_back(char(u8((0b10'000000u | (codepoint & 0b111111u)))));
        }
        else if (codepoint >= (1u << 7))
        {
            dst.push_back(char(u8((0b110'00000u | (codepoint >> 6)))));
            dst.push_back(char(u8((0b10'000000u | (codepoint & 0b111111u)))));
        }
        else
        {
            dst.push_back(char(u8(codepoint)));
        }

        return true;
    }

    inline bool Decoder::_consumeEscaped(std::string & dst)
    {
        char c{*_pos};
        ++_pos;

        switch (c)
        {
            case '0': c = '\0'; break;
            case 'a': c = '\a'; break;
            case 'b': c = '\b'; break;
            case 't': c = '\t'; break;
            case 'n': c = '\n'; break;
            case 'v': c = '\v'; break;
            case 'f': c = '\f'; break;
            case 'r': c = '\r'; break;
            case 'x': return _consumeCodePoint(2u, dst);
            case 'u': return _consumeCodePoint(4u, dst);
            case 'U': return _consumeCodePoint(8u, dst);
            case '"': break;
            case '/': break;
            case '\\': break;
            default:
            {
                --_pos;
                errorMessage = "Invalid escape sequence"sv;
                return false;
            }
        }

        dst.push_back(c);
        return true;
    }

    inline bool Decoder::_consumeString(std::string & dst)
    {
        // We already know we have `"`

        dst.clear();

        while (true)
        {
            char c{*_pos};
            if (c == '"')
            {
                ++_pos;

                _skipSpaceAndComments();

                if (*_pos == '"')
                {
                    ++_pos;
                }
                else
                {
                    return true;
                }
            }
            else if (c == '\\')
            {
                ++_pos;

                if (!_consumeEscaped(dst))
                {
                    return false;
                }
            }
            else if (_isControl(c))
            {
                errorMessage = "Invalid string content"sv;
                return false;
            }
            else
            {
                dst.push_back(c);
                ++_pos;
            }
        }
    }

    inline bool Decoder::_consumeKey(std::string & dst)
    {
        if (!_consumeChar('"'))
        {
            return false;
        }

        if (!_consumeString(dst))
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

    inline bool Decoder::_consumeBinaryInteger(u64 & dst)
    {
        const char * start{_pos};

        dst = 0u;

        while (_tryConsumeBinaryDigit<true>(dst));

        if (_pos == start)
        {
            errorMessage = "Missing binary digit"sv;
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeOctalInteger(u64 & dst)
    {
        const char * start{_pos};

        dst = 0u;

        while (_tryConsumeOctalDigit<true>(dst))

        if (_pos == start)
        {
            errorMessage = "Missing octal digit"sv;
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeDecimalInteger(u64 & dst)
    {
        const char * start{_pos};

        dst = 0u;

        while (_tryConsumeDecimalDigit<true>(dst));

        if (_pos == start)
        {
            errorMessage = "Missing decimal digit"sv;
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeHexInteger(u64 & dst)
    {
        const char * start{_pos};

        dst = 0u;

        while (_tryConsumeHexDigit<true>(dst));

        if (_pos == start)
        {
            errorMessage = "Missing hex digit"sv;
            return false;
        }

        return true;
    }

    inline bool Decoder::_consumeInteger(s64 & dst)
    {
        // Already know we have digit

        u64 v;

        // Check if hex/octal/binary
        if (*_pos == '0')
        {
            ++_pos;

            switch (*_pos)
            {
                case 'b':
                {
                    ++_pos;
                    if (!_consumeBinaryInteger(v))
                    {
                        return false;
                    }
                    break;
                }
                case 'o':
                {
                    ++_pos;
                    if (!_consumeOctalInteger(v))
                    {
                        return false;
                    }
                    break;
                }
                case 'x':
                {
                    ++_pos;
                    if (!_consumeHexInteger(v))
                    {
                        return false;
                    }
                    break;
                }
                default:
                {
                    --_pos;
                    if (!_consumeDecimalInteger(v))
                    {
                        return false;
                    }
                }
            }
        }
        else if (!_consumeDecimalInteger(v))
        {
            return false;
        }

        if (positive)
        {
            dst = s64(v);
        }
        else
        {
            // The integer is too large to fit in an `s64` when negative
            if (v > u64(std::numeric_limits<s64>::min()))
            {
                errorMessage = "Negative integer too large"sv;
                return false;
            }

            dst = -s64(v);
        }

        return true;
    }

    inline bool Decoder::_consumeFloater(double & dst)
    {
        // Determine end of QCON
        if (!_cachedEnd) [[unlikely]]
        {
            _cachedEnd = _pos + std::strlen(_pos);
        }

        const std::from_chars_result res{std::from_chars(_pos, _cachedEnd, dst)};

        // There was an issue parsing
        if (res.ec != std::errc{})
        {
            errorMessage = "Invalid floater"sv;
            return false;
        }

        if (!positive) dst = -dst;
        _pos = res.ptr;
        return true;
    }

    // `month` must be in range [1, 12]
    inline u8 _lastMonthDay(const u64 year, const u64 month)
    {
        static constexpr u8 monthDayCounts[16u]{31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u};

        if (month == 2u)
        {
            return 28u + std::chrono::year(int(year)).is_leap();
        }
        else
        {
            return monthDayCounts[month - 1u];
        }
    }

    // Utterly ignoring leap seconds with righteous conviction
    inline bool Decoder::_consumeDate(Date & dst)
    {
        // Already consumed `D`

        // Consume year
        u64 year;
        if (!_consumeDecimalDigits(4u, year))
        {
            return false;
        }

        if (!_consumeChar('-'))
        {
            return false;
        }

        // Consume month
        u64 month;
        if (!_consumeDecimalDigits(2u, month))
        {
            return false;
        }
        if (month < 1u || month > 12u)
        {
            errorMessage = "Invalid month"sv;
            return false;
        }

        if (!_consumeChar('-'))
        {
            return false;
        }

        // Consume day
        u64 day;
        if (!_consumeDecimalDigits(2u, day))
        {
            return false;
        }
        if (day < 1u || day > _lastMonthDay(year, month))
        {
            errorMessage = "Invalid day"sv;
            return false;
        }

        dst.year = u16(year);
        dst.month = u8(month);
        dst.day = u8(day);
        return true;
    }

    // Utterly ignoring leap seconds with righteous conviction
    inline bool Decoder::_consumeTime(Time & dst)
    {
        // Already consumed `T`

        // Consume hour
        u64 hour;
        if (!_consumeDecimalDigits(2u, hour))
        {
            return false;
        }
        if (hour >= 24u)
        {
            _pos -= 2;
            errorMessage = "Invalid hour"sv;
            return false;
        }

        if (!_consumeChar(':'))
        {
            return false;
        }

        // Consume minute
        u64 minute;
        if (!_consumeDecimalDigits(2u, minute))
        {
            return false;
        }
        if (minute >= 60u)
        {
            _pos -= 2;
            errorMessage = "Invalid minute"sv;
            return false;
        }

        if (!_consumeChar(':'))
        {
            return false;
        }

        // Consume second
        u64 second;
        if (!_consumeDecimalDigits(2u, second))
        {
            return false;
        }
        if (second >= 60u)
        {
            _pos -= 2;
            errorMessage = "Invalid second"sv;
            return false;
        }

        // Consume subsecond
        u64 subsecond{0u};
        if (_tryConsumeChar('.'))
        {
            const char * const start{_pos};
            if (!_consumeDecimalInteger(subsecond))
            {
                return false;
            }

            static constexpr u64 powersOf10[20u]{
                1u, 10u, 100u, 1'000u, 10'000u, 100'000u, 1'000'000u, 10'000'000u,
                100'000'000u, 1'000'000'000u, 10'000'000'000u, 100'000'000'000u, 1'000'000'000'000u,
                10'000'000'000'000u, 100'000'000'000'000u, 1'000'000'000'000'000u, 10'000'000'000'000'000u,
                100'000'000'000'000'000u, 1'000'000'000'000'000'000u, 10'000'000'000'000'000'000u};

            const unat digits{unat(_pos - start)};
            if (digits < 9u)
            {
                subsecond *= powersOf10[9u - digits];
            }
            else if (digits > 9u)
            {
                // Appropriate rounding
                const u64 divisor{powersOf10[digits - 9u]};
                subsecond = (subsecond + divisor / 2u) / divisor;
            }
        }

        dst.hour = u8(hour);
        dst.minute = u8(minute);
        dst.second = u8(second);
        dst.subsecond = u32(subsecond);
        return true;
    }

    inline bool Decoder::_consumeTimezone(Timezone & dst)
    {
        // GMT time
        if (_tryConsumeChar('Z'))
        {
            dst.format = utc;
            dst.offset = 0u;
        }
        // Has timezone offset
        else if (const int sign{_tryConsumeSign()}; sign)
        {
            u64 hour;
            if (!_consumeDecimalDigits(2u, hour))
            {
                return false;
            }
            if (hour > 23u)
            {
                _pos -= 2;
                errorMessage = "Invalid hour"sv;
                return false;
            }

            if (!_consumeChar(':'))
            {
                return false;
            }

            u64 minute;
            if (!_consumeDecimalDigits(2u, minute))
            {
                return false;
            }
            if (minute > 59u)
            {
                _pos -= 2;
                errorMessage = "Invalid minute"sv;
                return false;
            }

            dst.format = utcOffset;
            dst.offset = s16(hour * 60u + minute);
            if (sign < 0) dst.offset = -dst.offset;
        }
        // No timezone, local time
        else
        {
            dst.format = localTime;
            dst.offset = 0u;
        }

        return true;
    }

    inline void Decoder::_ingestStart(const Container container)
    {
        static constexpr u64 flags[2u]{0u, 1u};

        // Open brace/bracket already consumed

        if (_depth < 64u)
        {
            const bool isObject{container == object};
            _stack <<= 1;
            ++_depth;
            _stack |= u64(isObject);
            _state = isObject ? DecodeState::object : DecodeState::array;
            _skipSpaceAndComments();
        }
        else
        {
            errorMessage = "Exceeded max depth of 64"sv;
            _state = DecodeState::error;
        }
    }

    inline void Decoder::_ingestEnd()
    {
        // Already know we have close brace/bracket
        ++_pos;
        _stack >>= 1;
        --_depth;
        _postValue(DecodeState::end);
    }

    inline void Decoder::_ingestNumber()
    {
        // Already know we have one digit

        if (_isFloater(_pos + 1))
        {
            _postValue(_consumeFloater(floater), DecodeState::floater);
        }
        else
        {
            _postValue(_consumeInteger(integer), DecodeState::integer);
        }
    }

    inline void Decoder::_ingestValue()
    {
        positive = true;

        switch (*_pos)
        {
            case '\0':
            {
                errorMessage = "Hit end of QCON"sv;
                _state = DecodeState::error;
                return;
            }
            case '{':
            {
                ++_pos;
                _ingestStart(object);
                return;
            }
            case '[':
            {
                ++_pos;
                _ingestStart(array);
                return;
            }
            case '"':
            {
                ++_pos;
                _postValue(_consumeString(string), DecodeState::string);
                return;
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
                _ingestNumber();
                return;
            }
            case '+':
            {
                ++_pos;
                if (_isDigit(*_pos))
                {
                    _ingestNumber();
                    return;
                }
                else if (_tryConsumeChars("inf"sv))
                {
                    floater = std::numeric_limits<double>::infinity();
                    _postValue(DecodeState::floater);
                    return;
                }
                else if (_tryConsumeChars("nan"sv))
                {
                    floater = std::numeric_limits<double>::quiet_NaN();
                    _postValue(DecodeState::floater);
                    return;
                }
                --_pos;
                break;
            }
            case '-':
            {
                positive = false;
                ++_pos;
                if (_isDigit(*_pos))
                {
                    _ingestNumber();
                    return;
                }
                else if (_tryConsumeChars("inf"sv))
                {
                    floater = -std::numeric_limits<double>::infinity();
                    _postValue(DecodeState::floater);
                    return;
                }
                else if (_tryConsumeChars("nan"sv))
                {
                    floater = std::numeric_limits<double>::quiet_NaN();
                    _postValue(DecodeState::floater);
                    return;
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
                    _postValue(DecodeState::floater);
                    return;
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
                    _postValue(DecodeState::boolean);
                    return;
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
                    _postValue(DecodeState::boolean);
                    return;
                }
                --_pos;
                break;
            }
            case 'n':
            {
                ++_pos;
                if (_tryConsumeChars("ull"sv)) {
                    _postValue(DecodeState::null);
                    return;
                }
                else if (_tryConsumeChars("an"sv))
                {
                    floater = std::numeric_limits<double>::quiet_NaN();
                    _postValue(DecodeState::floater);
                    return;
                }
                --_pos;
                break;
            }
            case 'D':
            {
                ++_pos;
                _postValue(_consumeDate(date), DecodeState::date);

                if (*_pos == 'T')
                {
                    ++_pos;
                    _postValue(_consumeTime(time) && _consumeTimezone(datetime.zone), DecodeState::datetime);
                }

                return;
            }
            case 'T':
            {
                ++_pos;
                _postValue(_consumeTime(time), DecodeState::time);
                return;
            }
        }

        errorMessage = "Unknown value"sv;
        _state = DecodeState::error;
    }
}
