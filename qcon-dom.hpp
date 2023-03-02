#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This header provides a DOM QCON encoder and decoder
/// Uses `qcon-encode.hpp` to do the encoding and `qcon-decode.hpp` to do the decoding
/// See the README for more info
///

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
        date,
        time,
        datetime
    };

    class Value;

    // TODO: Use something more efficient
    using Object = std::map<std::string, Value>;

    using Array = std::vector<Value>;

    ///
    /// Represents a QCON value
    ///
    class Value
    {
      public:

        ///
        /// Specialization of `Encoder`'s `operator<<` for `Value`
        /// All default formatting is used. Containers are the base density, number are base 10, etc.
        ///
        friend Encoder & operator<<(Encoder & encoder, const Value & v);

        ///
        /// Construct a QCON value from the given underlying value
        /// @param v value with which to be constructed
        ///
        Value(std::nullptr_t = nullptr);
        Value(Object && v);
        Value(Array && v);
        Value(std::string && v);
        Value(std::string_view v);
        Value(const char * v);
        Value(char * v);
        Value(char v);
        Value(s64 v);
        Value(s32 v);
        Value(s16 v);
        Value(s8 v);
        Value(u64 v);
        Value(u32 v);
        Value(u16 v);
        Value(u8 v);
        Value(double v);
        Value(float v);
        Value(bool v);
        Value(const Date & v);
        Value(const Time & v);
        Value(const Datetime & v);

        Value(const Value &) = delete;
        Value(Value && other);

        ///
        /// Assigns the QCON value witha new underlying value
        /// @param v new value
        /// @return this
        ///
        Value & operator=(Object && v);
        Value & operator=(Array && v);
        Value & operator=(std::string && v);
        Value & operator=(std::string_view v);
        Value & operator=(const char * v);
        Value & operator=(char v);
        Value & operator=(s64 v);
        Value & operator=(s32 v);
        Value & operator=(s16 v);
        Value & operator=(s8 v);
        Value & operator=(u64 v);
        Value & operator=(u32 v);
        Value & operator=(u16 v);
        Value & operator=(u8 v);
        Value & operator=(double v);
        Value & operator=(float v);
        Value & operator=(bool v);
        Value & operator=(const Date & v);
        Value & operator=(const Time & v);
        Value & operator=(const Datetime & v);
        Value & operator=(nullptr_t);

        Value & operator=(const Value &) = delete;
        Value & operator=(Value && other);

        ~Value();

        ///
        /// @return type of the value
        ///
        [[nodiscard]] Type type() const { return _type; }

        ///
        /// @return this value as an object if it is an object, otherwise null
        ///
        [[nodiscard]] Object * object();
        [[nodiscard]] const Object * object() const;

        ///
        /// @return this value as an array if it is an array, otherwise null
        ///
        [[nodiscard]] Array * array();
        [[nodiscard]] const Array * array() const;

        ///
        /// @return this value as a string if it is a string, otherwise null
        ///
        [[nodiscard]] std::string * string();
        [[nodiscard]] const std::string * string() const;

        ///
        /// @return this value as an integer if it is an integer, otherwise null
        ///
        [[nodiscard]] s64 * integer();
        [[nodiscard]] const s64 * integer() const;

        ///
        /// @return this value as a floater if it is a floater, otherwise null
        ///
        [[nodiscard]] double * floater();
        [[nodiscard]] const double * floater() const;

        ///
        /// @return this value as a boolean if it is a boolean, otherwise null
        ///
        [[nodiscard]] bool * boolean();
        [[nodiscard]] const bool * boolean() const;

        ///
        /// @return this value as a date if it is a date or datetime, otherwise null
        ///
        [[nodiscard]] Date * date();
        [[nodiscard]] const Date * date() const;

        ///
        /// @return this value as a time if it is a time or datetime, otherwise null
        ///
        [[nodiscard]] Time * time();
        [[nodiscard]] const Time * time() const;

        ///
        /// @return this value as a datetime if it is a datetime, otherwise null
        ///
        [[nodiscard]] Datetime * datetime();
        [[nodiscard]] const Datetime * datetime() const;

        ///
        /// @return this value as a null if it is a null, otherwise null
        ///
        [[nodiscard]] nullptr_t * null();
        [[nodiscard]] const nullptr_t * null() const;

        ///
        /// @return whether the number was positive; useful for unsigned integers too large to fit in a s64
        ///
        [[nodiscard]] bool positive() const { return _positive; }

        ///
        /// @return whether this has the same type and value as `other`
        ///
        [[nodiscard]] bool operator==(const Value & other) const;

        ///
        /// Directly compares if this is the same type and has the same value as that provided
        ///
        [[nodiscard]] bool operator==(const Object & v) const;
        [[nodiscard]] bool operator==(const Array & v) const;
        [[nodiscard]] bool operator==(const std::string & v) const;
        [[nodiscard]] bool operator==(std::string_view v) const;
        [[nodiscard]] bool operator==(const char * v) const;
        [[nodiscard]] bool operator==(char v) const;
        [[nodiscard]] bool operator==(s64 v) const;
        [[nodiscard]] bool operator==(s32 v) const;
        [[nodiscard]] bool operator==(s16 v) const;
        [[nodiscard]] bool operator==(s8 v) const;
        [[nodiscard]] bool operator==(u64 v) const;
        [[nodiscard]] bool operator==(u32 v) const;
        [[nodiscard]] bool operator==(u16 v) const;
        [[nodiscard]] bool operator==(u8 v) const;
        [[nodiscard]] bool operator==(double v) const;
        [[nodiscard]] bool operator==(float v) const;
        [[nodiscard]] bool operator==(bool v) const;
        [[nodiscard]] bool operator==(const Date & v) const;
        [[nodiscard]] bool operator==(const Time & v) const;
        [[nodiscard]] bool operator==(const Datetime & v) const;
        [[nodiscard]] bool operator==(nullptr_t) const;

      private:

        union
        {
            Object * _object;
            Array * _array;
            std::string * _string;
            s64 _integer;
            double _floater;
            bool _boolean;
            Datetime * _datetime;
            nullptr_t _null;
        };
        Type _type{};
        bool _positive{};

        void _deleteValue();
    };

    /// `Value` is small, allowing for efficient container storage
    static_assert(sizeof(Value) == 16u);

    ///
    /// Creates an object by forward-constructing from the given key value pairs
    /// @param k first key
    /// @param v first value
    /// @param more any number of additional key and value pairs
    /// @return created object
    ///
    template <typename K, typename V, typename... MoreKVs> [[nodiscard]] Object makeObject(K && k, V && v, MoreKVs &&... moreKVs);
    [[nodiscard]] Object makeObject();

    ///
    /// Creates an array by forward-constructing from the given values
    /// @param vals any number of values to create into an array
    /// @return created array
    ///
    template <typename... Vs> [[nodiscard]] Array makeArray(Vs &&... vals);

    ///
    /// Decodes the given QCON string
    /// The QSON string *must* be null terminated (optimization allowing most range checks to be eliminated)
    /// @param qcon QCON string to decode
    /// @return decoded value of the QCON, or empty if the string is invalid or could otherwise not be decoded
    ///
    [[nodiscard]] std::optional<Value> decode(const char * qcon);
    [[nodiscard]] std::optional<Value> decode(const std::string & qcon) { return decode(qcon.c_str()); }
    [[nodiscard]] std::optional<Value> decode(std::string &&) = delete; /// Prevent binding to temporary
    [[nodiscard]] std::optional<Value> decode(std::string_view) = delete; /// QCON string must be null terminated, pass c-string instead

    ///
    /// Encodes the QCON value into a QCON string
    /// @param v QCON value to encode
    /// @param density density of the encoded QCON string
    /// @param indentStr string to use for indent; must be whitespace
    /// @return encoded QCON string, or empty if there was an issue encoding the QCON
    ///
    [[nodiscard]] std::optional<std::string> encode(const Value & v, Density density = Encoder::defaultDensity, std::string_view indentStr = Encoder::defaultIndentString);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
    inline Encoder & operator<<(Encoder & encoder, const Value & v)
    {
        switch (v.type())
        {
            case Type::null:
            {
                encoder << nullptr;
                break;
            }
            case Type::object:
            {
                encoder << object;
                for (const auto & [key, value] : *v._object)
                {
                    encoder << key << value;
                }
                encoder << end;
                break;
            }
            case Type::array:
            {
                encoder << array;
                for (const auto & value : *v._array)
                {
                    encoder << value;
                }
                encoder << end;
                break;
            }
            case Type::string:
            {
                encoder << *v._string;
                break;
            }
            case Type::integer:
            {
                if (v._positive)
                {
                    encoder << u64(v._integer);
                }
                else
                {
                    encoder << v._integer;
                }
                break;
            }
            case Type::floater:
            {
                encoder << v._floater;
                break;
            }
            case Type::boolean:
            {
                encoder << v._boolean;
                break;
            }
            case Type::date:
            {
                encoder << v._datetime->date;
                break;
            }
            case Type::time:
            {
                encoder << v._datetime->time;
                break;
            }
            case Type::datetime:
            {
                encoder << *v._datetime;
                break;
            }
        }

        return encoder;
    }

    inline Value::Value(Object && v) :
        _object{new Object{std::move(v)}},
        _type{Type::object}
    {}

    inline Value::Value(Array && v) :
        _array{new Array{std::move(v)}},
        _type{Type::array}
    {}

    inline Value::Value(std::string && v) :
        _string{new std::string{std::move(v)}},
        _type{Type::string}
    {}

    inline Value::Value(const std::string_view v) :
        _string{new std::string{v}},
        _type{Type::string}
    {}

    inline Value::Value(const char * const v) :
        Value(std::string_view{v})
    {}

    inline Value::Value(char * const v) :
        Value(std::string_view{v})
    {}

    inline Value::Value(const char v) :
        Value{std::string_view{&v, 1u}}
    {}

    inline Value::Value(const s64 v) :
        _integer{v},
        _type{Type::integer},
        _positive{_integer >= 0}
    {}

    inline Value::Value(const s32 v) :
        Value{s64{v}}
    {}

    inline Value::Value(const s16 v) :
        Value{s64{v}}
    {}

    inline Value::Value(const s8 v) :
        Value{s64{v}}
    {}

    inline Value::Value(const u64 v) :
        _integer{s64(v)},
        _type{Type::integer},
        _positive{true}
    {}

    inline Value::Value(const u32 v) :
        Value{u64{v}}
    {}

    inline Value::Value(const u16 v) :
        Value{u64{v}}
    {}

    inline Value::Value(const u8 v) :
        Value{u64{v}}
    {}

    inline Value::Value(const double v) :
        _floater{v},
        _type{Type::floater},
        _positive{_floater >= 0.0}
    {}

    inline Value::Value(const float v) :
        Value{double{v}}
    {}

    inline Value::Value(const bool v) :
        _boolean{v},
        _type{Type::boolean}
    {}

    inline Value::Value(const Date & v) :
        _datetime{new Datetime{.date = v}},
        _type{Type::date}
    {}

    inline Value::Value(const Time & v) :
        _datetime{new Datetime{.time = v}},
        _type{Type::time}
    {}

    inline Value::Value(const Datetime & v) :
        _datetime{new Datetime{v}},
        _type{Type::datetime}
    {}

    inline Value::Value(nullptr_t) :
        _null{},
        _type{Type::null}
    {}

    inline Value::Value(Value && other) :
        _integer{other._integer},
        _type{other._type},
        _positive{other._positive}
    {
        other._type = Type::null;
    }

    inline Value & Value::operator=(Object && v)
    {
        if (_type == Type::object)
        {
            *_object = std::move(v);
        }
        else
        {
            _deleteValue();
            _type = Type::object;
            _object = new Object{std::move(v)};
        }
        return *this;
    }

    inline Value & Value::operator=(Array && v)
    {
        if (_type == Type::array)
        {
            *_array = std::move(v);
        }
        else
        {
            _deleteValue();
            _type = Type::array;
            _array = new Array{std::move(v)};
        }
        return *this;
    }

    inline Value & Value::operator=(std::string && v)
    {
        if (_type == Type::string)
        {
            *_string = std::move(v);
        }
        else
        {
            _deleteValue();
            _type = Type::string;
            _string = new std::string{std::move(v)};
        }
        return *this;
    }

    inline Value & Value::operator=(const std::string_view v)
    {
        if (_type == Type::string)
        {
            *_string = v;
        }
        else
        {
            _deleteValue();
            _type = Type::string;
            _string = new std::string{v};
        }
        return *this;
    }

    inline Value & Value::operator=(const char * const v)
    {
        return *this = std::string_view{v};
    }

    inline Value & Value::operator=(const char v)
    {
        return *this = std::string_view{&v, 1u};
    }

    inline Value & Value::operator=(const s64 v)
    {
        if (_type != Type::integer)
        {
            _deleteValue();
            _type = Type::integer;
        }
        _integer = v;
        _positive = _integer >= 0;
        return *this;
    }

    inline Value & Value::operator=(const s32 v)
    {
        return *this = s64{v};
    }

    inline Value & Value::operator=(const s16 v)
    {
        return *this = s64{v};
    }

    inline Value & Value::operator=(const s8 v)
    {
        return *this = s64{v};
    }

    inline Value & Value::operator=(const u64 v)
    {
        if (_type != Type::integer)
        {
            _deleteValue();
            _type = Type::integer;
        }
        _integer = s64(v);
        _positive = true;
        return *this;
    }

    inline Value & Value::operator=(const u32 v)
    {
        return *this = u64{v};
    }

    inline Value & Value::operator=(const u16 v)
    {
        return *this = u64{v};
    }

    inline Value & Value::operator=(const u8 v)
    {
        return *this = u64{v};
    }

    inline Value & Value::operator=(const double v)
    {
        if (_type != Type::floater)
        {
            _deleteValue();
            _type = Type::floater;
        }
        _floater = v;
        _positive = _floater >= 0.0;
        return *this;
    }

    inline Value & Value::operator=(const float v)
    {
        return *this = double{v};
    }

    inline Value & Value::operator=(const bool v)
    {
        if (_type != Type::boolean)
        {
            _deleteValue();
            _type = Type::boolean;
        }
        _boolean = v;
        return *this;
    }

    inline Value & Value::operator=(const Date & v)
    {
        if (_type == Type::date || _type == Type::time || _type == Type::datetime)
        {
            _datetime->date = v;
        }
        else
        {
            _deleteValue();
            _datetime = new Datetime{.date = v};
        }
        _type = Type::date;
        return *this;
    }

    inline Value & Value::operator=(const Time & v)
    {
        if (_type == Type::date || _type == Type::time || _type == Type::datetime)
        {
            _datetime->time = v;
        }
        else
        {
            _deleteValue();
            _datetime = new Datetime{.time = v};
        }
        _type = Type::time;
        return *this;
    }

    inline Value & Value::operator=(const Datetime & v)
    {
        if (_type == Type::date || _type == Type::time || _type == Type::datetime)
        {
            *_datetime = v;
        }
        else
        {
            _deleteValue();
            _datetime = new Datetime{v};
        }
        _type = Type::datetime;
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

    inline Date * Value::date()
    {
        return _type == Type::date || _type == Type::datetime ? &_datetime->date : nullptr;
    }

    inline const Date * Value::date() const
    {
        return _type == Type::date || _type == Type::datetime ? &_datetime->date : nullptr;
    }

    inline Time * Value::time()
    {
        return _type == Type::time || _type == Type::datetime ? &_datetime->time : nullptr;
    }

    inline const Time * Value::time() const
    {
        return _type == Type::time || _type == Type::datetime ? &_datetime->time : nullptr;
    }

    inline Datetime * Value::datetime()
    {
        return _type == Type::datetime ? _datetime : nullptr;
    }

    inline const Datetime * Value::datetime() const
    {
        return _type == Type::datetime ? _datetime : nullptr;
    }

    inline nullptr_t * Value::null()
    {
        return _type == Type::null ? &_null : nullptr;
    }

    inline const nullptr_t * Value::null() const
    {
        return _type == Type::null ? &_null : nullptr;
    }

    inline bool Value::operator==(const Value & other) const
    {
        switch (other._type)
        {
            case Type::null: return *this == other._null;
            case Type::object: return *this == *other._object;
            case Type::array: return *this == *other._array;
            case Type::string: return *this == *other._string;
            case Type::integer: return *this == other._integer;
            case Type::floater: return *this == other._floater;
            case Type::boolean: return *this == other._boolean;
            case Type::date: return *this == other._datetime->date;
            case Type::time: return *this == other._datetime->time;
            case Type::datetime: return *this == *other._datetime;
            default: return false;
        }
    }

    inline bool Value::operator==(const Object & v) const
    {
        return _type == Type::object && *_object == v;
    }

    inline bool Value::operator==(const Array & v) const
    {
        return _type == Type::array && *_array == v;
    }

    inline bool Value::operator==(const std::string & v) const
    {
        return *this == std::string_view{v};
    }

    inline bool Value::operator==(const std::string_view v) const
    {
        return _type == Type::string && *_string == v;
    }

    inline bool Value::operator==(const char * const v) const
    {
        return *this == std::string_view{v};
    }

    inline bool Value::operator==(const char v) const
    {
        return *this == std::string_view{&v, 1u};
    }

    inline bool Value::operator==(const s64 v) const
    {
        return _type == Type::integer && _integer == v;
    }

    inline bool Value::operator==(const s32 v) const
    {
        return *this == s64{v};
    }

    inline bool Value::operator==(const s16 v) const
    {
        return *this == s64{v};
    }

    inline bool Value::operator==(const s8 v) const
    {
        return *this == s64{v};
    }

    inline bool Value::operator==(const u64 v) const
    {
        return *this == s64(v);
    }

    inline bool Value::operator==(const u32 v) const
    {
        return *this == u64{v};
    }

    inline bool Value::operator==(const u16 v) const
    {
        return *this == u64{v};
    }

    inline bool Value::operator==(const u8 v) const
    {
        return *this == u64{v};
    }

    inline bool Value::operator==(const double v) const
    {
        return _type == Type::floater && _floater == v;
    }

    inline bool Value::operator==(const float v) const
    {
        return *this == double{v};
    }

    inline bool Value::operator==(const bool v) const
    {
        return _type == Type::boolean && _boolean == v;
    }

    inline bool Value::operator==(const Date & v) const
    {
        return _type == Type::date && _datetime->date == v;
    }

    inline bool Value::operator==(const Time & v) const
    {
        return _type == Type::time && _datetime->time == v;
    }

    inline bool Value::operator==(const Datetime & v) const
    {
        return _type == Type::datetime && _datetime->date == v.date && _datetime->time == v.time && _datetime->zone.format == v.zone.format && _datetime->zone.offset == v.zone.offset;
    }

    inline bool Value::operator==(const nullptr_t) const
    {
        return _type == Type::null;
    }

    inline void Value::_deleteValue()
    {
        switch (_type)
        {
            case Type::object: delete _object; break;
            case Type::array: delete _array; break;
            case Type::string: delete _string; break;
            case Type::date: [[fallthrough]];
            case Type::time: [[fallthrough]];
            case Type::datetime: delete _datetime; break;
            default: break;
        }
    }

    template <typename K, typename V, typename... MoreKVs>
    inline void _makeObjectHelper(Object & obj, K && k, V && v, MoreKVs &&... moreKVs)
    {
        obj.emplace(std::forward<K>(k), std::forward<V>(v));
        if constexpr (sizeof...(moreKVs) > 0u)
        {
            _makeObjectHelper(obj, std::forward<MoreKVs>(moreKVs)...);
        }
    }

    template <typename K, typename V, typename... MoreKVs>
    inline Object makeObject(K && k, V && v, MoreKVs &&... moreKVs)
    {
        static_assert(sizeof...(moreKVs) % 2u == 0u, "Must provide an even number of arguments alternating between key and value");

        Object obj{};
        _makeObjectHelper(obj, std::forward<K>(k), std::forward<V>(v), std::forward<MoreKVs>(moreKVs)...);
        return obj;
    }

    inline Object makeObject()
    {
        return Object{};
    }

    template <typename... Vs>
    inline Array makeArray(Vs &&... vs)
    {
        Array arr{};
        arr.reserve(sizeof...(vs));
        (arr.emplace_back(std::forward<Vs>(vs)), ...);
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
                    Value & v{object.emplace(std::move(decoder.key), Object{}).first->second};
                    if (!_decodeObject(decoder, *v.object()))
                    {
                        return false;
                    }
                    break;
                }
                case DecodeState::array:
                {
                    Value & v{object.emplace(std::move(decoder.key), Array{}).first->second};
                    if (!_decodeArray(decoder, *v.array()))
                    {
                        return false;
                    }
                    break;
                }
                case DecodeState::end:
                {
                    return true;
                }
                case DecodeState::key:
                {
                    break;
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
                case DecodeState::date:
                {
                    object.emplace(std::move(decoder.key), decoder.date);
                    break;
                }
                case DecodeState::time:
                {
                    object.emplace(std::move(decoder.key), decoder.time);
                    break;
                }
                case DecodeState::datetime:
                {
                    object.emplace(std::move(decoder.key), decoder.datetime);
                    break;
                }
                case DecodeState::null:
                {
                    object.emplace(std::move(decoder.key), nullptr);
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
                    Value & v{array.emplace_back(Object{})};
                    if (!_decodeObject(decoder, *v.object()))
                    {
                        return false;
                    }
                    break;
                }
                case DecodeState::array:
                {
                    Value & v{array.emplace_back(Array{})};
                    if (!_decodeArray(decoder, *v.array()))
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
                case DecodeState::date:
                {
                    array.push_back(decoder.date);
                    break;
                }
                case DecodeState::time:
                {
                    array.push_back(decoder.time);
                    break;
                }
                case DecodeState::datetime:
                {
                    array.push_back(decoder.datetime);
                    break;
                }
                case DecodeState::null:
                {
                    array.push_back(nullptr);
                    break;
                }
                default:
                {
                    return false;
                }
            }
        }
    }

    inline std::optional<Value> decode(const char * const qcon)
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
            case DecodeState::date:
            {
                value = decoder.date;
                break;
            }
            case DecodeState::time:
            {
                value = decoder.time;
                break;
            }
            case DecodeState::datetime:
            {
                value = decoder.datetime;
                break;
            }
            case DecodeState::null:
            {
                break;
            }
            default:
            {
                return {};
            }
        }

        if (decoder)
        {
            return value;
        }
        else
        {
            return {};
        }
    }

    inline std::optional<std::string> encode(const Value & v, const Density density, const std::string_view indentStr)
    {
        Encoder encoder{density, indentStr};
        encoder << v;
        return encoder.finish();
    }
}
