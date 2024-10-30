# ZSCRIPT

## Content

1. [Introduction](#introduction)
2. [Types](#types)
   - 2.1 A
   - 2.2 B
   - 2.3 C
   - 2.4 D
3. [Functions](#functions)
4. [Functions](#functions)

This is a [link value blabla](function.md)

## 1. Introduction

Blablabala.

## 2. Types

The zscript language is an hybrid between **weakly** and **strongly** typed.

Basic types are `null`, `bool`, `integer`, `float`, `string`, `table`, `array`, `function`, `class`, `instance` and `userdata`.

A variable declared with `var` is **weakly** typed and can be assigned or reassigned any type.

### 2.1 Builtin types

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

## Variables

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

### Type restriction limitations

The type restriction gets lost when it is assigned to an **array**, a **table** or passed as a function parameter.

```js
// Declare a strongly typed integer variable.
int a = 55;

// Declare a table and assign the var a to int.
var t = {
  a = a
};

assert(typeof(t.a) == "integer");

// The variable 'a' is still strongly typed.
// ERROR - Trying to assign a string value to an integer variable.
a = "abc";

// But 't.a' isn't anymore.
// All good, no more restriction.
t.a = "abc";
assert(typeof(t.a) == "string");

var arr = [a];
assert(typeof(arr[0]) == "integer");

// All good, no more restriction.
arr[0] = "abc";
assert(typeof(arr[0]) == "string");

function my_function(v) {
  assert(typeof(v) == "integer");

  // All good, no more restriction.
  v = "abc";
  assert(typeof(v) == "string");
}

my_function(a);
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

## Functions

Functions.
