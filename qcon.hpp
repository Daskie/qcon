#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This header provides a DOM encoder and decoder
/// Uses `qcon-encode.hpp` to do the encoding and `qcon-decode.hpp` to do the decoding
/// See the README for more info and examples!
///

#include <concepts>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <qcon-decode.hpp>
#include <qcon-encode.hpp>

namespace qcon
{
    ///
    /// The type of the QCON value
    ///
    enum class Type
    {
        null,
        object,
        array,
        string,
        integer,
        floater,
        boolean,
        datetime
    };

    // Forward declarations
    class Value;

    ///
    /// Convenience type alias
    /// The internal representation for objects is `std::map<std::string, qcon::value>`
    ///
    using Object = std::map<std::string, Value>;

    ///
    /// Convenience type alias
    /// The internal representation for arrays is `std::vector<qcon::Value>`
    ///
    using Array = std::vector<Value>;

    ///
    /// Represents one QCON value, which can be an object, array, string, number, boolean, or null
    ///
    class Value
    {
      public:

        ///
        /// Specialization of the encoder's `operator<<` for `Value`
        /// @param encoder the encoder
        /// @param val the QCON value to encode
        /// @return `encoder`
        ///
        friend Encoder & operator<<(Encoder & encoder, const Value & val);

        ///
        /// @param val the value whith which to be constructed
        ///
        Value(std::nullptr_t = nullptr);
        Value(Object && val);
        Value(Array && val);
        Value(std::string && val);
        Value(std::string_view val);
        Value(const char * val);
        Value(char * val);
        Value(char val);
        Value(s64 val);
        Value(s32 val);
        Value(s16 val);
        Value(s8 val);
        Value(u64 val);
        Value(u32 val);
        Value(u16 val);
        Value(u8 val);
        Value(double val);
        Value(float val);
        Value(bool val);
        Value(Datetime val);

        Value(const Value &) = delete;
        Value(Value && other);

        ///
        /// Assigns a new value to the qcon value
        /// @param val
        /// @return this
        ///
        Value & operator=(Object && val);
        Value & operator=(Array && val);
        Value & operator=(std::string && val);
        Value & operator=(std::string_view val);
        Value & operator=(const char * val);
        Value & operator=(char val);
        Value & operator=(s64 val);
        Value & operator=(s32 val);
        Value & operator=(s16 val);
        Value & operator=(s8 val);
        Value & operator=(u64 val);
        Value & operator=(u32 val);
        Value & operator=(u16 val);
        Value & operator=(u8 val);
        Value & operator=(double val);
        Value & operator=(float val);
        Value & operator=(bool val);
        Value & operator=(nullptr_t);
        Value & operator=(Datetime val);

        Value & operator=(const Value &) = delete;
        Value & operator=(Value && other);

        ~Value();

        ///
        /// @return the type of the value
        ///
        Type type() const { return _type; }

        ///
        /// @return this value as an object if it is an object, otherwise null
        ///
        Object * object();
        const Object * object() const;

        ///
        /// @return this value as an array if it is an array, otherwise null
        ///
        Array * array();
        const Array * array() const;

        ///
        /// @return this value as a string if it is a string, otherwise null
        ///
        std::string * string();
        const std::string * string() const;

        ///
        /// @return this value as an integer if it is an integer, otherwise null
        ///
        s64 * integer();
        const s64 * integer() const;

        ///
        /// @return this value as a floater if it is a floater, otherwise null
        ///
        double * floater();
        const double * floater() const;

        ///
        /// @return this value as a boolean if it is a boolean, otherwise null
        ///
        bool * boolean();
        const bool * boolean() const;

        ///
        /// @return this value as a null if it is a null, otherwise null
        ///
        nullptr_t * null();
        const nullptr_t * null() const;

        ///
        /// @return this value as a datetime if it is a datetime, otherwise null
        ///
        Datetime * datetime();
        const Datetime * datetime() const;

        ///
        /// @return whether the number was positive; useful for unsigned integers too large to fit in a s64
        ///
        bool positive() const { return _positive; }

