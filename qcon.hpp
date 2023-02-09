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
        unsigner,
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
    /// Specialize `qcon::ValueFrom` to enable `Value` construction from custom types
    /// Example:
    ///     template <>
    ///     struct qcon::ValueFrom<std::pair<int, int>> {
    ///         qcon::Value operator()(const std::pair<int, int> & v) const {
    ///             return qcon::makeArray(v.first, f.second);
    ///         }
    ///     };
    ///
    template <typename T> struct ValueFrom;

    ///
    /// Concept describing a type for which `qcon::ValueFrom` has been specialized
    ///
    template <typename T> concept ValueFromAble = requires (T v) { { ::qcon::ValueFrom<T>{}(v) } -> std::same_as<Value>; };

    ///
    /// Specialize `qcon::ValueTo` to enable `Value::as` for custom types
    /// Example:
    ///     template <>
    ///     struct qcon::ValueTo<std::pair<int, int>> {
    ///         std::optional<std::pair<int, int>> operator()(const qcon::Value & v) const {
    ///             const qcon::Array * const arr{v.asArray()};
    ///             if (arr && arr->size() == 2u)
    ///             {
    ///                 const std::optional<int> v1{(*arr)[0].get<int>()};
    ///                 const std::optional<int> v2{(*arr)[1].get<int>()};
    ///                 if (v1 && v2)
    ///                 {
    ///                     return std::pair<int, int>{*v1, *v2};
    ///                 }
    ///             }
    ///             return {};
    ///         }
    ///     };
    ///
    template <typename T> struct ValueTo;

    ///
    /// Concept describing a type for which `qcon::ValueTo` has been specialized
    ///
    template <typename T> concept ValueToAble = requires (Value v) { { ::qcon::ValueTo<T>{}(v) } -> std::same_as<std::optional<T>>; };

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

        ///
        /// Attempts to construct a value from a custom type `T` using a specialized `qcon::ValueFrom` function,
        /// details of which can be found below
        /// @tparam T the custom type
        /// @param val the custom type value
        ///
        template <ValueFromAble T> Value(const T & val);

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
        /// @return whether the value is a signed integer
        ///
        bool isInteger() const { return _type == Type::integer; }

        ///
        /// @return whether the value is an unsigned integer
        ///
        bool isUnsigner() const { return _type == Type::unsigner; }

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
        /// Determines if the value type is compatible with `T`, which is to say calling `get<T>()` would be valid
        /// See the `get` method docs below for more details
        /// @tparam T the type in question, e.g. `int` or `std::string`
        /// @return whether the value type is compatible with type `T`
        ///
        template <typename T> bool is() const;

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
        /// @tparam safety whether to check if this value is actually a signed integer
        /// @return this value as a signed integer if it is a signed integer, otherwise null
        ///
        int64_t * asInteger();
        const int64_t * asInteger() const;

        ///
        /// @tparam safety whether to check if this value is actually an unsigned integer
        /// @return this value as an unsigned integer if it is an unsigned integer, otherwise null
        ///
        uint64_t * asUnsigner();
        const uint64_t * asUnsigner() const;

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
        /// Retrieves the value as the given type
        /// If the actual type does not match the requested type, returns empty
        /// If `T` is `std::string`, this call is equivalent to `asString`, except a copy of the string is returnsd
        /// If `T` is `std::string_view`, `const char *`, or `char *`, a view/pointer to the current string is returned
        /// If `T` is `char`, the first/only character of the current string is returned. If the current string has more
        ///   then one character, returns empty. Note that in c++ `char`, `signed char`, and `unsigned char` are
        ///   distinct types. Asking for a `signed char` or `unsigned char` will instead try to fetch a number of type
        ///   `int8_t` or `uint8_t` respectively
        /// If `T` is a numeric type...
        ///   ...and the value is a positive integer, it may be accessed as:
        ///     - any floater type (`double`, `float`)
        ///     - any signed integer type (`int64_t`, `int32_t`, `int16_t`, `int8_t`), but only if it can fit
        ///     - any unsigned integer type (`uint64_t`, `uint32_t`, `uint16_t`, `uint8_t`), but only if it can fit
        ///   ...and the value is a negative integer, it may be accessed as:
        ///     - any floater type (`double`, `float`)
        ///     - any signed integer type (`int64_t`, `int32_t`, `int16_t`, `int8_t`), but only if it can fit
        ///   ...and the value is not an integer, it may only be accessed as a floater (`double`, `float`)
        /// If `T` is `bool`, this call is equivalent to `asBoolean` by value
        /// If `T` is `nullptr_t` simply returns `nullptr`
        /// If `T` is an unrecognized type, then we attempt to use the specialized `qcon::ValueTo` struct, details
        ///   of which can be found below
        ///
        template <typename T> std::optional<T> get() const;

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
            uint64_t _unsigner;
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
    std::optional<std::string> encode(const Value & val, Density density = Density::multiline, size_t indentSpaces = 4u, bool singleQuotes = false, bool identifiers = false);
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
            case Type::unsigner:
            {
                encoder << val._unsigner;
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
        _unsigner{val},
        _type{Type::unsigner}
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

    template <ValueFromAble T>
    inline Value::Value(const T & val) :
        Value{::qcon::ValueFrom<T>{}(val)}
    {}

    inline Value::Value(Value && other) :
        _unsigner{std::exchange(other._unsigner, 0u)},
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
        if (_type != Type::unsigner)
        {
            _deleteValue();
            _type = Type::unsigner;
        }
        _unsigner = val;
        return *this;
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
        _unsigner = std::exchange(other._unsigner, 0u);
        _type = std::exchange(other._type, Type::null);
        return *this;
    }

    inline Value::~Value()
    {
        _deleteValue();
    }

    template <typename T>
    inline bool Value::is() const
    {
        using U = std::decay_t<T>;

        // Object
        if constexpr (std::is_same_v<U, Object>)
        {
            return isObject();
        }
        // Array
        else if constexpr (std::is_same_v<U, Array>)
        {
            return isArray();
        }
        // String
        else if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view> || std::is_same_v<U, const char *> || std::is_same_v<U, char *>)
        {
            return isString();
        }
        // Character
        else if constexpr (std::is_same_v<U, char>)
        {
            return isString() && _string->size() == 1u;
        }
        // Boolean
        else if constexpr (std::is_same_v<U, bool>)
        {
            return isBoolean();
        }
        // Signed integer
        else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>)
        {
            switch (_type)
            {
                case Type::integer:
                {
                    if constexpr (std::is_same_v<U, int64_t>)
                    {
                        return true;
                    }
                    else
                    {
                        return _integer <= std::numeric_limits<U>::max() && _integer >= std::numeric_limits<U>::min();
                    }
                }
                case Type::unsigner:
                {
                    return _unsigner <= uint64_t(std::numeric_limits<U>::max());
                }
                case Type::floater:
                {
                    return double(U(_floater)) == _floater;
                }
                default:
                {
                    return false;
                }
            }
        }
        // Unsigned integer
        else if constexpr (std::is_integral_v<U> && std::is_unsigned_v<U>)
        {
            switch (_type)
            {
                case Type::integer:
                {
                    return _integer >= 0 && uint64_t(_integer) <= std::numeric_limits<U>::max();
                }
                case Type::unsigner:
                {
                    if constexpr (std::is_same_v<U, uint64_t>)
                    {
                        return true;
                    }
                    else
                    {
                        return _unsigner <= std::numeric_limits<U>::max();
                    }
                }
                case Type::floater:
                {
                    return _floater >= 0.0 && double(U(_floater)) == _floater;
                }
                default:
                {
                    return false;
                }
            }
        }
        // Floater
        else if constexpr (std::is_floating_point_v<U>)
        {
            return _type == Type::integer || _type == Type::unsigner || _type == Type::floater;
        }
        // Other
        else
        {
            return false;
        }
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

    inline uint64_t * Value::asUnsigner()
    {
        return isUnsigner() ? &_unsigner : nullptr;
    }

    inline const uint64_t * Value::asUnsigner() const
    {
        return isUnsigner() ? &_unsigner : nullptr;
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

    template <typename T>
    inline std::optional<T> Value::get() const
    {
        using U = std::decay_t<T>;

        // Type must not be `qcon::Object`
        static_assert(!std::is_same_v<U, Object>, "This function would have to make a copy of the object, use `qcon::Value::asObject` instead");

        // Type must not be `qcon::Array`
        static_assert(!std::is_same_v<U, Array>, "This function would have to make a copy of the array. Use `qcon::Value::asArray` instead");

        // Type must not be `char *`
        static_assert(!std::is_same_v<U, char *>, "Mutable char pointer may not be accessed by const function. Use `qcon::Value::asString` or `qcon::Value::to<const char *>` instead");

        // String
        if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view>)
        {
            if (isString())
            {
                return *_string;
            }
            else
            {
                return {};
            }
        }
        else if constexpr (std::is_same_v<U, const char *>)
        {
            if (isString())
            {
                return _string->c_str();
            }
            else
            {
                return {};
            }
        }
        // Character
        else if constexpr (std::is_same_v<U, char>)
        {
            if (isString() && !_string->empty())
            {
                return _string->front();
            }
            else
            {
                return {};
            }
        }
        // Boolean
        else if constexpr (std::is_same_v<U, bool>)
        {
            if (isBoolean())
            {
                return _boolean;
            }
            else
            {
                return {};
            }
        }
        // Number
        else if constexpr (std::is_arithmetic_v<U>)
        {
            if (is<U>())
            {
                switch (_type)
                {
                    case Type::integer: return U(_integer);
                    case Type::unsigner: return U(_unsigner);
                    case Type::floater: return U(_floater);
                    default: return {};
                }
            }
            else
            {
                return {};
            }
        }
        // Null
        else if constexpr (std::is_same_v<U, nullptr_t>)
        {
            if (isNull())
            {
                return nullptr;
            }
            else
            {
                return {};
            }
        }
        // Other
        else
        {
            static_assert(ValueToAble<T>, "Must specialize `qcon::ValueTo` to convert to custom type");
            return ::qcon::ValueTo<U>{}(*this);
        }
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
            case Type::unsigner: return *this == other._unsigner;
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
        switch (_type)
        {
            case Type::integer: return _integer == val;
            case Type::unsigner: return val >= 0 && _unsigner == uint64_t(val);
            case Type::floater: return _floater == double(val) && int64_t(_floater) == val;
            default: return false;
        }
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
        switch (_type)
        {
            case Type::integer: return _integer >= 0 && uint64_t(_integer) == val;
            case Type::unsigner: return _unsigner == val;
            case Type::floater: return _floater == double(val) && uint64_t(_floater) == val;
            default: return false;
        }
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
        switch (_type)
        {
            case Type::integer: return double(_integer) == val && _integer == int64_t(val);
            case Type::unsigner: return double(_unsigner) == val && _unsigner == uint64_t(val);
            case Type::floater: return _floater == val;
            default: return false;
        }
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
        _Composer::State rootState{&root, Container::none};
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

    inline std::optional<std::string> encode(const Value & val, const Density density, size_t indentSpaces, bool singleQuotes, bool identifiers)
    {
        Encoder encoder{density, indentSpaces, singleQuotes, identifiers};
        encoder << val;
        return encoder.finish();
    }
}
