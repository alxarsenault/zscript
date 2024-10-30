# Function

## Function declaration

### Local function

```js
function my_function() {};
```

```js
local my_function = function() {
    return 0;
};
```

```js
local function my_function() {
    return 0;
};
```

### Table function declaration

```js
local t = {
    my_function = function() {
        return 0;
    }
};
```

```js
local t = {
    function my_function() {
        return 0;
    }
};
```

```js
local t = {};
t.my_function = function() {
    return 0;
};
```

```js
local t = {};
function t.my_function() {
    return 0;
};
```

## Function parameters

```js
function my_function(a, b) {
    return a + b;
};
```

## Function default named parameters

```js
function my_function(a, b = 1, c = 2) {
    return a + b + c;
};
```

## Function typed parameters

```js
function my_function(int a, float b) {
    return a + b;
};
```

```js
function my_function(int a, float b, var c) {
    return a + b + c;
};
```

```js
function my_function(int a, float b, c) {
    return a + b + c;
};
```

```js
function my_function(int a, var<int, float> b) {
    return a + b;
};
```

```js
function my_function(number a, var<number, string> b) {
    return to_string(a) + to_string(b);
};
```

## Variadic function

```js
function my_function(a, ...) {
    return a;
};
```

```js
function my_function(a, args...) {
    return a;
};
```

## Function call

```js
function my_function(a, b) {
    return a + b;
};

my_function(1, 2);
```

### Default named parameters

```js
function my_function(a, b = 1, c = 2) {
    return a + b + c;
};

my_function(1);

my_function(1, 2);

my_function(1, 2, 3);

my_function(1, c = 10);

// BAD (Missing required parameter 'a').
my_function();

// BAD (Missing required parameter 'a').
my_function(b = 1, c = 2);

// BAD (no sequential parameters after named parameters).
my_function(1, c = 10, 33);
```

### Variadic parameters

```js
function my_function(a, args...) {
    return a;
};

my_function(1, 2);
```