        ///
        /// Compares if two values are equivalent, that is they have the same type and value
        /// @param other the value to compare with
        /// @return whether this is equivalent to the other value
        ///
        bool operator==(const Value & other) const;

        ///
        /// Directly compares if this value is equivalent to that provided
        ///
        bool operator==(const Object & val) const;
        bool operator==(const Array & val) const;
        bool operator==(const std::string & val) const;
        bool operator==(std::string_view val) const;
        bool operator==(const char * val) const;
        bool operator==(char val) const;
        bool operator==(s64 val) const;
        bool operator==(s32 val) const;
        bool operator==(s16 val) const;
        bool operator==(s8 val) const;
        bool operator==(u64 val) const;
        bool operator==(u32 val) const;
        bool operator==(u16 val) const;
        bool operator==(u8 val) const;
        bool operator==(double val) const;
        bool operator==(float val) const;
        bool operator==(bool val) const;
        bool operator==(nullptr_t) const;
        bool operator==(Datetime val) const;

      private:

        union
        {
            Object * _object;
            Array * _array;
            std::string * _string;
            s64 _integer;
            double _floater;
            bool _boolean;
            nullptr_t _null;
            Datetime _datetime;
        };
        Type _type{};
        bool _positive{};

        void _deleteValue();
    };

    static_assert(sizeof(Value) == 16u);

    ///
    /// Efficiently creates an object from the given key and value arguments
    /// @param key the first key, forwarded to `std::string` constructor
    /// @param val the first value, forwarded to `qcon::Value` constructor
    /// @param more any number of additional key and value arguments
    /// @return the created object
    ///
    template <typename K, typename V, typename... MoreKVs> Object makeObject(K && key, V && val, MoreKVs &&... moreKVs);
    Object makeObject();

    ///
    /// Efficiently creates an array from the given value arguments
    /// @param vals the values, each forwarded to `qcon::Value` constructor
    /// @return the created array
    ///
    template <typename... Vs> Array makeArray(Vs &&... vals);

    ///
    /// @param qcon the QCON string to decode
    /// @return the decoded value of the QCON, or empty if the string is invalid or could otherwise not be parsed
    ///
    std::optional<Value> decode(std::string_view qcon);

    ///
    /// @param val the QCON value to encode
    /// @param density the density of the encoded QCON string
    /// @param indentSpaces the number of spaces to insert per level of indentation
    /// @param singleQuotes whether to use `'` instead of `"` for strings
    /// @param identifiers whether to encode all eligible keys as identifiers instead of strings
    /// @return an encoded QCON string, or empty if there was an issue encoding the QCON
    ///
    std::optional<std::string> encode(const Value & val, Density density = multiline, unat indentSpaces = 4u);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
    inline Encoder & operator<<(Encoder & encoder, const Value & val)
    {
        switch (val.type())
        {
            case Type::null:
            {
                encoder << nullptr;
                break;
            }
            case Type::object:
            {
                encoder << object;
                for (const auto & [key, v] : *val._object)
                {
                    encoder << key << v;
                }
                encoder << end;
                break;
            }
            case Type::array:
            {
                encoder << array;
                for (const auto & v : *val._array)
                {
                    encoder << v;
                }
                encoder << end;
                break;
            }
            case Type::string:
            {
                encoder << *val._string;
                break;
            }
            case Type::integer:
            {
                if (val._positive)
                {
                    encoder << u64(val._integer);
                }
                else
                {
                    encoder << val._integer;
                }
                break;
            }
            case Type::floater:
            {
                encoder << val._floater;
                break;
            }
            case Type::boolean:
            {
                encoder << val._boolean;
                break;
            }
            case Type::datetime:
            {
                encoder << utc << val._datetime;
            }
        }

        return encoder;
    }

    // `Datetime` (`std::chrono::system_clock::time_point`) isn't trivially constuctible, so technically it shouldn't be
    //   used in a union, but it's really just a 64 bit integer, and it's *really* convenient to use it in the union,
    //   so tell the compiler to deal
    #pragma warning(push)
    #pragma warning(disable: 4582)

