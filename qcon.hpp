#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This header provides a DOM encoder and decoder
/// Uses `qcon-encode.hpp` to do the encoding and `qcon-decode.hpp` to do the decoding
/// See the README for more info and examples!
///

#include <cstring>

#include <concepts>
#include <map>
#include <memory>
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
        boolean
    };

    // Forward declarations
    class Value;
    class _Composer;

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
        /// Constructs a null value
        ///
        Value(std::nullptr_t = nullptr) {}

        ///
        /// @param val the value whith which to be constructed
        ///
        Value(Object && val);
        Value(Array && val);
        Value(std::string && val);
        Value(std::string_view val);
        Value(const char * val);
        Value(char * val);
        Value(char val);
        Value(int64_t val);
        Value(int32_t val);
        Value(int16_t val);
        Value(int8_t val);
        Value(uint64_t val);
        Value(uint32_t val);
        Value(uint16_t val);
        Value(uint8_t val);
        Value(double val);
        Value(float val);
        Value(bool val);

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
        Value & operator=(int64_t val);
        Value & operator=(int32_t val);
        Value & operator=(int16_t val);
        Value & operator=(int8_t val);
        Value & operator=(uint64_t val);
        Value & operator=(uint32_t val);
        Value & operator=(uint16_t val);
        Value & operator=(uint8_t val);
        Value & operator=(double val);
        Value & operator=(float val);
        Value & operator=(bool val);
        Value & operator=(nullptr_t);

        Value & operator=(const Value &) = delete;
        Value & operator=(Value && other);

        ~Value();

        ///
        /// @return the type of the value
        ///
        Type type() const { return _type; }

        ///
        /// @return whether the value is an object
        ///
        bool isObject() const { return _type == Type::object; }

        ///
        /// @return whether the value is an array
        ///
        bool isArray() const { return _type == Type::array; }

        ///
        /// @return whether the value is a string
        ///
        bool isString() const { return _type == Type::string; }

        ///
        /// @return whether the value is an integer
        ///
        bool isInteger() const { return _type == Type::integer; }

        ///
        /// @return whether the value is a floater
        ///
        bool isFloater() const { return _type == Type::floater; }

        ///
        /// @return whether the value is a boolean
        ///
        bool isBoolean() const { return _type == Type::boolean; }

        ///
        /// @return whether the value is null
        ///
        bool isNull() const { return _type == Type::null; }

        ///
        /// @tparam safety whether to check if this value is actually an object
        /// @return this value as an object if it is an object, otherwise null
        ///
        Object * asObject();
        const Object * asObject() const;

        ///
        /// @tparam safety whether to check if this value is actually an array
        /// @return this value as an array if it is an array, otherwise null
        ///
        Array * asArray();
        const Array * asArray() const;

        ///
        /// @tparam safety whether to check if this value is actually a string
        /// @return this value as a string if it is a string, otherwise null
        ///
        std::string * asString();
        const std::string * asString() const;

        ///
        /// @tparam safety whether to check if this value is actually an integer
        /// @return this value as an integer if it is an integer, otherwise null
        ///
        int64_t * asInteger();
        const int64_t * asInteger() const;

        ///
        /// @tparam safety whether to check if this value is actually a floater
        /// @return this value as a floater if it is a floater, otherwise null
        ///
        double * asFloater();
        const double * asFloater() const;

        ///
        /// @tparam safety whether to check if this value is actually a boolean
        /// @return this value as a boolean if it is a boolean, otherwise null
        ///
        bool * asBoolean();
        const bool * asBoolean() const;

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
        bool operator==(int64_t val) const;
        bool operator==(int32_t val) const;
        bool operator==(int16_t val) const;
        bool operator==(int8_t val) const;
        bool operator==(uint64_t val) const;
        bool operator==(uint32_t val) const;
        bool operator==(uint16_t val) const;
        bool operator==(uint8_t val) const;
        bool operator==(double val) const;
        bool operator==(float val) const;
        bool operator==(bool val) const;
        bool operator==(nullptr_t) const;

      private:

        friend class _Composer;

        union
        {
            nullptr_t _null{};
            Object * _object;
            Array * _array;
            std::string * _string;
            int64_t _integer;
            double _floater;
            bool _boolean;
        };
        Type _type{};

        void _deleteValue();
    };

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
    std::optional<std::string> encode(const Value & val, Density density = multiline, size_t indentSpaces = 4u);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qcon
{
    class _Composer
    {
      public:

        struct State { Value * node; Container container; };

        State object(State & outerState)
        {
            Value * innerNode;

            switch (outerState.container)
            {
                case Container::object:
                    innerNode = &outerState.node->_object->emplace(std::move(_key), Object{}).first->second;
                    break;
                case Container::array:
                    innerNode = &outerState.node->_array->emplace_back(Object{});
                    break;
                default:
                    *outerState.node = Object{};
                    innerNode = outerState.node;
            }

            return {innerNode, Container::object};
        }

        State array(State & outerState)
        {
            Value * innerNode;

            switch (outerState.container)
            {
                case Container::object:
                    innerNode = &outerState.node->_object->emplace(std::move(_key), Array{}).first->second;
                    break;
                case Container::array:
                    innerNode = &outerState.node->_array->emplace_back(Array{});
                    break;
                default:
                    *outerState.node = Array{};
                    innerNode = outerState.node;
            }

            return {innerNode, Container::array};
        }

        void key(const std::string_view k, State & /*state*/)
        {
            _key = k;
        }

        void end(State && /*innerState*/, State & /*outerState*/)
        {

        }

        template <typename T>
        void val(const T v, State & state)
        {
            Value * composedVal;

            switch (state.container)
            {
                case Container::object:
                    composedVal = &state.node->_object->emplace(std::move(_key), v).first->second;
                    break;
                case Container::array:
                    composedVal = &state.node->_array->emplace_back(v);
                    break;
                default:
                    *state.node = v;
                    composedVal = state.node;
            }
        }

        // TODO: Remove
        void val(const int64_t v, const bool /*positive*/, State & state)
        {
            Value * composedVal;

            switch (state.container)
            {
                case Container::object:
                    composedVal = &state.node->_object->emplace(std::move(_key), v).first->second;
                    break;
                case Container::array:
                    composedVal = &state.node->_array->emplace_back(v);
                    break;
                default:
                    *state.node = v;
                    composedVal = state.node;
            }
        }

      private:

        std::string _key{};
    };

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
                encoder << val._integer;
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
        }

        return encoder;
    }

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

    inline Value::Value(const int64_t val) :
        _integer{val},
        _type{Type::integer}
    {}

    inline Value::Value(const int32_t val) :
        Value{int64_t{val}}
    {}

    inline Value::Value(const int16_t val) :
        Value{int64_t{val}}
    {}

    inline Value::Value(const int8_t val) :
        Value{int64_t{val}}
    {}

    inline Value::Value(const uint64_t val) :
        Value{int64_t(val)}
    {}

    inline Value::Value(const uint32_t val) :
        Value{uint64_t{val}}
    {}

    inline Value::Value(const uint16_t val) :
        Value{uint64_t{val}}
    {}

    inline Value::Value(const uint8_t val) :
        Value{uint64_t{val}}
    {}

    inline Value::Value(const double val) :
        _floater{val},
        _type{Type::floater}
    {}

    inline Value::Value(const float val) :
        Value{double{val}}
    {}

    inline Value::Value(const bool val) :
        _boolean{val},
        _type{Type::boolean}
    {}

    inline Value::Value(Value && other) :
        _integer{std::exchange(other._integer, 0)},
        _type{std::exchange(other._type, Type::null)}
    {}

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

    inline Value & Value::operator=(const int64_t val)
    {
        if (_type != Type::integer)
        {
            _deleteValue();
            _type = Type::integer;
        }
        _integer = val;
        return *this;
    }

    inline Value & Value::operator=(const int32_t val)
    {
        return *this = int64_t{val};
    }

    inline Value & Value::operator=(const int16_t val)
    {
        return *this = int64_t{val};
    }

    inline Value & Value::operator=(const int8_t val)
    {
        return *this = int64_t{val};
    }

    inline Value & Value::operator=(const uint64_t val)
    {
        return *this = int64_t(val);
    }

    inline Value & Value::operator=(const uint32_t val)
    {
        return *this = uint64_t{val};
    }

    inline Value & Value::operator=(const uint16_t val)
    {
        return *this = uint64_t{val};
    }

    inline Value & Value::operator=(const uint8_t val)
    {
        return *this = uint64_t{val};
    }

    inline Value & Value::operator=(const double val)
    {
        if (_type != Type::floater)
        {
            _deleteValue();
            _type = Type::floater;
        }
        _floater = val;
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

    inline Value & Value::operator=(Value && other)
    {
        _deleteValue();
        _integer = std::exchange(other._integer, 0u);
        _type = std::exchange(other._type, Type::null);
        return *this;
    }

    inline Value::~Value()
    {
        _deleteValue();
    }

    inline Object * Value::asObject()
    {
        return isObject() ? _object : nullptr;
    }

    inline const Object * Value::asObject() const
    {
        return isObject() ? _object : nullptr;
    }

    inline Array * Value::asArray()
    {
        return isArray() ? _array : nullptr;
    }

    inline const Array * Value::asArray() const
    {
        return isArray() ? _array : nullptr;
    }

    inline std::string * Value::asString()
    {
        return isString() ? _string : nullptr;
    }

    inline const std::string * Value::asString() const
    {
        return isString() ? _string : nullptr;
    }

    inline int64_t * Value::asInteger()
    {
        return isInteger() ? &_integer : nullptr;
    }

    inline const int64_t * Value::asInteger() const
    {
        return isInteger() ? &_integer : nullptr;
    }

    inline double * Value::asFloater()
    {
        return isFloater() ? &_floater : nullptr;
    }

    inline const double * Value::asFloater() const
    {
        return isFloater() ? &_floater : nullptr;
    }

    inline bool * Value::asBoolean()
    {
        return isBoolean() ? &_boolean : nullptr;
    }

    inline const bool * Value::asBoolean() const
    {
        return isBoolean() ? &_boolean : nullptr;
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

    inline bool Value::operator==(const int64_t val) const
    {
        return _type == Type::integer && _integer == val;
    }

    inline bool Value::operator==(const int32_t val) const
    {
        return *this == int64_t{val};
    }

    inline bool Value::operator==(const int16_t val) const
    {
        return *this == int64_t{val};
    }

    inline bool Value::operator==(const int8_t val) const
    {
        return *this == int64_t{val};
    }

    inline bool Value::operator==(const uint64_t val) const
    {
        return *this == int64_t(val);
    }

    inline bool Value::operator==(const uint32_t val) const
    {
        return *this == uint64_t{val};
    }

    inline bool Value::operator==(const uint16_t val) const
    {
        return *this == uint64_t{val};
    }

    inline bool Value::operator==(const uint8_t val) const
    {
        return *this == uint64_t{val};
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

    inline std::optional<Value> decode(const std::string_view qcon)
    {
        Value root{};
        _Composer::State rootState{&root, end};
        _Composer composer{};
        if (decode(qcon, composer, rootState).success)
        {
            return root;
        }
        else
        {
            return {};
        }
    }

    inline std::optional<std::string> encode(const Value & val, const Density density, size_t indentSpaces)
    {
        Encoder encoder{density, indentSpaces};
        encoder << val;
        return encoder.finish();
    }
}
