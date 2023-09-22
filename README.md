# Simple JSON Parser in C

An easy to use, very fast JSON parsing implementation written in pure C

## Features

- Fully [RFC-8259](https://datatracker.ietf.org/doc/html/rfc8259) compliant
- Small 2 file library
- Support for all data types
- Simple and efficient hash table implementation to search element by key
- Rust like `result` type used throughout fallible calls
- Compile with `-DJSON_SKIP_WHITESPACE` to parse non-minified JSON with whitespace in between

## Setup

Copy the following from this repository in your source

- `json.h`
- `json.c`

And in your code

```C
#include "json.h"
```

## MACROs

### The `typed` helper

A uniform type system used throughout the API
`typed(x)` is alias for `x_t`

```C
typed(json_element) // json_element_t
```

### Rust like `result`

A tagged union comprising two variants, either `ok` with the data or `err` with `typed(json_error)` with simplified API to manage variants

```C
result(json_element) // json_element_result_t
```

## API

### Parse JSON:

```C
result(json_element) json_parse(typed(json_string) json_str);
```

### Find an element by key

```C
result(json_element) json_object_find(typed(json_object) * object, typed(json_string) key);
```

### Print JSON with specified indentation

```C
void json_print(typed(json_element) *element, int indent);
```

### Free JSON from memory

```C
void json_free(typed(json_element) *element);
```

### Convert error into user friendly error String

```C
typed(json_string) json_error_to_string(typed(json_error) error);
```

## Types

### JSON String

A null-terminated char-sequence

```C
typed(json_string) // alias for const char *
```

### JSON Number

A 64-bit floating point number

```C
typed(json_number) // alias for double
```

### JSON Object

An array of key-value entries

```C
typed(json_object)
```

#### Fields

| **Name**  | **Type**              | **Description**       |
| --------- | --------------------- | --------------------- |
| `count`   | `typed(size)`         | The number of entries |
| `entries` | `typed(json_entry) *` | The array of entries  |

### JSON Array

A hetergeneous array of elements

```C
typed(json_array)
```

#### Fields

| **Name**   | **Type**                | **Description**        |
| ---------- | ----------------------- | ---------------------- |
| `count`    | `typed(size)`           | The number of elements |
| `elements` | `typed(json_element) *` | The array of elements  |

### JSON Boolean

A boolean value

```C
typed(json_boolean)
```

### Element

A tagged union representing a JSON value with its type

```C
typed(json_element)
```

#### Fields

| **Name** | **Type**                    | **Description**       |
| -------- | --------------------------- | --------------------- |
| `type`   | `typed(json_element_type)`  | The type of the value |
| `value`  | `typed(json_element_value)` | The actual value      |

### Element Type

An enum which represents a JSON type

```C
typed(json_element_type)
```

#### Variants

| **Variant**                 | **Description** |
| --------------------------- | --------------- |
| `JSON_ELEMENT_TYPE_STRING`  | JSON String     |
| `JSON_ELEMENT_TYPE_NUMBER`  | JSON Number     |
| `JSON_ELEMENT_TYPE_OBJECT`  | JSON Object     |
| `JSON_ELEMENT_TYPE_ARRAY`   | JSON Array      |
| `JSON_ELEMENT_TYPE_BOOLEAN` | JSON Boolean    |
| `JSON_ELEMENT_TYPE_NULL`    | JSON Null       |

### Element Value

A union for interpreting JSON data

```C
typed(json_element_value)
```

#### Fields

| **Name**     | **Type**               | **Interpret data as** |
| ------------ | ---------------------- | --------------------- |
| `as_string`  | `typed(json_string)`   | JSON String           |
| `as_number`  | `typed(json_number)`   | JSON Number           |
| `as_object`  | `typed(json_object) *` | JSON Object           |
| `as_array`   | `typed(json_array) *`  | JSON Array            |
| `as_boolean` | `typed(json_boolean)`  | JSON Boolean          |

### Error

An enum which represents an error

```C
typed(json_error)
```

#### Variants

| **Variant**                | **Description**                |
| -------------------------- | ------------------------------ |
| `JSON_ERROR_EMPTY`         | Null or empty value            |
| `JSON_ERROR_INVALID_TYPE`  | Type inference failed          |
| `JSON_ERROR_INVALID_KEY`   | Key is not a valid string      |
| `JSON_ERROR_INVALID_VALUE` | Value is not a valid JSON type |

## Usage

### Parse with error checking

```C
#include "json.h"

const char * some_json_str = "{\"hello\":\"world\",\"key\":\"value\"}";

int main() {
  result(json_element) element_result = json_parse(some_json_str);

  // Guard if
  if(result_is_err(json_element)(&element_result)) {
    typed(json_error) error = result_unwrap_err(json_element)(&element_result);
    fprintf(stderr, "Error parsing JSON: %s\n", json_error_to_string(error));
    return -1;
  }

  // Extract the data
  typed(json_element) element = result_unwrap(json_element)(&element_result);

  // Fetch the "hello" key value
  result(json_element) hello_element_result = json_object_find(element.value.as_object, "hello");
  if(result_is_err(json_element)(&hello_element_result)) {
    typed(json_error) error = result_unwrap_err(json_element)(&hello_element_result);
    fprintf(stderr, "Error getting element \"hello\": %s\n", json_error_to_string(error));
    return -1;
  }
  typed(json_element) hello_element = result_unwrap(json_element)(&hello_element_result);

  // Use the element
  printf("\"hello\": \"%s\"\n", hello_element.value.as_string);
  json_print(&element, 2);
  json_free(&element);

  return 0;
}
```

Outputs

```
"hello": "world"
{
  "hello": "world",
  "key": "value"
}
```

### Example in repository

1.  Clone this repository `git clone https://github.com/forkachild/C-Simple-JSON-Parser`
2.  Compile the example `clang example.c json.c -o example.out`
3.  Run the binary `./example.out`

## FAQs

### How to know the type?

At each Key-Value pair `typed(json_entry_t)`, there is a member `type`

```C
#include "json.h"

...

typed(json_element) element = ...; // See example above
typed(json_entry) entry = element.value.as_object->entries[0];

switch(entry.element.type) {
  case JSON_TYPE_STRING:
    // `entry.element.value.as_string` is a `json_string_t`
    break;
  case JSON_TYPE_NUMBER:
    // `entry.element.value.as_number` is a `json_number_t`
    break;
  case JSON_TYPE_OBJECT:
    // `entry.element.value.as_object` is a `json_object_t *`
    break;
  case JSON_TYPE_ARRAY:
    // `entry.element.value.as_array` is a `json_array_t *`
    break;
  case JSON_TYPE_BOOLEAN:
    // `entry.element.value.as_boolean` is a `json_boolean_t`
    break;
}
```

### How to get the count of number of Key-Value pairs?

In each `typed(json_object)`, there is a member `count`

```C
#include "json.h"

...

int i;

typed(json_element) element = ...; // See example above
typed(json_object) *obj = element.value.as_object;

for(i = 0; i < obj->count; i++) {
  typed(json_entry) entry = obj->entries[i];

  typed(json_string) key = entry.key;
  typed(json_element_type) type = entry.element.type;
  typed(json_element_value) value = entry.element.value;
  // Do something with `key`, `type` and `value`
}
```

### How to get the number of elements in an array?

In each `typed(json_array)`, there is a member `count`

```C
#include "json.h"

...

int i;

typed(json_element) element = ...; // See example above
typed(json_array) *arr = element.value.as_array;

for(i = 0; i < arr->count; i++) {
  typed(json_element) element = arr->elements[i];

  typed(json_element_type) type = element.type;
  typed(json_element_value) value = element.value;
  // Do something with `value`
}
```

### What if the JSON is poorly formatted with uneven whitespace

Compile using `-DJSON_SKIP_WHITESPACE`

## If this helped you in any way you can [buy me a beer](https://www.paypal.me/suhelchakraborty)
