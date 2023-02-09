#pragma once

///
/// QCON 0.0.0
/// https://github.com/daskie/qcon
/// This header provides a DOM encoder and decoder
/// Uses `qcon-encode.hpp` to do the encoding and `qcon-decode.hpp` to do the decoding
/// See the README for more info and examples!
///

#include <cstring>

#include <algorithm>
#include <concepts>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include <qcon-decode.hpp>
#include <qcon-encode.hpp>

namespace qcon
{
    ///
    /// Required for low-order bit packing
    ///
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= 8);

    ///
    /// The type of the QCON value
    ///
    enum class Type : uint8_t
    {
        null     = 0b000u,
        object   = 0b001u,
        array    = 0b010u,
        string   = 0b011u,
        integer  = 0b100u,
        unsigner = 0b101u,
        floater  = 0b110u,
        boolean  = 0b111u
    };

    // Forward declarations
    class Value;
    class _Composer;

    ///
    /// Convenience type alias
    /// The internal representation for objects is `std::map<string, qcon::value>`
    ///
    using Object = std::map<string, Value>;

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
        Value(Object && val, Density density = Density::unspecified);
        Value(Array && val, Density density = Density::unspecified);
        Value(string && val);
        Value(string_view val);
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
        Value & operator=(string && val);
        Value & operator=(string_view val);
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
        Type type() const;

        ///
        /// @return the density of the object or array, or `unspecified` if not an object or array
        ///
        Density density() const;

        ///
        /// Sets the density of the object or array
        /// @param density the new density
        ///
        void setDensity(Density density);

        ///
        /// @return whether the value is an object
        ///
        bool isObject() const;

        ///
        /// @return whether the value is an array
        ///
        bool isArray() const;

        ///
        /// @return whether the value is a string
        ///
        bool isString() const;

        ///
        /// @return whether the value is a number
        ///
        bool isNumber() const;

        ///
        /// @return whether the value is a signed integer
        ///
        bool isInteger() const;

        ///
        /// @return whether the value is an unsigned integer
        ///
        bool isUnsigner() const;

        ///
        /// @return whether the value is a floater
        ///
        bool isFloater() const;

        ///
        /// @return whether the value is a boolean
        ///
        bool isBoolean() const;

        ///
        /// @return whether the value is null
        ///
        bool isNull() const;

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
        string * asString();
        const string * asString() const;

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
        /// @return whether the value has a comment
        ///
        bool hasComment() const;

        ///
        /// @return the value's comment, or `nullptr` if it has no comment
        ///
        string * comment();
        const string * comment() const;

        ///
        /// @param str the new comment
        ///
        void setComment(string && str);
        void setComment(string_view str);
        void setComment(const char * str);

        ///
        /// Removes the value's comment
        /// @return ownership of the value's comment
        ///
        std::unique_ptr<string> removeComment();

        ///
        /// Compares if two values are equivalent, that is they have the same type and value
        /// The presence/content of comments is ignored
        /// @param other the value to compare with
        /// @return whether this is equivalent to the other value
        ///
        bool operator==(const Value & other) const;

        ///
        /// Directly compares if this value is equivalent to that provided
        ///
        bool operator==(const Object & val) const;
        bool operator==(const Array & val) const;
        bool operator==(const string & val) const;
        bool operator==(string_view val) const;
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
            uintptr_t _ptrAndDensity;
            string * _string;
            int64_t _integer;
            uint64_t _unsigner;
            double _floater;
            bool _boolean;
            nullptr_t _null{};
        };
        uintptr_t _typeAndComment{};

        Object * _object();
        const Object * _object() const;

        Array * _array();
        const Array * _array() const;

        void _setType(Type type);

        template <typename T> void _setComment(T && str);

        void _deleteValue();

        void _deleteComment();
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
    std::optional<Value> decode(string_view qcon);

    ///
    /// @param val the QCON value to encode
    /// @param density the base density of the encoded QCON string
    /// @param indentSpaces the number of spaces to insert per level of indentation
    /// @param singleQuotes whether to use `'` instead of `"` for strings
    /// @param identifiers whether to encode all eligible keys as identifiers instead of strings
    /// @return an encoded QCON string, or empty if there was an issue encoding the QCON
    ///
    std::optional<string> encode(const Value & val, Density density = Density::multiline, size_t indentSpaces = 4u, bool singleQuotes = false, bool identifiers = false);
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
                    innerNode = &outerState.node->_object()->emplace(std::move(_key), Object{}).first->second;
                    break;
                case Container::array:
                    innerNode = &outerState.node->_array()->emplace_back(Object{});
                    break;
                default:
                    *outerState.node = Object{};
                    innerNode = outerState.node;
            }

            if (!_comment.empty())
            {
                innerNode->setComment(std::move(_comment));
            }

            return {innerNode, Container::object};
        }

        State array(State & outerState)
        {
            Value * innerNode;

            switch (outerState.container)
            {
                case Container::object:
                    innerNode = &outerState.node->_object()->emplace(std::move(_key), Array{}).first->second;
                    break;
                case Container::array:
                    innerNode = &outerState.node->_array()->emplace_back(Array{});
                    break;
                default:
                    *outerState.node = Array{};
                    innerNode = outerState.node;
            }

            if (!_comment.empty())
            {
                innerNode->setComment(std::move(_comment));
            }

            return {innerNode, Container::array};
        }

        void key(const string_view k, State & /*state*/)
        {
            _key = k;
        }

        void end(const Density density, State && innerState, State & /*outerState*/)
        {
            switch (innerState.container)
            {
                case Container::object:
                    innerState.node->setDensity(density);
                    break;
                case Container::array:
                    innerState.node->setDensity(density);
                    break;
                default:
                    break;
            }

            _comment.clear();
        }

        template <typename T>
        void val(const T v, State & state)
        {
            Value * composedVal;

            switch (state.container)
            {
                case Container::object:
                    composedVal = &state.node->_object()->emplace(std::move(_key), v).first->second;
                    break;
                case Container::array:
                    composedVal = &state.node->_array()->emplace_back(v);
                    break;
                default:
                    *state.node = v;
                    composedVal = state.node;
            }

            if (!_comment.empty())
            {
                composedVal->setComment(std::move(_comment));
            }
        }

        void comment(const string_view comment, State & /*state*/)
        {
            _comment = comment;
        }

      private:

        string _key{};
        string _comment{};
    };

    inline Encoder & operator<<(Encoder & encoder, const Value & val)
    {
        if (val.hasComment() && encoder.container() != Container::object)
        {
            encoder << comment(*val.comment());
        }

        switch (val.type())
        {
            case Type::null:
            {
                encoder << nullptr;
                break;
            }
            case Type::object:
            {
                encoder << object(val.density());
                for (const auto & [key, v] : *val._object())
                {
                    if (v.hasComment())
                    {
                        encoder << comment(*v.comment());
                    }
                    encoder << key << v;
                }
                encoder << end;
                break;
            }
            case Type::array:
            {
                encoder << array(val.density());
                for (const auto & v : *val._array())
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

    inline Value::Value(Object && val, const Density density) :
        _ptrAndDensity{std::bit_cast<uintptr_t>(new Object{std::move(val)}) | uintptr_t(density)},
        _typeAndComment{uintptr_t(Type::object)}
    {}

    inline Value::Value(Array && val, const Density density) :
        _ptrAndDensity{std::bit_cast<uintptr_t>(new Array{std::move(val)}) | uintptr_t(density)},
        _typeAndComment{uintptr_t(Type::array)}
    {}

    inline Value::Value(string && val) :
        _string{new string{std::move(val)}},
        _typeAndComment{uintptr_t(Type::string)}
    {}

    inline Value::Value(const string_view val) :
        _string{new string{val}},
        _typeAndComment{uintptr_t(Type::string)}
    {}

    inline Value::Value(const char * const val) :
        Value(string_view{val})
    {}

    inline Value::Value(char * const val) :
        Value(string_view{val})
    {}

    inline Value::Value(const char val) :
        Value{string_view{&val, 1u}}
    {}

    inline Value::Value(const int64_t val) :
        _integer{val},
        _typeAndComment{uintptr_t(Type::integer)}
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
        _typeAndComment{uintptr_t(Type::unsigner)}
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
        _typeAndComment{uintptr_t(Type::floater)}
    {}

    inline Value::Value(const float val) :
        Value{double{val}}
    {}

    inline Value::Value(const bool val) :
        _boolean{val},
        _typeAndComment{uintptr_t(Type::boolean)}
    {}

    template <ValueFromAble T>
    inline Value::Value(const T & val) :
        Value{::qcon::ValueFrom<T>{}(val)}
    {}

    inline Value::Value(Value && other) :
        _unsigner{std::exchange(other._unsigner, 0u)},
        _typeAndComment{std::exchange(other._typeAndComment, 0u)}
    {}

    inline Value & Value::operator=(Object && val)
    {
        if (type() == Type::object)
        {
            *_object() = std::move(val);
        }
        else
        {
            _deleteValue();
            _setType(Type::object);
            _ptrAndDensity = std::bit_cast<uintptr_t>(new Object{std::move(val)});
        }
        return *this;
    }

    inline Value & Value::operator=(Array && val)
    {
        if (type() == Type::array)
        {
            *_array() = std::move(val);
        }
        else
        {
            _deleteValue();
            _setType(Type::array);
            _ptrAndDensity = std::bit_cast<uintptr_t>(new Array{std::move(val)});
        }
        return *this;
    }

    inline Value & Value::operator=(string && val)
    {
        if (type() == Type::string)
        {
            *_string = std::move(val);
        }
        else
        {
            _deleteValue();
            _setType(Type::string);
            _string = new string{std::move(val)};
        }
        return *this;
    }

    inline Value & Value::operator=(const string_view val)
    {
        if (type() == Type::string)
        {
            *_string = val;
        }
        else
        {
            _deleteValue();
            _setType(Type::string);
            _string = new string{val};
        }
        return *this;
    }

    inline Value & Value::operator=(const char * const val)
    {
        return *this = string_view{val};
    }

    inline Value & Value::operator=(const char val)
    {
        return *this = string_view{&val, 1u};
    }

    inline Value & Value::operator=(const int64_t val)
    {
        if (type() != Type::integer)
        {
            _deleteValue();
            _setType(Type::integer);
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
        if (type() != Type::unsigner)
        {
            _deleteValue();
            _setType(Type::unsigner);
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
        if (type() != Type::floater)
        {
            _deleteValue();
            _setType(Type::floater);
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
        if (type() != Type::boolean)
        {
            _deleteValue();
            _setType(Type::boolean);
        }
        _boolean = val;
        return *this;
    }

    inline Value & Value::operator=(const nullptr_t)
    {
        if (type() != Type::null)
        {
            _deleteValue();
            _setType(Type::null);
        }
        _null = nullptr;
        return *this;
    }

    inline Value & Value::operator=(Value && other)
    {
        _deleteValue();
        _deleteComment();
        _unsigner = std::exchange(other._unsigner, 0u);
        _typeAndComment = std::exchange(other._typeAndComment, 0u);
        return *this;
    }

    inline Value::~Value()
    {
        _deleteValue();
        _deleteComment();
    }

    inline Type Value::type() const
    {
        return Type(_typeAndComment & 0b111u);
    }

    inline Density Value::density() const
    {
        const Type type{this->type()};
        if (type == Type::object || type == Type::array)
        {
            return Density(_ptrAndDensity & 0b111u);
        }
        else
        {
            return Density::unspecified;
        }
    }

    inline void Value::setDensity(const Density density)
    {
        const Type type{this->type()};
        if (type == Type::object || type == Type::array)
        {
            _ptrAndDensity &= ~uintptr_t{0b111u};
            _ptrAndDensity |= uintptr_t(density);
        }
    }

    inline bool Value::isObject() const
    {
        return type() == Type::object;
    }

    inline bool Value::isArray() const
    {
        return type() == Type::array;
    }

    inline bool Value::isString() const
    {
        return type() == Type::string;
    }

    inline bool Value::isNumber() const
    {
        const Type type{this->type()};
        return type == Type::integer || type == Type::unsigner || type == Type::floater;
    }

    inline bool Value::isInteger() const
    {
        return type() == Type::integer;
    }

    inline bool Value::isUnsigner() const
    {
        return type() == Type::unsigner;
    }

    inline bool Value::isFloater() const
    {
        return type() == Type::floater;
    }

    inline bool Value::isBoolean() const
    {
        return type() == Type::boolean;
    }

    inline bool Value::isNull() const
    {
        return type() == Type::null;
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
        else if constexpr (std::is_same_v<U, string> || std::is_same_v<U, string_view> || std::is_same_v<U, const char *> || std::is_same_v<U, char *>)
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
            switch (type())
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
            switch (type())
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
            return isNumber();
        }
        // Other
        else
        {
            return false;
        }
    }

    inline Object * Value::asObject()
    {
        return isObject() ? _object() : nullptr;
    }

    inline const Object * Value::asObject() const
    {
        return isObject() ? _object() : nullptr;
    }

    inline Array * Value::asArray()
    {
        return isArray() ? _array() : nullptr;
    }

    inline const Array * Value::asArray() const
    {
        return isArray() ? _array() : nullptr;
    }

    inline string * Value::asString()
    {
        return isString() ? _string : nullptr;
    }

    inline const string * Value::asString() const
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
        if constexpr (std::is_same_v<U, string> || std::is_same_v<U, string_view>)
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
                switch (type())
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

    inline bool Value::hasComment() const
    {
        return comment();
    }

    inline string * Value::comment()
    {
        return std::bit_cast<string *>(_typeAndComment & ~uintptr_t{0b111u});
    }

    inline const string * Value::comment() const
    {
        return std::bit_cast<const string *>(_typeAndComment & ~uintptr_t{0b111u});
    }

    inline void Value::setComment(string && str)
    {
        _setComment(std::move(str));
    }

    inline void Value::setComment(const string_view str)
    {
        _setComment(str);
    }

    inline void Value::setComment(const char * str)
    {
        _setComment(str);
    }

    template <typename T>
    inline void Value::_setComment(T && str)
    {
        string * const comment{this->comment()};
        if (comment)
        {
            *comment = std::forward<T>(str);
        }
        else
        {
            _typeAndComment &= 0b111u;
            _typeAndComment |= std::bit_cast<uintptr_t>(new string{std::move(str)});
        }
    }

    inline std::unique_ptr<string> Value::removeComment()
    {
        string * const comment{this->comment()};
        _typeAndComment &= 0b111u;
        return std::unique_ptr<string>{comment};
    }

    inline bool Value::operator==(const Value & other) const
    {
        switch (other.type())
        {
            case Type::null: return *this == other._null;
            case Type::object: return *this == *other._object();
            case Type::array: return *this == *other._array();
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
        return type() == Type::object && *_object() == val;
    }

    inline bool Value::operator==(const Array & val) const
    {
        return type() == Type::array && *_array() == val;
    }

    inline bool Value::operator==(const string & val) const
    {
        return *this == string_view{val};
    }

    inline bool Value::operator==(const string_view val) const
    {
        return type() == Type::string && *_string == val;
    }

    inline bool Value::operator==(const char * const val) const
    {
        return *this == string_view{val};
    }

    inline bool Value::operator==(const char val) const
    {
        return *this == string_view{&val, 1u};
    }

    inline bool Value::operator==(const int64_t val) const
    {
        switch (type())
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
        switch (type())
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
        switch (type())
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
        return type() == Type::boolean && _boolean == val;
    }

    inline bool Value::operator==(const nullptr_t) const
    {
        return type() == Type::null;
    }

    inline Object * Value::_object()
    {
        return std::bit_cast<Object *>(_ptrAndDensity & ~uintptr_t{0b111u});
    }

    inline const Object * Value::_object() const
    {
        return std::bit_cast<const Object *>(_ptrAndDensity & ~uintptr_t{0b111u});
    }

    inline Array * Value::_array()
    {
        return std::bit_cast<Array *>(_ptrAndDensity & ~uintptr_t{0b111u});
    }

    inline const Array * Value::_array() const
    {
        return std::bit_cast<const Array *>(_ptrAndDensity & ~uintptr_t{0b111u});
    }

    inline void Value::_setType(const Type type)
    {
        _typeAndComment &= ~uintptr_t{0b111u};
        _typeAndComment |= uintptr_t(type);
    }

    inline void Value::_deleteValue()
    {
        switch (type())
        {
            case Type::object: delete _object(); break;
            case Type::array: delete _array(); break;
            case Type::string: delete _string; break;
            default: break;
        }
    }

    inline void Value::_deleteComment()
    {
        string * const comment{this->comment()};
        if (comment)
        {
            delete comment;
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

    inline std::optional<Value> decode(const string_view qcon)
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

    inline std::optional<string> encode(const Value & val, const Density density, size_t indentSpaces, bool singleQuotes, bool identifiers)
    {
        Encoder encoder{density, indentSpaces, singleQuotes, identifiers};
        encoder << val;
        return encoder.finish();
    }
}
