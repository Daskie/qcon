# QCON

QCON is a JSON superset designed to fill in the gaps of base JSON and round out its features to better cover common
serialization needs for low level programming.

### Short Example

```QCON
# Check out this comment
[
    inf,           # Direct representation for +/- infinity
    nan,           # Direct represenation for NaN
    [ 1, 1.0 ],    # Integer and floating-point are distinct types
    0x1A,          # Hex
    0x17,          # Octal
    0b10,          # Binary
    "\x12",        # Two digit unicode codepoint
    "\u1234",      # Four digit unicode codepoint
    "\U12345678",  # Eight digit unicode codepoint
    D1970-01-01,   # Date
    T12:34:56.789, # Time
    D1970-01-01T00:00:00Z, # Datetime
    null,          # Trailing comma allowed
]
```

**See [grammar.md](grammar.md) for a complete description of the QCON syntax.**

### Compared to [JSON](https://www.json.org/)

QCON is a strict superset of JSON, meaning that every valid JSON document is a valid QCON document.

- Adds `# Comments`
- Adds date, time, and datetime types
- Separates the "number" type into two distinct "integer" and "floater" types, distinguished by the presence of a
  decimal point `.` or exponent `e`
- Adds `inf`, `+inf`, and `-inf` to represent floating-point infinity values
- Adds `nan` to represent a floating-point NaN value
- Adds support for hex, octal, and binary integers with the `0x`, `0o`, and `0b` prefixes, respectively
- Allows numbers to begin with `+`
- Allows numbers to have leading zeroes
- Allows the last element in an object or array to have *one* trailing comma
- Adds `\x` and `\U` unicode escape prefixes to support 2 and 8 digit codepoint representations, respectively. This is
  in addition to the 4 digit `\u` prefix available in JSON
- Allows for `"adjacent" "string" "concatenation"`. This is especially useful for splitting
  long strings accross multiple lines
- Adds the `\a` and `\v` escape characters

### Compared to [JSON5](https://json5.org/)

JSON5 is great, but there are a few things I don't like that motivated the creation of QCON.
Most notably, it lacks a datetime type, and some of its features (such as unquoted keys) add considerable parser
complexity/cost without providing meaninful benefit, in my opinion.

*Some minor differences omitted*

- Adds date, time, and datetime types
- Adds octal `0o` and binary `0b` integers
- Uses `inf` instead of `Infinity` and `nan` instead of `NaN`
- Uses `#` for line comments instead of `//`, and does not support `/* block */` comments
- Does not support unquoted keys
- Does not support single quote strings
- Does not support escaped newlines
- Does not support additional whitespace characters

## Types

### Object

An object is a key-value collection of elements and is denoted with `{` braces `}`.
A QCON object is the same as a JSON object, except that the last element may have one trailing comma.

```QCON
{
    "Empty": {},
    "Typical": { "k1": 123, "k2": true, "k3": null },
    "Trailing comma": { "key": "val" },
}
```

### Array

An array is a list of values and is denoted with `[` brackets `]`.
A QCON array is the same as a JSON array, except that the last element may have one trailing comma.

```
[
    [],  # Empty
    [ 123, true, null ],  # Typical
    [ "val" ],  # Trailing comma
]
```

### String

A string is a sequence of unicode codepoints and is denoted with `"` double quotes `"`.

```QCON
"I'm a string made from Úñïçôðè"
```

Adjacent strings are combined as one.

```QCON
"This is a comp" "lete sentence."
```

```QCON
{
    "Script": "Water. Earth. Fire. Air. Long ago, the four nations lived"
              " together in harmony. Then, everything changed when the"
              " Fire Nation attacked. Only the Avatar, master of all four"
              " elements, could stop them, but when the world needed him"
              " most, he vanished."
}
```

Certain special characters can be escaped.

| Sequence | Character | Name            |
|----------|-----------|-----------------|
| `\"`     | "         | Double quote    |
| `\\`     | \         | Back slash      |
| `\/`     | /         | Forward slash   |
| `\0`     | U+0000    | Null            |
| `\a`     | U+0007    | Bell            |
| `\b`     | U+0008    | Backspace       |
| `\t`     | U+0009    | Tab             |
| `\n`     | U+000A    | Newline         |
| `\v`     | U+000B    | Vertical tab    |
| `\f`     | U+000C    | Form feed       |
| `\r`     | U+000D    | Carriage return |

```QCON
"To be\r\nor not\tto be"
```

Any codepoint can be directly encoded using the prefix `\x`, `\u`, or `\U` followed by two, four, or eight hex digits
respectively.

```QCON
"\x12 \u1234 \U12345678"
```

### Integer

An integer is a whole number, without decimal point or exponent.

An integer may be prefixed with a sign `+`/`-`.

An integer may have leading zeroes.

```QCON
[ 123, +123, -123, 000123 ]
```

A hexadecimal, octal, or binary integer may be represented when prefixed with `0x`, `0o`, or `0b` respectively.

```QCON
{
    "hex": 0x0123456789ABCDEF,
    "octal": 0o01234567
    "binary": 0b01
}
```

### Floater

A floater is a number with a decimal point or exponent.

A floater may be prefixed with a sign `+`/`-`.

A floater may have leading zeroes.

```QCON
[ 123.4, +123.4, -123.4, 000123.4, 12.34e2, 12.34e+2, 1234e-2 ]
```

A whole number floater must end in `.0` or contain an exponent to disambiguate it from an integer.

```QCON
[
    7,    # Integer
    7.0,  # Floater
    7e0   # Floater
]
```

Infinity may be represented with `inf`.

NaN may be represented with `nan`.

```QCON
[
     inf,  # Positive infinity
    +inf,  # Positive infinity
    -inf,  # Negative infinity
     nan   # NaN must not have a sign
]
```

### Boolean

A boolean may be either `true` or `false`.

```QCON
{
    "The cake is a lie": true,
    "Take life's lemons": false
}
```

### Date

A date represents a gregorian calendar date in the ISO 8601 format `YYYY-MM-DD`.

A date is prefixed with `D` to streamline parsing.

```QCON
{ "Date of writing": D2023-02-27 }
```

### Time

A time represents a 24-hour time of day in the ISO 8601 format `Thh:mm:ss`.

```QCON
T12:05:33  # Five minutes and 33 seconds past lunch time
```

A time may have any number of subsecond fractional digits.

```QCON
{
    "High precision lunch time": T12:00:00.000000007
}
```

A time may specify a timezone, either with `Z` to indicate UTC time, of a `±hh:mm` offset.

If no timezone is specified, the time is local.

```QCON
{
    "Local": T12:05:33,
    "UTC": T19:05:33Z,
    "Pacific": T12:05:33-07:00,
```

### Datetime

A datetime combines a date and a time into a specific timepoint.

```QCON
D2023-02-27T12:05:33.069-07:00
```

### Null

A null value may be represented with `null`.

```QCON
{ "This statement is false": null }
```
