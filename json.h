#pragma once

#include <stddef.h>

#ifndef __cplusplus
typedef unsigned int bool;
#define true (1)
#define false (0)
#endif

#define typed(name) name##_t

typedef const char *typed(json_string);
typedef bool typed(json_boolean);

typedef union json_number_value_u typed(json_number_value);
typedef signed long typed(json_number_long);
typedef double typed(json_number_double);
typedef struct json_number_s typed(json_number);
typedef union json_element_value_u typed(json_element_value);
typedef struct json_element_s typed(json_element);
typedef struct json_entry_s typed(json_entry);
typedef struct json_object_s typed(json_object);
typedef struct json_array_s typed(json_array);

#define result(name) name##_result_t
#define result_ok(name) name##_result_ok
#define result_err(name) name##_result_err
#define result_is_ok(name) name##_result_is_ok
#define result_is_err(name) name##_result_is_err
#define result_unwrap(name) name##_result_unwrap
#define result_unwrap_err(name) name##_result_unwrap_err
#define result_map_err(outer_name, inner_name, value)                          \
  result_err(outer_name)(result_unwrap_err(inner_name)(value))
#define result_try(outer_name, inner_name, lvalue, rvalue)                     \
  result(inner_name) lvalue##_result = rvalue;                                 \
  if (result_is_err(inner_name)(&lvalue##_result))                             \
    return result_map_err(outer_name, inner_name, &lvalue##_result);           \
  const typed(inner_name) lvalue = result_unwrap(inner_name)(&lvalue##_result);
#define declare_result_type(name)                                              \
  typedef struct name##_result_s {                                             \
    typed(json_boolean) is_ok;                                                 \
    union {                                                                    \
      typed(name) value;                                                       \
      typed(json_error) err;                                                   \
    } inner;                                                                   \
  } result(name);                                                              \
  result(name) result_ok(name)(typed(name));                                   \
  result(name) result_err(name)(typed(json_error));                            \
  typed(json_boolean) result_is_ok(name)(result(name) *);                      \
  typed(json_boolean) result_is_err(name)(result(name) *);                     \
  typed(name) result_unwrap(name)(result(name) *);                             \
  typed(json_error) result_unwrap_err(name)(result(name) *);

typedef enum json_element_type_e {
  JSON_ELEMENT_TYPE_STRING = 0,
  JSON_ELEMENT_TYPE_NUMBER,
  JSON_ELEMENT_TYPE_OBJECT,
  JSON_ELEMENT_TYPE_ARRAY,
  JSON_ELEMENT_TYPE_BOOLEAN,
  JSON_ELEMENT_TYPE_NULL
} typed(json_element_type);

typedef enum json_number_type_e {
  JSON_NUMBER_TYPE_LONG = 0,
  JSON_NUMBER_TYPE_DOUBLE,
} typed(json_number_type);

union json_number_value_u {
  typed(json_number_long) as_long;
  typed(json_number_double) as_double;
};

struct json_number_s {
  typed(json_number_type) type;
  typed(json_number_value) value;
};

union json_element_value_u {
  typed(json_string) as_string;
  typed(json_number) as_number;
  typed(json_object) * as_object;
  typed(json_array) * as_array;
  typed(json_boolean) as_boolean;
};

struct json_element_s {
  typed(json_element_type) type;
  typed(json_element_value) value;
};

struct json_entry_s {
  typed(json_string) key;
  typed(json_element) element;
};

struct json_object_s {
  typed(size) count;
  typed(json_entry) * *entries;
};

struct json_array_s {
  typed(size) count;
  typed(json_element) * elements;
};

typedef enum json_error_e {
  JSON_ERROR_EMPTY = 0,
  JSON_ERROR_INVALID_TYPE,
  JSON_ERROR_INVALID_KEY,
  JSON_ERROR_INVALID_VALUE
} typed(json_error);

declare_result_type(json_element_type)
declare_result_type(json_element_value)
declare_result_type(json_element)
declare_result_type(json_entry)
declare_result_type(json_string)
declare_result_type(size)

/**
 * @brief Parses a JSON string into a JSON element {json_element_t}
 * with a fallible `result` type
 *
 * @param json_str The raw JSON string
 * @return The parsed {json_element_t} wrapped in a `result` type
 */
result(json_element) json_parse(typed(json_string) json_str);

/**
 * @brief Tries to get the element by key. If not found, returns
 * a {JSON_ERROR_INVALID_KEY} error
 *
 * @param object The object to find the key in
 * @param key The key of the element to be found
 * @return Either a {json_element_t} or {json_error_t}
 */
result(json_element)
    json_object_find(typed(json_object) * object, typed(json_string) key);

/**
 * @brief Prints a JSON element {json_element_t} with proper
 * indentation
 *
 * @param indent The number of spaces to indent each level by
 */
void json_print(typed(json_element) * element, int indent);

/**
 * @brief Frees a JSON element {json_element_t} from memory
 *
 * @param element The JSON element {json_element_t} to free
 */
void json_free(typed(json_element) * element);

/**
 * @brief Returns a string representation of JSON error {json_error_t} type
 *
 * @param error The JSON error enum {json_error_t} type
 * @return The string representation
 */
typed(json_string) json_error_to_string(typed(json_error) error);

