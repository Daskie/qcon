#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This standalone header provides a SAX encoder
/// See the README for more info and examples!
///

#include <bit>
#include <charconv>
#include <chrono>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <qcon-common.hpp>

namespace qcon
{
    enum class Density
    {
        multiline, /// Elements are put on new lines
        uniline,   /// Elements are put on one line separated by spaces
        nospace    /// No space is used whatsoever
    };
    using enum Density;

    enum class Container
    {
        end,
        object,
        array
    };
    using enum Container;

    enum class Base
    {
        decimal,
        binary,
        octal,
        hex
    };
    using enum Base;

    ///
    /// Instantiate this class to do the encoding
    ///
    class Encoder
    {
      public:

        ///
        /// Construct a new `Encoder` with the given options
        ///
        /// @param density the starting density for the QCON
        /// @param indentSpaces the number of spaces to insert per level of indentation
        ///
        Encoder(Density density = multiline, unat indentSpaces = 4u);

        Encoder(const Encoder &) = delete;

        ///
        /// Move constructor
        ///
        /// @param other is left in a valid but unspecified state
        /// @return this
        ///
        Encoder(Encoder && other);

        Encoder & operator=(const Encoder &) = delete;

        ///
        /// Move assignment operator
        ///
        /// @param other is left in a valid but unspecified state
        /// @return this
        ///
        Encoder & operator=(Encoder && other);

        ~Encoder() = default;

        ///
        /// The next container streamed will have the given density
        ///
        /// Density is always maximized, that is the encoded density of a given container will always be at least that
        ///   of its parent
        ///
        /// This flag is cleared afterwards
        ///
        Encoder & operator<<(Density v);

        ///
        /// Start a new object, array, or end current container
        ///
        Encoder & operator<<(Container v);

        ///
        /// Set the numeric base of the next integer streamed
        ///
        /// This flag is defaulted back to decimal afterwards
        ///
        Encoder & operator<<(Base base);

        ///
        /// Set the timezone format of the next datetime streamed
        ///
        /// This flag is defaulted back to `offset` afterwards
        ///
        Encoder & operator<<(TimezoneFormat timezoneFormat);

        ///
        /// Encode a value into the QCON
        ///
        /// @param v the value to encode
        /// @return this
        ///
        Encoder & operator<<(std::string_view v);
        Encoder & operator<<(const std::string & v);
        Encoder & operator<<(const char * v);
        Encoder & operator<<(char * v);
        Encoder & operator<<(char v);
        Encoder & operator<<(s64 v);
        Encoder & operator<<(s32 v);
        Encoder & operator<<(s16 v);
        Encoder & operator<<(s8 v);
        Encoder & operator<<(u64 v);
        Encoder & operator<<(u32 v);
        Encoder & operator<<(u16 v);
        Encoder & operator<<(u8 v);
        Encoder & operator<<(double v);
        Encoder & operator<<(float v);
        Encoder & operator<<(bool v);
        Encoder & operator<<(const Date & v);
        Encoder & operator<<(const Time & v);
        Encoder & operator<<(const Datetime & v);
        Encoder & operator<<(Timepoint v);
        Encoder & operator<<(std::nullptr_t);

        ///
        /// @return whether the encoding has been thusfar successful
        ///
        [[nodiscard]] bool status() const { return _expect != _Expect::error; }

        ///
        /// Return the encoder to a clean initial state
        ///
        void reset();

        ///
        /// Collapses the internal string stream into the encoded QCON string. This function resets the internal state
        /// of the encoder to a "clean slate" such that it can be safely reused
        ///
        /// @return the encoded QCON string
        ///
        [[nodiscard]] std::optional<std::string> finish();

        [[nodiscard]] Container container() const { return _container; }

        [[nodiscard]] Density density() const { return _density; }

      private:

        enum class _Expect
        {
            any,
            key,
            container,
            integer,
            timepoint,
            nothing,
            error
        };

        struct _ScopeInfo
        {
            Container container;
            Density density;
        };

        Density _baseDensity;
        unat _indentSpaces;

        std::string _str;
        std::vector<_ScopeInfo> _scopeInfos;
        Container _container;
        Density _density;
        unat _indentation;
        unat _lineStartI;
        Density _nextDensity;
        Base _nextBase;
        TimezoneFormat _nextTimezoneFormat;
        _Expect _expect;

        void _start(Container container);

        void _end();

        template <typename T> void _val(T v);

        void _key(std::string_view key);

        void _putSpace();

