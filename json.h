#ifndef JSON_H
#define JSON_H

#include <stddef.h>

#ifndef __cplusplus
typedef unsigned char bool;
#define true (1)
#define false (0)
#endif

#define result(type) type##_result_t
#define result_ok(type) type##_result_ok
#define result_err(type) type##_result_err
#define result_is_ok(type) type##_result_is_ok
#define result_is_err(type) type##_result_is_err
#define result_unwrap(type) type##_result_unwrap
#define result_unwrap_err(type) type##_result_unwrap_err
#define result_map_err(outer_type, inner_type, value) result_err(outer_type)(result_unwrap_err(inner_type)(value))
#define result_try(outer_type, inner_type, lvalue, rvalue)               \
    result(inner_type) lvalue##_result = rvalue;                         \
    if (result_is_err(inner_type)(&lvalue##_result))                     \
        return result_map_err(outer_type, inner_type, &lvalue##_result); \
    const inner_type##_t lvalue = result_unwrap(inner_type)(&lvalue##_result);
#define define_result_type(type)                                 \
    typedef struct type##_result_s {                             \
        json_boolean_t is_ok;                                    \
        union {                                                  \
            type##_t value;                                      \
            const char *err;                                     \
        } inner;                                                 \
    } result(type);                                              \
    result(type) result_ok(type)(type##_t value) {               \
        result(type) retval = {                                  \
            .is_ok = true,                                       \
            .inner = {                                           \
                .value = value,                                  \
            },                                                   \
        };                                                       \
        return retval;                                           \
    }                                                            \
    result(type) result_err(type)(const char *err) {             \
        result(type) retval = {                                  \
            .is_ok = false,                                      \
            .inner = {                                           \
                .err = err,                                      \
            },                                                   \
        };                                                       \
        return retval;                                           \
    }                                                            \
    json_boolean_t result_is_ok(type)(result(type) * result) {   \
        return result->is_ok;                                    \
    }                                                            \
    json_boolean_t result_is_err(type)(result(type) * result) {  \
        return !result->is_ok;                                   \
    }                                                            \
    type##_t result_unwrap(type)(result(type) * result) {        \
        return result->inner.value;                              \
    }                                                            \
    const char *result_unwrap_err(type)(result(type) * result) { \
        return result->inner.err;                                \
    }

typedef const char *json_string_t;
typedef bool json_boolean_t;
typedef double json_number_t;
typedef enum json_type_t json_type_t;
typedef struct json_object_t json_object_t;
typedef struct json_array_t json_array_t;
typedef union json_value_t json_value_t;
typedef struct json_entry_t json_entry_t;

enum json_type_t {
    JSON_TYPE_STRING = 0,
    JSON_TYPE_NUMBER,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NULL
};

struct json_object_t {
    size_t count;
    json_entry_t *entries;
};

struct json_array_t {
    size_t count;
    json_type_t type;
    json_value_t *values;
};

union json_value_t {
    json_string_t as_string;
    json_number_t as_number;
    json_object_t *as_object;
    json_array_t *as_array;
    json_boolean_t as_boolean;
};

struct json_entry_t {
    json_string_t key;
    json_type_t type;
    json_value_t value;
};

define_result_type(json_value);
define_result_type(json_entry);
define_result_type(json_type);
define_result_type(size);

json_object_t *json_parse(const char *json_str);
void json_print(json_object_t *obj, int indent);
void json_free(json_object_t *obj);

#endif