    inline Value::Value(Object && val) :
        _object{new Object{std::move(val)}},
        _type{Type::object}
    {}

    inline Value::Value(Array && val) :
        _array{new Array{std::move(val)}},
        _type{Type::array}
    {}

    inline Value::Value(std::string && val) :
        _string{new std::string{std::move(val)}},
        _type{Type::string}
    {}

    inline Value::Value(const std::string_view val) :
        _string{new std::string{val}},
        _type{Type::string}
    {}

    inline Value::Value(const char * const val) :
        Value(std::string_view{val})
    {}

    inline Value::Value(char * const val) :
        Value(std::string_view{val})
    {}

    inline Value::Value(const char val) :
        Value{std::string_view{&val, 1u}}
    {}

    inline Value::Value(const s64 val) :
        _integer{val},
        _type{Type::integer},
        _positive{_integer >= 0}
    {}

    inline Value::Value(const s32 val) :
        Value{s64{val}}
    {}

    inline Value::Value(const s16 val) :
        Value{s64{val}}
    {}

    inline Value::Value(const s8 val) :
        Value{s64{val}}
    {}

    inline Value::Value(const u64 val) :
        _integer{s64(val)},
        _type{Type::integer},
        _positive{true}
    {}

    inline Value::Value(const u32 val) :
        Value{u64{val}}
    {}

    inline Value::Value(const u16 val) :
        Value{u64{val}}
    {}

    inline Value::Value(const u8 val) :
        Value{u64{val}}
    {}

    inline Value::Value(const double val) :
        _floater{val},
        _type{Type::floater},
        _positive{_floater >= 0.0}
    {}

    inline Value::Value(const float val) :
        Value{double{val}}
    {}

    inline Value::Value(const bool val) :
        _boolean{val},
        _type{Type::boolean}
    {}

    inline Value::Value(nullptr_t) :
        _null{},
        _type{Type::null}
    {}

    inline Value::Value(const Datetime val) :
        _datetime{val},
        _type{Type::datetime}
    {}

    inline Value::Value(Value && other) :
        _integer{other._integer},
        _type{other._type},
        _positive{other._positive}
    {
        other._type = Type::null;
    }

    #pragma warning(pop)

    inline Value & Value::operator=(Object && val)
    {
        if (_type == Type::object)
        {
            *_object = std::move(val);
        }
        else
        {
            _deleteValue();
            _type = Type::object;
            _object = new Object{std::move(val)};
        }
        return *this;
    }

    inline Value & Value::operator=(Array && val)
    {
        if (_type == Type::array)
        {
            *_array = std::move(val);
        }
        else
        {
            _deleteValue();
            _type = Type::array;
            _array = new Array{std::move(val)};
        }
        return *this;
    }

    inline Value & Value::operator=(std::string && val)
    {
        if (_type == Type::string)
        {
            *_string = std::move(val);
        }
        else
        {
            _deleteValue();
            _type = Type::string;
            _string = new std::string{std::move(val)};
        }
        return *this;
    }

    inline Value & Value::operator=(const std::string_view val)
    {
        if (_type == Type::string)
        {
            *_string = val;
        }
        else
        {
            _deleteValue();
            _type = Type::string;
            _string = new std::string{val};
        }
        return *this;
    }

    inline Value & Value::operator=(const char * const val)
    {
        return *this = std::string_view{val};
    }

    inline Value & Value::operator=(const char val)
    {
        return *this = std::string_view{&val, 1u};
    }

    inline Value & Value::operator=(const s64 val)
    {
        if (_type != Type::integer)
        {
            _deleteValue();
            _type = Type::integer;
        }
        _integer = val;
        _positive = _integer >= 0;
        return *this;
    }

    inline Value & Value::operator=(const s32 val)
    {
        return *this = s64{val};
    }

