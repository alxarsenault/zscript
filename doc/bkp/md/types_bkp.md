# Types

## Variables

The zscript language is an hybrid between **weakly** and **strongly** typed.

Basic types are `null`, `bool`, `integer`, `float`, `string`, `table`, `array`, `function`, `class`, `instance` and `userdata`.

A variable declared with `var` is **weakly** typed and can be assigned or reassigned any type.

```cpp
var a = null;
assert(typeof(a) == "null");

a = 1;
assert(typeof(a) == "integer");

a = "abc";
assert(typeof(a) == "string");
```

### Strongly typed

A variable declared with a type, is **strongly** typed and can only be assigned the same type.

```cpp
int a = 51;
assert(typeof(a) == "integer");

// ERROR - Trying to assign a string value to an integer variable.
a = "abc";
```

Examples:

```js
int v = 1;
float v = 1.1;
bool v = true;
bool v = false;
string v = "abc";
string v = """abc""";
string v = '''abc''';
array v = [];
table v = {};

// TODO: No sure about this yet.
char v = 'a';
```

### Auto

A variable declared with the `auto` keyword is also **strongly** typed but the assigned type is automatically detected.

```cpp
auto a = 31;
assert(typeof(a) == "integer");

// ERROR - Trying to assign a string value to an integer variable.
a = "abc";
```

### Type restriction

It is also possible to restrict a variable assignable types.

```cpp
var<int, float> a = 31;
assert(typeof(a) == "integer");

a = 31.1;
assert(typeof(a) == "float");

// ERROR - Trying to assign a string value to a var<int, float> variable.
a = "abc";
```

The `number` keyword is the equivalent of `var<int, float>`.

```cpp
number a = 31;
assert(typeof(a) == "integer");

a = 31.1;
assert(typeof(a) == "float");

// ERROR - Trying to assign a string value to a var<int, float> variable.
a = "abc";
```

### Const

A variable is by default mutable. The `const` keyword makes it non-mutable.

```js
const var a = 21;

// ERROR - Trying to assign a value to a const variable.
a = 22;
```

Implicit var.

```js
const a = 21;
```

The `const` keyword can prefix any type of variables.

```js
const v = null;
const var v = null;
const var v = 1;
const int v = 1;
const float v = 1.1;
const bool v = true;
const bool v = false;
const string v = "abc";
const string v = """abc""";
const string v = '''abc''';
const array v = [];
const table v = {};
const char v = 'a';
```

A const variable must be declared with a litteral value.

```js
// ERROR - Const variable 'a' is not a litteral.
const var a = 2.2 * sin(1.2);
```

```js
// ERROR - Const variable 'a' is not a litteral.
const var a = get_my_table();
```

### Global variables

```js
// Weakly typed.
global a = 5;
global var a = 5;

// Strongly typed.
global int a = 5;

// Weakly typed const.
const global a = 5;
global const a = 5;

// Strongly typed const.
const global int a = 5;
global const int a = 5;
```

### Global access

```js
var k = global.a;
```

```js
global.a = 5;
global.a = 8;
assert(global.a == 8);
```

## Builtin types

- null
- bool
- char
- integer
- float
- string
- array
- table
- class
- instance (class instance)
- function
- user_data
- raw_pointer
- weak_ref

## How to declare a variable

`TYPE NAME = VALUE;`

A variable is.

```js
var v = null;
var v = 1;
int v = 1;
float v = 1.1;
bool v = true;
bool v = false;
string v = "abc";
string v = """abc""";
string v = '''abc''';
array v = [];
table v = {};
char v = 'a';
```

## Auto

```cpp
auto v = 2.2;
```

## Variable attribute

```js
const v = 1;
const int v = 1;
const var v = 1;
const auto v = 1;

global v = 1;
global int v = 1;
global var v = 1;
global auto v = 1;

@v = 1;
int @v = 1;
var @v = 1;
auto @v = 1;

global const v = 1;
global const int v = 1;
global const var v = 1;
global const auto v = 1;

const @v = 1;
const int @v = 1;
const var @v = 1;
const auto @v = 1;
```

## Variable declaration

```js
local a = 1;                         // Integer.
local b = 1.0;                       // Float.
local c = false;                     // Bool.
local d = true;                      // Bool.
local e = {};                        // Table.
local f = [];                        // Array.
local g = "abc";                     // String.
local h = """abc""";                 // Multi-line string.
local i = '''abc''';                 // Multi-line string.
local j = 'a';                       // Char.
local k = null;                      // Null.
local l = function(x) { return x; }; // Function.
local m = $(x) { return x; };        // Lamda function.
local n = $212387832718921387283721; // Big integer.
```

> **TODO :** How to declare big int?

## Typed variable declaration

```js
int a = 1;            // Integer.
float b = 1.0;        // Float.
number c = 12;        // Number.
bool d = false;       // Bool.
bool e = true;        // Bool.
table f = {};         // Table.
array g = [];         // Array.
string h = "abc";     // String.
string i = """abc"""; // Multi-line string.
string j = '''abc'''; // Multi-line string.
char k = 'k';         // Char.
var l;                // Anything.
```

## Array declaration

```lua
local a = [1, 2, 3];
local b = [1, "abc"];
```

## Table declaration

```lua
local a = {
    k1 = 1,
    k2 = 2.2,
    k3 = "abc"
};

local b = {
    "k1": 1,
    "k2": 2.2,
    "k3": "abc"
};
```

## Macros

```js
#macro banana() {

}
```

## Comments

```js
// This is a comment.

/*
 * This is a multi-line comment.
 */
```

## Global variable

```lua
@a = "abc";
glob b = "def";
```

## Constant variable

```lua
const a = "abc";
```

## NOT SURE

### Safe assignment

```js
var t = {
    a = 1
};

// Valid.
t.b = 41;

// ERROR - No value 'c' was found in table t.
t.c := 42;
```