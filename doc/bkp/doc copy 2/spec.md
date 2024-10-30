# Specification

bkbkbbbkb

## 1. Identifier

Identifiers start with an alphabetic character or the symbol `_` followed by any number of alphabetic characters, `_` or digits ([0-9]).
zscript is a case sensitive language meaning that the lowercase and uppercase representation of the same alphabetic character are considered different characters.
For instance, “foo”, “Foo” and “fOo” are treated as 3 distinct identifiers.

> [!NOTE]
> Highlights information that users should take into account, even when skimming.

> [!TIP]
> Optional information to help a user be more successful.

> [!IMPORTANT]
> Crucial information necessary for users to succeed.

> [!WARNING]
> Critical content demanding immediate user attention due to potential risks.

> [!CAUTION]
> Negative potential consequences of an action.



## 2. Keywords

The following words are reserved and cannot be used as identifiers:
||||||
|-|-|-|-|-|
|`none`|`null`|`var`|`int`|`float`|
|`bool`|`table`|`array`|`string`|`char`|
|`auto`|`true`|`false`|`return`|`extend`|
|`and`|`or`|`if`|`else`|`for`|
|`do`|`while`|`switch`|`case`|`break`|
|`default`|`continue`|`try`|`catch`|`throw`|
|`this`|`base`|`class`|`namespace`|`global`|
|`const`|`static`|`function`|`typeof`|`typeid`|
|`constructor`|`destructor`|`in`|`enum`|`not`|
|`include`|`import`|&nbsp;|&nbsp;|&nbsp;|

## 3. Comments

A comment is text that the compiler ignores but that is useful for programmers. Comments are normally used to embed annotations in the code. The compiler treats them as white space.

#### 3.1 Multi-line comment  <sub><sup>[comments-001]</sup></sub>

A comment can be `/*` (slash, asterisk) characters, followed by any sequence of characters (including new lines), followed by the `*/` characters.
  This syntax is the same as ANSI C.

```zscript
/*
 * This is a multi-line comment.
 * Keeps going.
 */
```

#### 3.2 Multi-line comment  <sub><sup>[comments-002]</sup></sub>

```zscript
/*
   This is a multi-line comment.
   Keeps going.
*/
```

#### 3.3 Inplace comment  <sub><sup>[comments-003]</sup></sub>

```zscript
var a = /* comment */ 55;
```

#### 3.4 Single line comment  <sub><sup>[comments-004]</sup></sub>

A comment can also be `//` (two slashes) characters, followed by any sequence of characters.

```zscript
// This is a single line comment.
```

## 4. Variable Declaration

The zscript language is an hybrid between **weakly** and **strongly** typed.

Basic types are `null`, `bool`, `integer`, `float`, `string`, `table`, `array`, `function`, `class`, `instance` and `userdata`.

A variable declared with `var` is **weakly** typed and can be assigned or reassigned any type.


### 4.1. Empty

jkjkjjlkjlk

#### 4.1.1 Empty variable declaration  <sub><sup>[var-decl-001]</sup></sub>

`a` is `null`.

```zscript
var a;
```

#### 4.1.2 Empty variable declaration (without *;*)  <sub><sup>[var-decl-002]</sup></sub>

`a` is `null`.

```zscript
var a
```

### 4.2. Null

jkjkjjlkjlk

#### 4.2.1 Null variable declaration  <sub><sup>[var-decl-003]</sup></sub>

`a` is `null`.

```zscript
var a = null;
```

#### 4.2.2 Null variable declaration  <sub><sup>[var-decl-004]</sup></sub>

`a` is `null`.

```zscript
var abcdefghijklmnopqrstuvwxyz = 12;
```

### 4.3. Integer