    inline Value & Value::operator=(const s16 val)
    {
        return *this = s64{val};
    }

    inline Value & Value::operator=(const s8 val)
    {
        return *this = s64{val};
    }

    inline Value & Value::operator=(const u64 val)
    {
        if (_type != Type::integer)
        {
            _deleteValue();
            _type = Type::integer;
        }
        _integer = s64(val);
        _positive = true;
        return *this;
    }

    inline Value & Value::operator=(const u32 val)
    {
        return *this = u64{val};
    }

    inline Value & Value::operator=(const u16 val)
    {
        return *this = u64{val};
    }

    inline Value & Value::operator=(const u8 val)
    {
        return *this = u64{val};
    }

    inline Value & Value::operator=(const double val)
    {
        if (_type != Type::floater)
        {
            _deleteValue();
            _type = Type::floater;
        }
        _floater = val;
        _positive = _floater >= 0.0;
        return *this;
    }

    inline Value & Value::operator=(const float val)
    {
        return *this = double{val};
    }

    inline Value & Value::operator=(const bool val)
    {
        if (_type != Type::boolean)
        {
            _deleteValue();
            _type = Type::boolean;
        }
        _boolean = val;
        return *this;
    }

    inline Value & Value::operator=(const nullptr_t)
    {
        if (_type != Type::null)
        {
            _deleteValue();
            _type = Type::null;
        }
        _null = nullptr;
        return *this;
    }

    inline Value & Value::operator=(const Datetime val)
    {
        if (_type != Type::datetime)
        {
            _deleteValue();
            _type = Type::datetime;
        }
        _datetime = val;
        return *this;
    }

    inline Value & Value::operator=(Value && other)
    {
        _deleteValue();
        _integer = other._integer;
        _type = other._type;
        _positive = other._positive;
        other._type = Type::null;
        return *this;
    }

    inline Value::~Value()
    {
        _deleteValue();
    }

    inline Object * Value::object()
    {
        return _type == Type::object ? _object : nullptr;
    }

    inline const Object * Value::object() const
    {
        return _type == Type::object ? _object : nullptr;
    }

    inline Array * Value::array()
    {
        return _type == Type::array ? _array : nullptr;
    }

    inline const Array * Value::array() const
    {
        return _type == Type::array ? _array : nullptr;
    }

    inline std::string * Value::string()
    {
        return _type == Type::string ? _string : nullptr;
    }

    inline const std::string * Value::string() const
    {
        return _type == Type::string ? _string : nullptr;
    }

    inline s64 * Value::integer()
    {
        return _type == Type::integer ? &_integer : nullptr;
    }

    inline const s64 * Value::integer() const
    {
        return _type == Type::integer ? &_integer : nullptr;
    }

    inline double * Value::floater()
    {
        return _type == Type::floater ? &_floater : nullptr;
    }

    inline const double * Value::floater() const
    {
        return _type == Type::floater ? &_floater : nullptr;
    }

    inline bool * Value::boolean()
    {
        return _type == Type::boolean ? &_boolean : nullptr;
    }

    inline const bool * Value::boolean() const
    {
        return _type == Type::boolean ? &_boolean : nullptr;
    }

    inline nullptr_t * Value::null()
    {
        return _type == Type::null ? &_null : nullptr;
    }

    inline const nullptr_t * Value::null() const
    {
        return _type == Type::null ? &_null : nullptr;
    }

    inline Datetime * Value::datetime()
    {
        return _type == Type::datetime ? &_datetime : nullptr;
    }

    inline const Datetime * Value::datetime() const
    {
        return _type == Type::datetime ? &_datetime : nullptr;
    }

    inline bool Value::operator==(const Value & other) const
    {
        switch (other._type)
        {
            case Type::object: return *this == *other._object;
            case Type::array: return *this == *other._array;
            case Type::string: return *this == *other._string;
            case Type::integer: return *this == other._integer;
            case Type::floater: return *this == other._floater;
            case Type::boolean: return *this == other._boolean;
            case Type::null: return *this == other._null;
            case Type::datetime: return *this == other._datetime;
            default: return false;
        }
    }