        [[nodiscard]] bool _encode(std::string_view v);
        [[nodiscard]] bool _encode(s64 v);
        [[nodiscard]] bool _encode(u64 v);
        void _encodeBinary(u64 v);
        void _encodeOctal(u64 v);
        void _encodeDecimal(u64 v);
        void _encodeHex(u64 v);
        [[nodiscard]] bool _encode(double v);
        [[nodiscard]] bool _encode(bool v);
        [[nodiscard]] bool _encode(const Date & v);
        [[nodiscard]] bool _encode(const Time & v);
        [[nodiscard]] bool _encode(const Datetime & v);
        [[nodiscard]] bool _encode(std::nullptr_t);
    };
}

///
/// Specialize `qcon::Encoder & operator<<(qcon::Encoder &, const Custom &)` to enable encoding for `Custom` type
///
/// Example:
///     qcon::Encoder & operator<<(qcon::Encoder & encoder, const std::pair<int, int> & v)
///     {
///         return encoder << array << v.first << v.second << end;
///     }
///

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
    constexpr char _hexEncodeTable[16u]{
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'};

    inline Encoder::Encoder(const Density density, const unat indentSpaces) :
        _baseDensity{density},
        _indentSpaces{indentSpaces}
    {
        reset();
    }

    inline Encoder::Encoder(Encoder && other) :
        _baseDensity{other._baseDensity},
        _indentSpaces{other._indentSpaces},

        _str{std::move(other._str)},
        _scopeInfos{std::move(other._scopeInfos)},
        _container{other._container},
        _density{other._density},
        _indentation{other._indentation},
        _lineStartI{other._lineStartI},
        _nextDensity{other._nextDensity},
        _nextBase{other._nextBase},
        _nextTimezoneFormat{other._nextTimezoneFormat},
        _expect{other._expect}
    {
        other.reset();
    }

    inline Encoder & Encoder::operator=(Encoder && other)
    {
        _baseDensity = other._baseDensity;
        _indentSpaces = other._indentSpaces;

        _str = std::move(other._str);
        _scopeInfos = std::move(other._scopeInfos);
        _container = other._container;
        _density = other._density;
        _indentation = other._indentation;
        _lineStartI = other._lineStartI;
        _nextDensity = other._nextDensity;
        _nextBase = other._nextBase;
        _nextTimezoneFormat = other._nextTimezoneFormat;
        _expect = other._expect;

        other.reset();

        return *this;
    }

    inline Encoder & Encoder::operator<<(const Density density)
    {
        if (_expect != _Expect::any && _expect != _Expect::container)
        {
            _expect = _Expect::error;
            return *this;
        }

        _nextDensity = std::max(_density, density);
        _expect = _Expect::container;
        return *this;
    }

    inline Encoder & Encoder::operator<<(const Container container)
    {
        if (container == end)
        {
            _end();
        }
        else
        {
            _start(container);
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const Base base)
    {
        if (_expect != _Expect::any && _expect != _Expect::integer)
        {
            _expect = _Expect::error;
            return *this;
        }

        _nextBase = base;
        _expect = _Expect::integer;
        return *this;
    }

    inline Encoder & Encoder::operator<<(const TimezoneFormat timezoneFormat)
    {
        if (_expect != _Expect::any && _expect != _Expect::timepoint)
        {
            _expect = _Expect::error;
            return *this;
        }

        _nextTimezoneFormat = timezoneFormat;
        _expect = _Expect::timepoint;
        return *this;
    }

    inline Encoder & Encoder::operator<<(const std::string_view v)
    {
        if (_expect == _Expect::any)
        {
            _val(v);
        }
        else if (_expect == _Expect::key)
        {
            _key(v);
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const std::string & v)
    {
        return operator<<(std::string_view{v});
    }

    inline Encoder & Encoder::operator<<(const char * const v)
    {
        return operator<<(std::string_view{v});
    }

    inline Encoder & Encoder::operator<<(char * const v)
    {
        return operator<<(std::string_view{v});
    }

    inline Encoder & Encoder::operator<<(const char v)
    {
        return operator<<(std::string_view{&v, 1u});
    }

    inline Encoder & Encoder::operator<<(const s64 v)
    {
        if (_expect == _Expect::any || _expect == _Expect::integer)
        {
            _val(v);
            _nextBase = decimal;
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const s32 v)
    {
        return operator<<(s64(v));
    }

    inline Encoder & Encoder::operator<<(const s16 v)
    {
        return operator<<(s64(v));
    }

    inline Encoder & Encoder::operator<<(const s8 v)
    {
        return operator<<(s64(v));
    }

    inline Encoder & Encoder::operator<<(const u64 v)
    {
        if (_expect == _Expect::any || _expect == _Expect::integer)
        {
            _val(v);
            _nextBase = decimal;
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const u32 v)
    {
        return operator<<(u64(v));
    }

    inline Encoder & Encoder::operator<<(const u16 v)
    {
        return operator<<(u64(v));
    }

    inline Encoder & Encoder::operator<<(const u8 v)
    {
        return operator<<(u64(v));
    }

    inline Encoder & Encoder::operator<<(const double v)
    {
        if (_expect == _Expect::any)
        {
            _val(v);
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const float v)
    {
        return operator<<(double(v));
    }

    inline Encoder & Encoder::operator<<(const bool v)
    {
        if (_expect == _Expect::any)
        {
            _val(v);
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const Date & v)
    {
        if (_expect == _Expect::any)
        {
            _val(v);
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const Time & v)
    {
        if (_expect == _Expect::any)
        {
            _val(v);
            _nextTimezoneFormat = utcOffset;
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const Datetime & v)
    {
        if (_expect == _Expect::any)
        {
            _val(v);
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const Timepoint v)
    {
        if (_expect == _Expect::any || _expect == _Expect::timepoint)
        {
            Datetime datetime;
            if (datetime.fromTimepoint(v, _nextTimezoneFormat))
            {
                _val(datetime);
                _nextTimezoneFormat = utcOffset;
            }
            else
            {
                _expect = _Expect::error;
            }
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline Encoder & Encoder::operator<<(const std::nullptr_t)
    {
        if (_expect == _Expect::any)
        {
            _val(nullptr);
        }
        else
        {
            _expect = _Expect::error;
        }

        return *this;
    }

    inline void Encoder::reset()
    {
        _str.clear();
        _scopeInfos.clear();
        _container = end;
        _density = _baseDensity;
        _indentation = 0u;
        _lineStartI = 0u;
        _nextDensity = _density;
        _nextBase = decimal;
        _nextTimezoneFormat = utcOffset;
        _expect = _Expect::any;
    }

    inline std::optional<std::string> Encoder::finish()
    {
        // QCON is not yet complete
        if (_expect != _Expect::nothing)
        {
            reset();
            return {};
        }

        // Remove value's trailing comma
        if (_str.back() == ',')
        {
            _str.pop_back();
        }

        const std::optional<std::string> result{std::move(_str)};
        reset();
        return result;
    }

    inline void Encoder::_start(const Container container)
    {
        if (_expect != _Expect::any && _expect != _Expect::container)
        {
            _expect = _Expect::error;
            return;
        }

        // Only put space if not at start of document or not after key
        if (!_str.empty() && _container != object)
        {
            _putSpace();
        }

        _str += container == object ? '{' : '[';

        _scopeInfos.push_back(_ScopeInfo{_container, _density});
        _container = container;
        _density = std::max(_density, _nextDensity);
        _nextDensity = _density;
        _indentation += _indentSpaces;
        _expect = _container == object ? _Expect::key : _Expect::any;
    }

    inline void Encoder::_end()
    {
        if (_expect != _Expect::key && !(_container == array && _expect == _Expect::any))
        {
            _expect = _Expect::error;
            return;
        }

        _indentation -= _indentSpaces;
        const bool empty{_str.back() == (_container == object ? '{' : '[')};
        if (!empty)
        {
            // Remove value's trailing comma
            _str.pop_back();
            _putSpace();
        }
        _str += (_container == object ? "},"sv : "],"sv);
        _container = _scopeInfos.back().container;
        _density = _scopeInfos.back().density;
        _nextDensity = _density;
        _scopeInfos.pop_back();
        switch (_container)
        {
            case end: _expect = _Expect::nothing; break;
            case object: _expect = _Expect::key; break;
            case array : _expect = _Expect::any; break;
        }
    }

    template <typename T>
    inline void Encoder::_val(const T v)
    {
        // Only put space if not at start of document or not after key
        if (!_str.empty() && _container != object)
        {
            _putSpace();
        }

        if (!_encode(v))
        {
            _expect = _Expect::error;
            return;
        }

        _str += ',';

        switch (_container)
        {
            case end: _expect = _Expect::nothing; break;
            case object: _expect = _Expect::key; break;
            case array: _expect = _Expect::any; break;
        }
    }

    inline void Encoder::_key(const std::string_view key)
    {
        _putSpace();

        if (!_encode(key))
        {
            _expect = _Expect::error;
            return;
        }

        _str += ':';

        if (_density < nospace) _str += ' ';

        _expect = _Expect::any;
    }

    inline void Encoder::_putSpace()
    {
        switch (_density)
        {
            case multiline:
            {
                _str += '\n';
                _lineStartI = _str.size();
                _str.append(_indentation, ' ');
                break;
            }
            case uniline:
            {
                _str += ' ';
                break;
            }
            case nospace:
            {
                break;
            }
        }
    }

    inline bool Encoder::_encode(const std::string_view v)
    {
        struct ControlString
        {
            alignas(4u) char chars[4u];

            consteval ControlString(const char c)
            {
                chars[0] = '\\';
                if (_isControl(c))
                {
                    chars[1] = 'x';
                    chars[2] = _hexEncodeTable[u8(c) / 16u];
                    chars[3] = _hexEncodeTable[u8(c) % 16u];
                }
                else
                {
                    chars[1] = c;
                    chars[2] = '\0';
                    chars[3] = '\0';
                }
            }

            unat length() const { return chars[2] ? 4u : 2u; };
        };

        static constexpr ControlString controlStrings[32u]{
            '0',
            1, 2, 3, 4, 5, 6,
            'a', 'b', 't', 'n', 'v', 'f', 'r',
            14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

        const unat extraIndentation{_str.size() - _lineStartI - _indentation};

        _str += '"';

        for (unat i{0u}, n{v.size()}; i < n; ++i)
        {
            const char c{v[i]};

            if (_isControl(c))
            {
                // Split strings on newlines in multiline density
                if (c == '\n' && _density <= multiline && i < n - 1u)
                {
                    _str += "\\n\""sv;
                    _putSpace();
                    _str.append(extraIndentation, ' ');
                    _str += '"';
                }
                else
                {
                    const ControlString controlStr{controlStrings[c]};
                    _str.append(controlStr.chars, controlStr.length());
                }
            }
            else
            {
                if (c == '"' || c == '\\') _str += '\\';
                _str += c;
            }
        }

        _str += '"';

        return true;
    }

    inline bool Encoder::_encode(s64 v)
    {
        if (v < 0)
        {
            _str += '-';
            v = -v;
        }

        return _encode(u64(v));
    }

    inline bool Encoder::_encode(const u64 v)
    {
        switch (_nextBase)
        {
            case decimal: _encodeDecimal(v); break;
            case binary: _encodeBinary(v); break;
            case octal: _encodeOctal(v); break;
            case hex: _encodeHex(v); break;
        }

        return true;
    }

    inline void Encoder::_encodeBinary(u64 v)
    {
        // Digits reversed due to endianess
        static constexpr u32 binaryTable[16u]{
            '0000', '1000', '0100', '1100',
            '0010', '1010', '0110', '1110',
            '0001', '1001', '0101', '1101',
            '0011', '1011', '0111', '1111'};

        static thread_local u32 chunkBuffer[16u];
        static thread_local char * const charBuffer{reinterpret_cast<char *>(chunkBuffer)};

        const unat leadZeroCount{unat(std::countl_zero(v))};

        // Process four bits at a time
        u32 * dst{chunkBuffer + 16};
        do
        {
            *--dst = binaryTable[v & 0b1111u];
            v >>= 4;
        } while (v);

        _str += "0b"sv;
        _str.append(charBuffer + std::min(leadZeroCount, unat(63u)), charBuffer + 64);
    }

    inline void Encoder::_encodeOctal(u64 v)
    {
        static thread_local char buffer[22u];

        char * const bufferEnd{buffer + sizeof(buffer)};
        char * dst{bufferEnd};

        do
        {
            *--dst = char('0' + (v & 0b111u));
            v >>= 3;
        } while (v);

        _str += "0o"sv;
        _str.append(dst, bufferEnd);
    }

    inline void Encoder::_encodeDecimal(u64 v)
    {
        static thread_local char buffer[20u];

        char * const bufferEnd{buffer + sizeof(buffer)};
        char * dst{bufferEnd};

        do
        {
            // Encourage compiler to combine into one instruction
            const u64 remainder{v % 10};
            const u64 quotient{v / 10};
            *--dst = char('0' + remainder);
            v = quotient;
        } while (v);

        _str.append(dst, bufferEnd);
    }

    inline void Encoder::_encodeHex(u64 v)
    {
        static thread_local char buffer[16u];

        char * const bufferEnd{buffer + sizeof(buffer)};
        char * dst{bufferEnd};

        do
        {
            *--dst = _hexEncodeTable[v & 0b1111u];
            v >>= 4;
        } while (v);

        _str += "0x"sv;
        _str.append(dst, bufferEnd);
    }

    inline bool Encoder::_encode(const double v)
    {
        static thread_local char buffer[24u];

        // Ensure all NaNs are encoded the same
        if (v != v)
        {
            _str.append("nan"sv);
            return true;
        }

        const std::to_chars_result res{std::to_chars(buffer, buffer + sizeof(buffer), v)};
        const unat length{unat(res.ptr - buffer)};
        _str.append(buffer, length);

        // Add trailing `.0` if necessary
        if (_isDigit(buffer[length - 1u]))
        {
            bool needsZero{true};
            for (unat i{0u}; i < length; ++i)
            {
                if (buffer[i] == '.' || buffer[i] == 'e')
                {
                    needsZero = false;
                    break;
                }
            }
            if (needsZero)
            {
                _str.append(".0"sv);
            }
        }

        return true;
    }

    inline bool Encoder::_encode(const bool v)
    {
        _str += v ? "true"sv : "false"sv;

        return true;
    }

    inline bool Encoder::_encode(const Date & v)
    {
        static thread_local char buffer[]{"DYYYY-MM-DD"};

        // `D` already in buffer

        // Encode year
        if (v.year > 9999u)
        {
            return false;
        }
        unat year{v.year};
        buffer[4] = char('0' + year % 10u); year /= 10u;
        buffer[3] = char('0' + year % 10u); year /= 10u;
        buffer[2] = char('0' + year % 10u); year /= 10u;
        buffer[1] = char('0' + year);

        // `-` already in buffer

        // Encode month
        if (v.month == 0u || v.month > 12u)
        {
            return false;
        }
        buffer[6] = char('0' + v.month / 10u);
        buffer[7] = char('0' + v.month % 10u);

        // `-` already in buffer

        // Encode day
        if (v.day == 0u || v.day > 31u)
        {
            return false;
        }
        buffer[ 9] = char('0' + v.day / 10u);
        buffer[10] = char('0' + v.day % 10u);

        _str.append(buffer, 11u);
        return true;
    }

    inline bool Encoder::_encode(const Time & v)
    {
        static thread_local char buffer[]{"Thh:mm:ss.fffffffff+hh:mm"};

        // `T` already in buffer

        // Encode hours
        if (v.hour > 23u)
        {
            return false;
        }
        buffer[1] = char('0' + v.hour / 10u);
        buffer[2] = char('0' + v.hour % 10u);

        // `:` already in buffer

        // Encode minutes
        if (v.minute > 59u)
        {
            return false;
        }
        buffer[4] = char('0' + v.minute / 10u);
        buffer[5] = char('0' + v.minute % 10u);

        // `:` already in buffer

        // Encode seconds
        if (v.second > 59u)
        {
            return false;
        }
        buffer[7] = char('0' + v.second / 10u);
        buffer[8] = char('0' + v.second % 10u);

        char * bufferEnd{buffer + 9};

        // Encode subseconds
        if (v.subsecond)
        {
            *bufferEnd = '.';

            // Temporarily point to the last element instead of one-past for simplicity
            bufferEnd += 9u;

            unat nanoseconds{v.subsecond};
            if (nanoseconds > 999'999'999u)
            {
                return false;
            }
            for (int i{0}; i < 8; ++i)
            {
                *bufferEnd = char('0' + nanoseconds % 10u);
                nanoseconds /= 10u;
                --bufferEnd;
            }
            *bufferEnd = char('0' + nanoseconds);

            bufferEnd += 8;

            // Drop trialing zeroes
            while (*bufferEnd == '0') --bufferEnd;

            // Increment to point to one-past the last element
            ++bufferEnd;
        }

        // Encode timezone
        switch (v.zone.format)
        {
            case utcOffset:
            {
                unat offset;
                if (v.zone.offset >= 0)
                {
                    bufferEnd[0] = '+';
                    offset = unat(v.zone.offset);
                }
                else
                {
                    bufferEnd[0] = '-';
                    offset = unat(-v.zone.offset);
                }

                if (offset >= 100u * 60u)
                {
                    return false;
                }

                bufferEnd[5] = char('0' + offset % 10u); offset /= 10u;
                bufferEnd[4] = char('0' + offset % 6u); offset /= 6u;
                bufferEnd[3] = ':';
                bufferEnd[2] = char('0' + offset % 10u); offset /= 10;
                bufferEnd[1] = char('0' + offset);

                bufferEnd += 6;
                break;
            }
            case utc:
            {
                *bufferEnd = 'Z';
                ++bufferEnd;
                break;
            }
            case localTime:
            {
                break;
            }
        }

        _str.append(buffer, bufferEnd);
        return true;
    }

    // Utterly ignoring leap seconds with righteous conviction
    inline bool Encoder::_encode(const Datetime & v)
    {
        return _encode(v.date) && _encode(v.time);
    }

    inline bool Encoder::_encode(std::nullptr_t)
    {
        _str += "null"sv;

        return true;
    }
}