- decimal-constant is a non-zero decimal digit (1, 2, 3, 4, 5, 6, 7, 8, 9), followed by zero or more decimal digits (0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
- hex-constant is the character sequence 0x or the character sequence 0X followed by one or more hexadecimal digits (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, A, b, B, c, C, d, D, e, E, f, F)
- binary-constant is the character sequence 0b or the character sequence 0B followed by one or more binary digits (0, 1)
- octal-constant is the character sequence 0h followed by zero or more octal digits (0, 1, 2, 3, 4, 5, 6, 7)

#### 4.3.1 Integer variable declaration  <sub><sup>[var-decl-005]</sup></sub>

```zscript
var a = 56;
```

#### 4.3.2 Integer variable declaration  <sub><sup>[var-decl-006]</sup></sub>

```zscript
const  int a = 56;
```

#### 4.3.3 Integer variable declaration  <sub><sup>[var-decl-007]</sup></sub>

```zscript
var a = -89;
```

#### 4.3.4 Integer variable declaration  <sub><sup>[var-decl-008]</sup></sub>

Hexadecimal.

```zscript
var a = 0xFFAC;
```

#### 4.3.5 Integer variable declaration  <sub><sup>[var-decl-009]</sup></sub>

Hexadecimal.

```zscript
var a = 0XFFAC;
```

#### 4.3.6 Integer variable declaration  <sub><sup>[var-decl-010]</sup></sub>

Hexadecimal.

```zscript
var a = 0xffa08;
```

#### 4.3.7 Integer variable declaration  <sub><sup>[var-decl-011]</sup></sub>

```zscript
var a = 0x1e5;
```

#### 4.3.8 Integer variable declaration  <sub><sup>[var-decl-012]</sup></sub>

Octal number.

```zscript
var a = 0h567;
```

#### 4.3.9 Integer variable declaration  <sub><sup>[var-decl-013]</sup></sub>

Binary.

```zscript
var a = 0b01001;
```

### 4.4. Float

jkjkjjlkjlk

#### 4.4.1 Float variable declaration  <sub><sup>[var-decl-014]</sup></sub>

```zscript
var a = 56.6;
```

#### 4.4.2 Float variable declaration  <sub><sup>[var-decl-015]</sup></sub>

```zscript
var a = 6.;
```

#### 4.4.3 Float variable declaration  <sub><sup>[var-decl-016]</sup></sub>

```zscript
var a = .8;
```

#### 4.4.4 Float variable declaration  <sub><sup>[var-decl-017]</sup></sub>

```zscript
var a = 4e2;
```

#### 4.4.5 Float variable declaration  <sub><sup>[var-decl-018]</sup></sub>

```zscript
var a = 123.456e-67;
```

#### 4.4.6 Float variable declaration  <sub><sup>[var-decl-019]</sup></sub>

```zscript
var a = .1E4;
```

### 4.5. Boolean

jkjkjjlkjlk

#### 4.5.1 Boolean variable declaration  <sub><sup>[var-decl-020]</sup></sub>

```zscript
var a = true;
```

#### 4.5.2 Boolean variable declaration  <sub><sup>[var-decl-021]</sup></sub>

```zscript
var a = false;
```

### 4.6. Char

jkjkjjlkjlk

#### 4.6.1 Character variable declaration  <sub><sup>[var-decl-022]</sup></sub>

```zscript
var a = 'A';
```

#### 4.6.2 Character variable declaration  <sub><sup>[var-decl-023]</sup></sub>

utf8

```zscript
var a = 'π';
```

### 4.7. String

jkjkjjlkjlk

#### 4.7.1 String variable declaration  <sub><sup>[var-decl-024]</sup></sub>

```zscript
var a = "abc";
```

#### 4.7.2 String variable declaration  <sub><sup>[var-decl-025]</sup></sub>

```zscript
var a = "π";
```

#### 4.7.3 Multiline string variable declaration  <sub><sup>[var-decl-026]</sup></sub>

```zscript
var a = """abc""";
```

#### 4.7.4 Multiline string variable declaration  <sub><sup>[var-decl-027]</sup></sub>

```zscript
var a = '''abc''';
```

### 4.8. Array

jkjkjjlkjlk

#### 4.8.1 Array variable declaration  <sub><sup>[var-decl-028]</sup></sub>

```zscript
var a = [];
```

#### 4.8.2 Array variable declaration  <sub><sup>[var-decl-029]</sup></sub>

```zscript
var a = [1, 2.89, "abc", true];
```

### 4.9. Table

jkjkjjlkjlk

#### 4.9.1 Table variable declaration  <sub><sup>[var-decl-030]</sup></sub>

```zscript
var a = {};
```

#### 4.9.2 Table variable declaration  <sub><sup>[var-decl-031]</sup></sub>

```zscript
var a = {
  a = 78,
  b = 7.9
};
```

#### 4.9.3 Table variable declaration  <sub><sup>[var-decl-032]</sup></sub>

The comma is optional.

```zscript
var a = {
  a = 78
  b = 7.9
};
```

#### 4.9.4 Table variable declaration  <sub><sup>[var-decl-033]</sup></sub>

json style.

```zscript
var a = {
  "a": 78,
  "b": 7.9
};
```

#### 4.9.5 Table variable declaration  <sub><sup>[var-decl-034]</sup></sub>

Bracket.

```zscript
var a = {
  ["a a"] = 78,
  ["a-a"] = 79
};
```

#### 4.9.6 Table variable declaration  <sub><sup>[var-decl-035]</sup></sub>

Mix decl.

```zscript
var a = {
  a = 89,
  ["a a"] = 78,
  ["a-a"] = 79,
  "b": 7.9
};
```

### 4.10. Multi

jkjkjjlkjlk

#### 4.10.1 Multiple variable declaration on a single line  <sub><sup>[var-decl-036]</sup></sub>

Mix decl.

```zscript
var a = 32, b = 55, c = "abc";
```

#### 4.10.2 Table variable declaration  <sub><sup>[var-decl-037]</sup></sub>

```zscript
var v1 = 5;
var v2 = 6;
var a = v1 == v2;
```

#### 4.10.3 Const  <sub><sup>[var-decl-038]</sup></sub>

```zscript
const var a = 44;
a = 34;
```

#### 4.10.4 Table variable declaration  <sub><sup>[var-decl-039]</sup></sub>

```zscript
var v1 = 5;
var v2 = 5;
var a = v1 == v2;
```

#### 4.10.5 as_string  <sub><sup>[var-decl-040]</sup></sub>

```zscript
var a = #as_string("/Users/alexarse/Develop/zscript/zscript/tests/unit_tests/resources/data/text_01.txt");
```

#### 4.10.6 as_table  <sub><sup>[var-decl-041]</sup></sub>

```zscript
var a = #as_table("/Users/alexarse/Develop/zscript/zscript/tests/unit_tests/resources/data/obj_01.json");
```

#### 4.10.7 define  <sub><sup>[var-decl-042]</sup></sub>

```zscript
#define k_my_value = 54;
var a = @@k_my_value;
```

#### 4.10.8 define  <sub><sup>[var-decl-043]</sup></sub>

```zscript
#define k_my_value = 54;
var a = @@k_my_value;
```

#### 4.10.9 blabalbal  <sub><sup>[var-decl-044]</sup></sub>

```zscript
var a = #load_json_file("/Users/alexarse/Develop/zscript/zscript/tests/unit_tests/resources/data/obj_01.json");
```

## 5. Typed Variable Declaration

The type restriction gets lost when it is assigned to an **array**, a **table** or passed as a function parameter.

### 5.1. Integer

jkjkjjlkjlk

#### 5.1.1 Empty variable declaration  <sub><sup>[typed-var-decl-001]</sup></sub>

`a` is `null`.

```zscript
int a;
```

#### 5.1.2 Integer variable declaration  <sub><sup>[typed-var-decl-002]</sup></sub>

```zscript
int a = 21;
```

#### 5.1.3 Empty variable declaration  <sub><sup>[typed-var-decl-003]</sup></sub>

```zscript
int a;
a = 78;
```

#### 5.1.4 Wrong variable declaration  <sub><sup>[typed-var-decl-004]</sup></sub>

🔴 <span style="color:#DE5030">**Runtime Error**</span> `b` is not an integer.

```zscript
int a;
var b = 32.2;
a = b; // Error 'b' is not an integer.
```

#### 5.1.5 Wrong type variable declaration  <sub><sup>[typed-var-decl-005]</sup></sub>

🔴 <span style="color:#DE5030">**Compiler Error**</span> not an integer.

```zscript
int a = 33.2; // Error not an integer.
```

#### 5.1.6 Empty variable declaration  <sub><sup>[typed-var-decl-006]</sup></sub>

🔴 <span style="color:#DE5030">**Compiler Error**</span> not an integer.

```zscript
int a;
a = 78.8; // Error not an integer.
```

### 5.2. Float

jkjkjjlkjlk

#### 5.2.1 Float variable declaration  <sub><sup>[typed-var-decl-007]</sup></sub>

```zscript
float a = 21.2;
```

#### 5.2.2 Float variable declaration  <sub><sup>[typed-var-decl-008]</sup></sub>

<span style="color:#DE5030">**Compiler Error**</span> not a float.

```zscript
float a = 21; // Error not a float.
```

### 5.3. Boolean

jkjkjjlkjlk

#### 5.3.1 Boolean variable declaration  <sub><sup>[typed-var-decl-009]</sup></sub>

```zscript
bool a = false;
```

#### 5.3.2 Boolean variable declaration  <sub><sup>[typed-var-decl-010]</sup></sub>

```zscript
bool a = true;
```

### 5.4. Character

jkjkjjlkjlk

#### 5.4.1 Character variable declaration  <sub><sup>[typed-var-decl-011]</sup></sub>

```zscript
char a = 'A';
```

### 5.5. String

jkjkjjlkjlk

#### 5.5.1 String variable declaration  <sub><sup>[typed-var-decl-012]</sup></sub>

```zscript
string a = "abc";
```

### 5.6. Array

jkjkjjlkjlk

#### 5.6.1 Array variable declaration  <sub><sup>[typed-var-decl-013]</sup></sub>

```zscript
array a = [];
```

### 5.7. Table

jkjkjjlkjlk

#### 5.7.1 Table variable declaration  <sub><sup>[typed-var-decl-014]</sup></sub>

```zscript
table a = {};
```

### 5.8. Multiple

jkjkjjlkjlk

#### 5.8.1 Typed variable declaration  <sub><sup>[typed-var-decl-015]</sup></sub>

Same as `int a = 32;`.

```zscript
var<int> a = 32;
```

#### 5.8.2 Empty variable declaration  <sub><sup>[typed-var-decl-016]</sup></sub>

`a` is `null`.

```zscript
var<int> a = 88;
a = 78.6;
```

#### 5.8.3 Empty variable declaration  <sub><sup>[typed-var-decl-017]</sup></sub>

`a` is `null`.

```zscript
var<int, float> a = 88;
a = 78.6;
```

#### 5.8.4 Typed variable declaration  <sub><sup>[typed-var-decl-018]</sup></sub>

Same as `float a = 32.2;`.

```zscript
var<float> a = 32.2;
```

#### 5.8.5 Typed variable declaration  <sub><sup>[typed-var-decl-019]</sup></sub>

Same as `bool a = true;`.

```zscript
var<bool> a = true;
```

#### 5.8.6 Typed variable declaration  <sub><sup>[typed-var-decl-020]</sup></sub>

Same as `char a = 'A';`.

```zscript
var<char> a = 'A';
```

#### 5.8.7 Typed variable declaration  <sub><sup>[typed-var-decl-021]</sup></sub>

Same as `string a = "abc";`.

```zscript
var<string> a = "abc";
```

#### 5.8.8 Typed variable declaration  <sub><sup>[typed-var-decl-022]</sup></sub>

Same as `array a = [];`.

```zscript
var<array> a = [];
```

#### 5.8.9 Typed variable declaration  <sub><sup>[typed-var-decl-023]</sup></sub>

Same as `table a = {};`.

```zscript
var<table> a = {};
```

#### 5.8.10 Integer variable declaration  <sub><sup>[typed-var-decl-024]</sup></sub>

`a` is `null`.

```zscript
var<int> a = 88;
var b = 77;
a = b;
```

#### 5.8.11 Integer variable declaration  <sub><sup>[typed-var-decl-025]</sup></sub>

```zscript
var<int> a = 88;
var b = 77.7;
a = b;
```

#### 5.8.12 Integer or float variable declaration  <sub><sup>[typed-var-decl-026]</sup></sub>

```zscript
var<int, float, string> a = 88;
var b = 77.9;
a = b;
a = false;
```

#### 5.8.13 Integer or float variable declaration  <sub><sup>[typed-var-decl-027]</sup></sub>

```zscript
var<int, float> a = 177;
```

#### 5.8.14 Integer or float variable declaration  <sub><sup>[typed-var-decl-028]</sup></sub>

```zscript
var<int> a = 177.8;
```

#### 5.8.15 Integer or float variable declaration  <sub><sup>[typed-var-decl-029]</sup></sub>

```zscript
var<int, float> a = 177.8;
a = false;
```

#### 5.8.16 Typed variable declaration  <sub><sup>[typed-var-decl-030]</sup></sub>

```zscript
var<int, float> a = 32.2;
```

#### 5.8.17 Typed variable declaration  <sub><sup>[typed-var-decl-031]</sup></sub>

```zscript
var<string, float> a = "bacon";
```

#### 5.8.18 Multiple variable declaration on a single line  <sub><sup>[typed-var-decl-032]</sup></sub>

Mix decl.

```zscript
int a = 32, b = 55, c = 324;
```

#### 5.8.19 Multiple variable declaration on a single line  <sub><sup>[typed-var-decl-033]</sup></sub>

Mix decl.

```zscript
var<int, float> a = 32, b = 55.5, c = 324;
```

#### 5.8.20 Multiple variable declaration on a single line  <sub><sup>[typed-var-decl-034]</sup></sub>

Mix decl.

```zscript
var<int, MyClass> a = 32;
```

#### 5.8.21 Multiple variable declaration on a single line  <sub><sup>[typed-var-decl-035]</sup></sub>

Mix decl.

```zscript
var<MyClass> a;
```

#### 5.8.22 Multiple variable declaration on a single line  <sub><sup>[typed-var-decl-036]</sup></sub>

Mix decl.

```zscript
var<int> a = 55, b = 45;
```

#### 5.8.23 Multiple variable declaration on a single line  <sub><sup>[typed-var-decl-037]</sup></sub>

🔴 <span style="color:#DE5030">**Compiler Error**</span> `b` is not an integer or a float.

```zscript
var<int, float> a = 55, b = "abc";
```

## 6. Function Declaration

hjhkj

#### 6.1 bkbbkbkb  <sub><sup>[function-decl-001]</sup></sub>

```zscript
int a = 56;
```

#### 6.2 Function declaration  <sub><sup>[function-decl-002]</sup></sub>

```zscript
var f = function(k) {
  return k;
};

var a = f(32);
```

#### 6.3 Function declaration  <sub><sup>[function-decl-003]</sup></sub>

```zscript
var f = function(int k) {
  return k;
};

var a = f(32);
```

#### 6.4 Function declaration  <sub><sup>[function-decl-004]</sup></sub>

```zscript
var f = function(const int k) {
  return k;
};

var a = f(32);
```

#### 6.5 Function declaration  <sub><sup>[function-decl-005]</sup></sub>

At compile time, the variable assignment of `k` will generate an error.

```zscript
var f = function(const var<int> k) {
  k = 32; // Error: trying to assign to a const value.
  return k;
};

var a = f(32);
```

#### 6.6 Function declaration  <sub><sup>[function-decl-006]</sup></sub>

```zscript
var f = function(const int k) {
  return k;
};

var a = f(32.2);
```

#### 6.7 Function declaration  <sub><sup>[function-decl-007]</sup></sub>

```zscript
var f = function(const int k) {
  return k;
};

var a = f(32);
```

## 7. Compare

hjhkj

#### 7.1 Compare  <sub><sup>[compare-001]</sup></sub>

```zscript
var a = 32;

if(a == 32) {
  a = 33;
}

```

#### 7.2 Compare  <sub><sup>[compare-002]</sup></sub>

```zscript
var a = 32;

if(a == 35) {
  a = 33;
}

```

#### 7.3 Compare  <sub><sup>[compare-003]</sup></sub>

```zscript
var a = 32;

if(a < 35) {
  a = 21;
}

```

#### 7.4 Compare  <sub><sup>[compare-004]</sup></sub>

```zscript
var a = 32;

if(a < 3) {
  a = 21;
}

```

#### 7.5 Compare  <sub><sup>[compare-005]</sup></sub>

```zscript
var a = 32;

if(a > 3) {
  a = 21;
}

```

#### 7.6 Compare  <sub><sup>[compare-006]</sup></sub>

```zscript
var a = 32;

if(a > 35) {
  a = 21;
}

```

#### 7.7 Compare  <sub><sup>[compare-007]</sup></sub>

```zscript
var a = 32;

if(a <= 35) {
  a = 21;
}

```

#### 7.8 Compare  <sub><sup>[compare-008]</sup></sub>

```zscript
var a = 32;

if(a == 32.0) {
  a = 21;
}

```

#### 7.9 Compare  <sub><sup>[compare-009]</sup></sub>

```zscript
var a = 32;

if(a === 32.0) {
  a = 21;
}

```

#### 7.10 Compare  <sub><sup>[compare-010]</sup></sub>

```zscript
var a = 32;

if(a === 32) {
  a = 21;
}

```

#### 7.11 Compare  <sub><sup>[compare-011]</sup></sub>

```zscript
var a = 32;

if(a) {
  a = 21;
}

```

#### 7.12 Compare  <sub><sup>[compare-012]</sup></sub>

```zscript
var a = 0;

if(a) {
  a = 21;
}

```

#### 7.13 Compare  <sub><sup>[compare-013]</sup></sub>

```zscript
var a = 0;

if(!a) {
  a = 21;
}

```

#### 7.14 Compare  <sub><sup>[compare-014]</sup></sub>

```zscript
var a = 32;

if(a <==> 32) {
  a = 21;
}

```

#### 7.15 Compare  <sub><sup>[compare-015]</sup></sub>

```zscript
var a = 32;

if(a <==> 32.0) {
  a = 21;
}

```

#### 7.16 Compare  <sub><sup>[compare-016]</sup></sub>

```zscript
var a = 32;

if(a <--> 32) {
  a = 21;
}

```

#### 7.17 Compare  <sub><sup>[compare-017]</sup></sub>

```zscript
var a = 32;

if(a <--> 32.0) {
  a = 21;
}

```

#### 7.18 Compare  <sub><sup>[compare-018]</sup></sub>

```zscript
var a = "bacon";

if(a == "bacon") {
  a = 21;
}

```

#### 7.19 Compare  <sub><sup>[compare-019]</sup></sub>

```zscript
var a = "bacon";

if(a == "ham") {
  a = 21;
}

```

#### 7.20 Compare  <sub><sup>[compare-020]</sup></sub>

```zscript
var a = "bacon";

if(a != "ham") {
  a = 21;
}

```

#### 7.21 Compare  <sub><sup>[compare-021]</sup></sub>

```zscript
var a = "bacon";

if(a) {
  a = 21;
}

```

#### 7.22 Compare  <sub><sup>[compare-022]</sup></sub>

```zscript
var a = 2;

if(a == 2) {
  a = 21;
}
else {
 a = 23;
}

```

#### 7.23 Compare  <sub><sup>[compare-023]</sup></sub>

```zscript
var a = 2;

if(a == 32) {
  a = 21;
}
else {
 a = 23;
}

```

#### 7.24 If Compare  <sub><sup>[compare-024]</sup></sub>

jgghhjghjghjgjh

```zscript
var a = 2;

if(a == 1) {
  a = 88;
}
else if(a == 3) {
  a = 99;
}
else {
}

```

#### 7.25 Compare  <sub><sup>[compare-025]</sup></sub>

```zscript
 var a = "bacon";

 if(!a) {
  a = 21;
}

```