    inline bool Value::operator==(const Object & val) const
    {
        return _type == Type::object && *_object == val;
    }

    inline bool Value::operator==(const Array & val) const
    {
        return _type == Type::array && *_array == val;
    }

    inline bool Value::operator==(const std::string & val) const
    {
        return *this == std::string_view{val};
    }

    inline bool Value::operator==(const std::string_view val) const
    {
        return _type == Type::string && *_string == val;
    }

    inline bool Value::operator==(const char * const val) const
    {
        return *this == std::string_view{val};
    }

    inline bool Value::operator==(const char val) const
    {
        return *this == std::string_view{&val, 1u};
    }

    inline bool Value::operator==(const s64 val) const
    {
        return _type == Type::integer && _integer == val;
    }

    inline bool Value::operator==(const s32 val) const
    {
        return *this == s64{val};
    }

    inline bool Value::operator==(const s16 val) const
    {
        return *this == s64{val};
    }

    inline bool Value::operator==(const s8 val) const
    {
        return *this == s64{val};
    }

    inline bool Value::operator==(const u64 val) const
    {
        return *this == s64(val);
    }

    inline bool Value::operator==(const u32 val) const
    {
        return *this == u64{val};
    }

    inline bool Value::operator==(const u16 val) const
    {
        return *this == u64{val};
    }

    inline bool Value::operator==(const u8 val) const
    {
        return *this == u64{val};
    }

    inline bool Value::operator==(const double val) const
    {
        return _type == Type::floater && _floater == val;
    }

    inline bool Value::operator==(const float val) const
    {
        return *this == double{val};
    }

    inline bool Value::operator==(const bool val) const
    {
        return _type == Type::boolean && _boolean == val;
    }

    inline bool Value::operator==(const nullptr_t) const
    {
        return _type == Type::null;
    }

    inline bool Value::operator==(const Datetime val) const
    {
        return _type == Type::datetime && _datetime == val;
    }

    inline void Value::_deleteValue()
    {
        switch (_type)
        {
            case Type::object: delete _object; break;
            case Type::array: delete _array; break;
            case Type::string: delete _string; break;
            default: break;
        }
    }

    template <typename K, typename V, typename... MoreKVs>
    inline void _makeObjectHelper(Object & obj, K && key, V && val, MoreKVs &&... moreKVs)
    {
        obj.emplace(std::forward<K>(key), std::forward<V>(val));
        if constexpr (sizeof...(moreKVs) > 0u)
        {
            _makeObjectHelper(obj, std::forward<MoreKVs>(moreKVs)...);
        }
    }

    template <typename K, typename V, typename... MoreKVs>
    inline Object makeObject(K && key, V && val, MoreKVs &&... moreKVs)
    {
        static_assert(sizeof...(moreKVs) % 2u == 0u, "Must provide an even number of arguments alternating between key and value");
        Object obj{};
        _makeObjectHelper(obj, std::forward<K>(key), std::forward<V>(val), std::forward<MoreKVs>(moreKVs)...);
        return obj;
    }

    inline Object makeObject()
    {
        return Object{};
    }

    template <typename... Vs>
    inline Array makeArray(Vs &&... vals)
    {
        Array arr{};
        arr.reserve(sizeof...(vals));
        (arr.emplace_back(std::forward<Vs>(vals)), ...);
        return arr;
    }

    inline bool _decodeArray(Decoder & decoder, Array & array);

