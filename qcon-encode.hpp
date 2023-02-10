#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This standalone header provides a SAX encoder
/// See the README for more info and examples!
///

#include <cctype>
#include <cstddef>

#include <charconv>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace qcon
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    using uchar = unsigned char;

    ///
    /// Namespace provided to allow the user to `using namespace qcon::tokens` to avoid the verbosity of fully
    /// qualifying the tokens namespace
    ///
    inline namespace tokens
    {
        ///
        /// Stream to the encoder to specify the density of the next container
        ///
        enum class Density : int8_t
        {
            multiline, /// Elements are put on new lines
            uniline,   /// Elements are put on one line separated by spaces
            nospace    /// No space is used whatsoever
        };

        using enum Density;

        ///
        /// Simple enum representing a QCON container type
        ///
        // TODO: These shouldn't need to be int8_t?
        enum class Container : int8_t
        {
            end,
            object,
            array
        };

        using enum Container;

        ///
        /// Stream to the encoder to specify the base of the next integer
        ///
        enum class Base
        {
            decimal,
            binary,
            octal,
            hex
        };

        using enum Base;
    }

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
        Encoder(Density density = multiline, size_t indentSpaces = 4u);

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
        /// This flag is cleared after ANY value is streamed
        ///
        Encoder & operator<<(Density v);

        ///
        /// Start a new object, array, or end current container
        ///
        Encoder & operator<<(Container v);

        ///
        /// Set the numeric base of the next integer streamed
        ///
        /// This flag is defaulted back to decimal after ANY value is streamed
        ///
        Encoder & operator<<(Base base);

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
        Encoder & operator<<(int64_t v);
        Encoder & operator<<(int32_t v);
        Encoder & operator<<(int16_t v);
        Encoder & operator<<(int8_t v);
        Encoder & operator<<(uint64_t v);
        Encoder & operator<<(uint32_t v);
        Encoder & operator<<(uint16_t v);
        Encoder & operator<<(uint8_t v);
        Encoder & operator<<(double v);
        Encoder & operator<<(float v);
        Encoder & operator<<(bool v);
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

        ///
        /// @return the current container
        ///
        [[nodiscard]] Container container() const { return _container; }

        ///
        /// @return the current density
        ///
        [[nodiscard]] Density density() const { return _density; }

        ///
        /// @return the current base
        ///
        [[nodiscard]] Base base() const { return _nextBase; }

      private:

        enum class _Expect
        {
            any,
            key,
            container,
            integer,
            nothing,
            error
        };

        // Using deltas allows us to start with an empty scope vector without needing a bunch of special root-case logic
        struct _ScopeDelta
        {
            int8_t containerDelta;
            int8_t densityDelta;
        };

        Density _baseDensity;
        size_t _indentSpaces;

        std::string _str;
        std::vector<_ScopeDelta> _scopeDeltas;
        Container _container;
        Density _density;
        size_t _indentation;
        Density _nextDensity;
        Base _nextBase;
        _Expect _expect;

        void _start(Container container);

        void _end();

        template <typename T> void _val(T v);

        void _key(std::string_view key);

        void _putSpace();

        void _encode(std::string_view v);
        void _encode(int64_t v);
        void _encode(uint64_t v);
        void _encodeDecimal(uint64_t v);
        void _encodeBinary(uint64_t v);
        void _encodeOctal(uint64_t v);
        void _encodeHex(uint64_t v);
        void _encode(double v);
        void _encode(bool v);
        void _encode(std::nullptr_t);
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
    inline Encoder::Encoder(const Density density, const size_t indentSpaces) :
        _baseDensity{density},
        _indentSpaces{indentSpaces}
    {
        reset();
    }

    inline Encoder::Encoder(Encoder && other) :
        _baseDensity{other._baseDensity},
        _indentSpaces{other._indentSpaces},

        _str{std::move(other._str)},
        _scopeDeltas{std::move(other._scopeDeltas)},
        _container{other._container},
        _density{other._density},
        _indentation{other._indentation},
        _nextDensity{other._nextDensity},
        _nextBase{other._nextBase},
        _expect{other._expect}
    {
        other.reset();
    }

    inline Encoder & Encoder::operator=(Encoder && other)
    {
        _baseDensity = other._baseDensity;
        _indentSpaces = other._indentSpaces;

        _str = std::move(other._str);
        _scopeDeltas = std::move(other._scopeDeltas);
        _container = other._container;
        _density = other._density;
        _indentation = other._indentation;
        _nextDensity = other._nextDensity;
        _nextBase = other._nextBase;
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

        _nextDensity = density;
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

    inline Encoder & Encoder::operator<<(const std::string_view v)
    {
        switch (_expect)
        {
            case _Expect::any: _val(v); break;
            case _Expect::key: _key(v); break;
            default: _expect = _Expect::error;
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

    inline Encoder & Encoder::operator<<(const int64_t v)
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

    inline Encoder & Encoder::operator<<(const int32_t v)
    {
        return operator<<(int64_t(v));
    }

    inline Encoder & Encoder::operator<<(const int16_t v)
    {
        return operator<<(int64_t(v));
    }

    inline Encoder & Encoder::operator<<(const int8_t v)
    {
        return operator<<(int64_t(v));
    }

    inline Encoder & Encoder::operator<<(const uint64_t v)
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

    inline Encoder & Encoder::operator<<(const uint32_t v)
    {
        return operator<<(uint64_t(v));
    }

    inline Encoder & Encoder::operator<<(const uint16_t v)
    {
        return operator<<(uint64_t(v));
    }

    inline Encoder & Encoder::operator<<(const uint8_t v)
    {
        return operator<<(uint64_t(v));
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
        _scopeDeltas.clear();
        _container = end;
        _density = _baseDensity;
        _indentation = 0u;
        _nextDensity = multiline;
        _nextBase = decimal;
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

        const Density newDensity{_nextDensity > _density ? _nextDensity : _density};
        const int8_t containerDelta{int8_t(int8_t(container) - int8_t(_container))};
        const int8_t densityDelta{int8_t(int8_t(newDensity) - int8_t(_density))};
        _scopeDeltas.push_back(_ScopeDelta{containerDelta, densityDelta});
        _container = container;
        _density = newDensity;
        _nextDensity = multiline;
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
        _container = Container(int8_t(_container) - _scopeDeltas.back().containerDelta);
        _density = Density(int8_t(_density) - _scopeDeltas.back().densityDelta);
        _scopeDeltas.pop_back();
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

        _encode(v);
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
        _encode(key);
        _str += ':';
        if (_density < nospace) _str += ' ';
        _expect = _Expect::any;
    }

    inline void Encoder::_putSpace()
    {
        switch (_density)
        {
            case multiline: _str += '\n'; _str.append(_indentation, ' '); break;
            case uniline: _str += ' '; break;
            case nospace: break;
        }
    }

    inline void Encoder::_encode(const std::string_view v)
    {
        static constexpr char hexChars[16u]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        _str += '"';

        for (const char c : v)
        {
            if (std::isprint(uchar(c)))
            {
                if (c == '"' || c == '\\') _str += '\\';
                _str += c;
            }
            else
            {
                switch (c)
                {
                    case '\0': _str += R"(\0)"; break;
                    case '\b': _str += R"(\b)"; break;
                    case '\t': _str += R"(\t)"; break;
                    case '\n': _str += R"(\n)"; break;
                    case '\v': _str += R"(\v)"; break;
                    case '\f': _str += R"(\f)"; break;
                    case '\r': _str += R"(\r)"; break;
                    default:
                        _str += "\\x"sv;
                        _str += hexChars[(uchar(c) >> 4) & 0xF];
                        _str += hexChars[uchar(c) & 0xF];
                }
            }
        }

        _str += '"';
    }

    inline void Encoder::_encode(int64_t v)
    {
        if (v < 0)
        {
            _str += '-';
            v = -v;
        }

        _encode(uint64_t(v));
    }

    inline void Encoder::_encode(const uint64_t v)
    {
        switch (_nextBase)
        {
            case decimal: _encodeDecimal(v); return;
            case binary: _encodeBinary(v); return;
            case octal: _encodeOctal(v); return;
            case hex: _encodeHex(v); return;
        }
    }

    inline void Encoder::_encodeDecimal(const uint64_t v)
    {
        static thread_local char buffer[24u];

        const std::to_chars_result res{std::to_chars(buffer, buffer + sizeof(buffer), v)};
        _str.append(buffer, size_t(res.ptr - buffer));
    }

    inline void Encoder::_encodeBinary(const uint64_t v)
    {
        static thread_local char buffer[66u]{'0', 'b'};

        const std::to_chars_result res{std::to_chars(buffer + 2, buffer + sizeof(buffer), v, 2)};
        _str.append(buffer, size_t(res.ptr - buffer));
    }

    inline void Encoder::_encodeOctal(const uint64_t v)
    {
        static thread_local char buffer[26u]{'0', 'o'};

        const std::to_chars_result res{std::to_chars(buffer + 2, buffer + sizeof(buffer), v, 8)};
        _str.append(buffer, size_t(res.ptr - buffer));
    }

    inline void Encoder::_encodeHex(uint64_t v)
    {
        // We're hand rolling this because `std::to_chars` doesn't support uppercase hex
        static constexpr char hexTable[16u]{
            '0', '1', '2', '3',
            '4', '5', '6', '7',
            '8', '9', 'A', 'B',
            'C', 'D', 'E', 'F'};

        static thread_local char buffer[18u];

        size_t bufferI{sizeof(buffer)};

        if (v)
        {
            do
            {
                buffer[--bufferI] = hexTable[v & 0xFu];
                v >>= 4;
            } while (v);
        }
        else
        {
            buffer[--bufferI] = '0';
        }

        buffer[--bufferI] = 'x';
        buffer[--bufferI] = '0';

        _str.append(buffer + bufferI, sizeof(buffer) - bufferI);
    }

    inline void Encoder::_encode(const double v)
    {
        static thread_local char buffer[24u];

        const std::to_chars_result res{std::to_chars(buffer, buffer + sizeof(buffer), v)};
        const size_t length{size_t(res.ptr - buffer)};
        _str.append(buffer, length);

        // Add trailing `.0` if necessary
        if (std::isdigit(buffer[length - 1u]))
        {
            bool needsZero{true};
            for (size_t i{0u}; i < length; ++i)
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
    }

    inline void Encoder::_encode(const bool v)
    {
        _str += v ? "true"sv : "false"sv;
    }

    inline void Encoder::_encode(std::nullptr_t)
    {
        _str += "null"sv;
    }
}
