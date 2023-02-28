# QCON Grammar

**Qcon:**  
    X Value X

**Value:**  
    Object  
    Array  
    String  
    Integer  
    Floater  
    Boolean  
    Date  
    Time  
    Datetime  
    `null`

**Object:**  
    `{` X `}`  
    `{` X Members X `}`  

**Members:**  
    Member  
    Member X `,`  
    Member X `,` X Members

**Member:**  
    String X `:` X Value

**Array:**  
    `[` X `]`  
    `[` X Elements X `]`  

**Elements:**  
    Value  
    Value X `,`  
    Value X `,` X Elements

**String:**  
    `""`  
    `"` Characters `"`  
    String X String

**Characters:**  
    Character  
    Character Characters

**Character:**  
    U+0020 . U+10FFFF - `"` - `\​`  
    `\​` Escape

**Escape:**  
    `"`  
    `/`  
    `\​`  
    `0`  
    `a`  
    `b`  
    `t`  
    `n`  
    `v`  
    `f`  
    `r`  
    `x` HexDigit HexDigit  
    `u` HexDigit HexDigit HexDigit HexDigit  
    `U` HexDigit HexDigit HexDigit HexDigit HexDigit HexDigit HexDigit HexDigit

**Integer:**  
    DecimalInteger  
    BinaryInteger  
    OctalInteger  
    HexInteger

**DecimalInteger:**  
    DecimalDigits

**HexInteger:**  
    `0x` HexDigits

**OctalInteger:**  
    `0o` OctalDigits

**BinaryInteger:**  
    `0b` BinaryDigits

**DecimalDigits:**  
    DecimalDigit  
    DecimalDigit DecimalDigits

**HexDigits:**  
    HexDigit  
    HexDigit HexDigits

**OctalDigits:**  
    OctalDigit  
    OctalDigit OctalDigits

**BinaryDigits:**  
    BinaryDigit  
    BinaryDigit BinaryDigits

**DecimalDigit:**  
    `0` . `9`

**BinaryDigit:**  
    `0` . `1`  

**OctalDigit:**  
    `0` . `7`

**HexDigit:**  
    `0` . `9`  
    `A` . `F`  
    `a` . `f`

**Floater:**  
    DecimalDigits Fraction  
    DecimalDigits Exponent  
    DecimalDigits Fraction Exponent

**Fraction:**  
    `.` DecimalDigits

**Exponent:**  
    `e` DecimalDigits  
    `E` DecimalDigits  
    `e` Sign DecimalDigits  
    `E` Sign DecimalDigits

**Sign:**  
    `+`  
    `-`

**Boolean:**  
    `true`  
    `false`

**Date:**  
    `D` YearMonthDay

**YearMonthDay:**  
    Year `-` Month `-` Day

**Year:**  
    `0000` . `9999`

**Month:**  
    `1` . `12`

**Day:**  
    `1` . `31`

**Time:**  
    `T` HourMinuteSecond  
    `T` HourMinuteSecond Fraction  
    `T` HourMinuteSecond TimeZone  
    `T` HourMinuteSecond Fraction TimeZone

**HourMinuteSecond:**  
    Hour `:` Minute `:` Second

**Hour:**  
    `00` . `23`

**Minute:**  
    `00` . `59`

**Second:**  
    `00` . `59`

**TimeZone:**  
    `Z`  
    Sign Hour `:` Minute

**Datetime:**  
    Date Time

**X:**  
    *nothing*  
    Space  
    Comment  
    X X

**Space:**  
    U+0009  
    U+000A  
    U+000D  
    U+0020  

**Comment:**  
    `#` *anything* U+000A