    inline bool _decodeObject(Decoder & decoder, Object & object)
    {
        while (true)
        {
            switch (decoder.step())
            {
                case DecodeState::object:
                {
                    Value & val{object.emplace(std::move(decoder.key), Object{}).first->second};
                    if (!_decodeObject(decoder, *val.object()))
                    {
                        return false;
                    }
                    break;
                }
                case DecodeState::array:
                {
                    Value & val{object.emplace(std::move(decoder.key), Array{}).first->second};
                    if (!_decodeArray(decoder, *val.array()))
                    {
                        return false;
                    }
                    break;
                }
                case DecodeState::end:
                {
                    return true;
                }
                case DecodeState::string:
                {
                    object.emplace(std::move(decoder.key), std::move(decoder.string));
                    break;
                }
                case DecodeState::integer:
                {
                    if (decoder.positive)
                    {
                        object.emplace(std::move(decoder.key), u64(decoder.integer));
                    }
                    else
                    {
                        object.emplace(std::move(decoder.key), decoder.integer);
                    }
                    break;
                }
                case DecodeState::floater:
                {
                    object.emplace(std::move(decoder.key), decoder.floater);
                    break;
                }
                case DecodeState::boolean:
                {
                    object.emplace(std::move(decoder.key), decoder.boolean);
                    break;
                }
                case DecodeState::null:
                {
                    object.emplace(std::move(decoder.key), nullptr);
                    break;
                }
                case DecodeState::datetime:
                {
                    object.emplace(std::move(decoder.key), decoder.datetime);
                    break;
                }
                default:
                {
                    return false;
                }
            }
        }
    }

    inline bool _decodeArray(Decoder & decoder, Array & array)
    {
        while (true)
        {
            switch (decoder.step())
            {
                case DecodeState::object:
                {
                    Value & val{array.emplace_back(Object{})};
                    if (!_decodeObject(decoder, *val.object()))
                    {
                        return false;
                    }
                    break;
                }
                case DecodeState::array:
                {
                    Value & val{array.emplace_back(Array{})};
                    if (!_decodeArray(decoder, *val.array()))
                    {
                        return false;
                    }
                    break;
                }
                case DecodeState::end:
                {
                    return true;
                }
                case DecodeState::string:
                {
                    array.push_back(std::move(decoder.string));
                    break;
                }
                case DecodeState::integer:
                {
                    if (decoder.positive)
                    {
                        array.push_back(u64(decoder.integer));
                    }
                    else
                    {
                        array.push_back(decoder.integer);
                    }
                    break;
                }
                case DecodeState::floater:
                {
                    array.push_back(decoder.floater);
                    break;
                }
                case DecodeState::boolean:
                {
                    array.push_back(decoder.boolean);
                    break;
                }
                case DecodeState::null:
                {
                    array.push_back(nullptr);
                    break;
                }
                case DecodeState::datetime:
                {
                    array.push_back(decoder.datetime);
                    break;
                }
                default:
                {
                    return false;
                }
            }
        }
    }

    inline std::optional<Value> decode(const std::string_view qcon)
    {
        Value value{};
        Decoder decoder{qcon};

        switch (decoder.step())
        {
            case DecodeState::object:
            {
                Object obj{};
                if (!_decodeObject(decoder, obj))
                {
                    return {};
                }
                value = std::move(obj);
                break;
            }
            case DecodeState::array:
            {
                Array arr{};
                if (!_decodeArray(decoder, arr))
                {
                    return {};
                }
                value = std::move(arr);
                break;
            }
            case DecodeState::string:
            {
                value = std::move(decoder.string);
                break;
            }
            case DecodeState::integer:
            {
                if (decoder.positive)
                {
                    value = u64(decoder.integer);
                }
                else
                {
                    value = decoder.integer;
                }
                break;
            }
            case DecodeState::floater:
            {
                value = decoder.floater;
                break;
            }
            case DecodeState::boolean:
            {
                value = decoder.boolean;
                break;
            }
            case DecodeState::null:
            {
                break;
            }
            case DecodeState::datetime:
            {
                value = decoder.datetime;
                break;
            }
            default:
            {
                return {};
            }
        }

        if (decoder.step() == DecodeState::done)
        {
            return value;
        }
        else
        {
            return {};
        }
    }

    inline std::optional<std::string> encode(const Value & val, const Density density, unat indentSpaces)
    {
        Encoder encoder{density, indentSpaces};
        encoder << val;
        return encoder.finish();
    }
}
