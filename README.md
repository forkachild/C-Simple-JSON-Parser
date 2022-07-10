# Simple JSON Parser in C

## Features

 * [RFC-8259](https://datatracker.ietf.org/doc/html/rfc8259) compliant
 * Simple interface
   * `json_parse`: Parse a JSON string into a `json_object_t *`
   * `json_print`: Print a `json_object_t *` using specified indentation
   * `json_free`: Free a `json_object_t *` from memory
 * Support for all data types
   * `String`
   * `Number`
   * `Object`
   * `Array`
   * `Boolean`
   * `Null` _(omitted from end result)_
 * Data types:
   * `json_string_t`: The `String` type (alias for `const char *`)
   * `json_number_t`: The `Number` type (alias for `double`)
   * `json_object_t`: The `Object` type
     * `count`: Number of entries (as `size_t`)
     * `entries`: Array of `json_entry_t` of `.count` size
   * `json_array_t`: The heterogenious `Array` type
     * `count`: Number of elements (as `size_t`)
     * `elements`: Array of `json_array_element_t` of `.count` size
   * `json_boolean_t`: The `Boolean` type (alias for `unsigned char`)
   * `json_type_t`: An **enum** of basic JSON type
     * `JSON_TYPE_STRING`
     * `JSON_TYPE_NUMBER`
     * `JSON_TYPE_OBJECT`
     * `JSON_TYPE_ARRAY`
     * `JSON_TYPE_BOOLEAN`
     * `JSON_TYPE_NULL` _(only as an indicator)_
   * `json_value_t`: The JSON value **union** with easy to interpret fields
     * `as_string`: As `json_string_t` value
     * `as_number`: As `json_number_t` value
     * `as_object`: As `json_object_t *` value
     * `as_array`: As `json_array_t *` value
     * `as_boolean`: As `json_boolean_t` value
     * **Note**: The `null` type is not represented
   * `json_entry_t`: The Key-Value entry (used in `json_object_t`)
     * `key`: The key of the entry (as `json_string_t`)
     * `type`: The type of data represented `.value` field (as `json_type_t`)
     * `value`: The value of this entry (as `json_value_t`)
   * `json_array_element_t`: A typed element of an array (used in `json_array_t`)
     * `type`: The type of data represented by `.value` field (as `json_type_t`)
     * `value`: The value of this element (as `json_value_t`)
 * **Value or Error** (Rust like) `result` type used throughout fallible calls
 * Recursive parsing
 * Compile with `-DJSON_SCRAPE_WHITESPACE` to parse non-minified JSON with whitespace in between

## Setup

Copy the following from this repository in your source

 * `json.h`
 * `json_types.h`
 * `json.c`

And in your code

```C
#include "json.h"
```

## Interface

### Parse a JSON String

```C
// `json_string_t` is sugar for `const char *`
json_object_t *json_parse(json_string_t json_str);
```

### Print a JSON Object

```C
void json_print(json_object_t *obj);
```

### Free a JSON Object from memory

```C
void json_free(json_object_t *obj);
```

## Guide

### Single level parsing

```C
#include "json.h"

const char * some_json_string = "{\"hello\":\"world\",\"key\":\"value\"}";

int main() {
  json_object_t *json = json_parse(some_json_string);
  printf("Count: %i\n", json->count);
  printf("Key: %s\nValue: %s", json->entries[0].type, json->entries[0].value.as_string);
  return 0;
}
```
Outputs

```bash
Count: 2
Key: hello
Value: world
```

### Recursive Parsing

```C
#include "json.h"

const char * complex_json = "{\"name\":{\"first\":\"John\",\"last\":\"Doe\"},\"age\":21.5}";

int main() {
  json_object_t *json = json_parse(complex_json);
  json_object_t *name_json = json->entries[0].value.as_object;
  printf("First name: %s\nLast name: %s\nAge: %f",
    name_json->entries[0].value.as_string,
    name_json->entries[1].value.as_string,
    json->entries[1].value.as_number);
  return 0;
}
```
Outputs

```bash
First name: John
Last name: Doe
Age: 21.5
```

### Example in repository

 1. Clone this repository `git clone https://github.com/forkachild/C-Simple-JSON-Parser`
 2. Compile the example `clang example.c json.c -o example.out`
 3. Run the binary `./example.out`

## FAQs

### How to know the type?

At each Key-Value pair `json_entry_t`, there is a member `type`

```C
#include "json.h"

...

json_object_t *json = json_parse(some_json_string);
json_entry_t entry = json->entries[0];

switch(entry.type) {
  case JSON_TYPE_STRING:
    // `entry.value.as_string` is a `json_string_t`
    break;
  case JSON_TYPE_NUMBER:
    // `entry.value.as_number` is a `json_number_t`
    break;
  case JSON_TYPE_OBJECT:
    // `entry.value.as_object` is a `json_object_t *`
    break;
  case JSON_TYPE_ARRAY:
    // `entry.value.as_array` is a `json_array_t *`
    break;
  case JSON_TYPE_BOOLEAN:
    // `entry.value.as_boolean` is a `json_boolean_t`
    break;
}
```

### How to get the count of number of Key-Value pairs?

In each `json_object_t`, there is a member `count`

```C
#include "json.h"

...

int i;

json_object_t *json = json_parse(some_json_string);

for(i = 0; i < json->count; i++) {
  json_entry_t entry = json->entries[i];

  json_string_t key = entry.key;
  json_type_t type = entry.type;
  json_value_t value = entry.value;
  // Do something with `key`, `type` and `value`
}
```

### How to get the number of elements in an array?

In each `json_array_t`, there is a member `count`

```C
#include "json.h"

...

int i;

json_object_t *json = json_parse(some_json_string);
json_array_t *array = json->entries[0].value.as_array;

for(i = 0; i < array->count; i++) {
  json_array_element_t element = array->elements[i];

  json_type_t type = element.type;
  json_value_t value = element.value;
  // Do something with `value`
}
```

### What if the JSON is poorly formatted with uneven whitespace

Compile using `-DJSON_SCRAPE_WHITESPACE`

### What if there is error in JSON

That is when `json_parse` returns `NULL`

## If this helped you in any way you can [buy me a beer](https://www.paypal.me/suhelchakraborty)
