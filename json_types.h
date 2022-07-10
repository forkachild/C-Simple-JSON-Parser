#pragma once

#ifndef __cplusplus
typedef unsigned char bool;
#define true (1)
#define false (0)
#endif

#define typed(name) name##_t

typedef enum json_type_e typed(json_type);
typedef const char *typed(json_string);
typedef bool typed(json_boolean);
typedef double typed(json_number);
typedef struct json_object_s typed(json_object);
typedef struct json_array_s typed(json_array);
typedef union json_value_u typed(json_value);
typedef struct json_entry_s typed(json_entry);
typedef enum json_error_e typed(json_error);

#define result(name) name##_result_t
#define result_ok(name) name##_result_ok
#define result_err(name) name##_result_err
#define result_is_ok(name) name##_result_is_ok
#define result_is_err(name) name##_result_is_err
#define result_unwrap(name) name##_result_unwrap
#define result_unwrap_err(name) name##_result_unwrap_err
#define result_map_err(outer_name, inner_name, value) \
    result_err(outer_name)(result_unwrap_err(inner_name)(value))
#define result_try(outer_name, inner_name, lvalue, rvalue)               \
    result(inner_name) lvalue##_result = rvalue;                         \
    if (result_is_err(inner_name)(&lvalue##_result))                     \
        return result_map_err(outer_name, inner_name, &lvalue##_result); \
    const typed(inner_name) lvalue = result_unwrap(inner_name)(&lvalue##_result);
#define declare_result_type(name)                            \
    typedef struct name##_result_s {                         \
        typed(json_boolean) is_ok;                           \
        union {                                              \
            typed(name) value;                               \
            typed(json_error) err;                           \
        } inner;                                             \
    } result(name);                                          \
    result(name) result_ok(name)(typed(name));               \
    result(name) result_err(name)(typed(json_error));        \
    typed(json_boolean) result_is_ok(name)(result(name) *);  \
    typed(json_boolean) result_is_err(name)(result(name) *); \
    typed(name) result_unwrap(name)(result(name) *);         \
    typed(json_error) result_unwrap_err(name)(result(name) *);

#define define_result_type(name)                                       \
    result(name) result_ok(name)(typed(name) value) {                  \
        result(name) retval = {                                        \
            .is_ok = true,                                             \
            .inner = {                                                 \
                .value = value,                                        \
            },                                                         \
        };                                                             \
        return retval;                                                 \
    }                                                                  \
    result(name) result_err(name)(typed(json_error) err) {             \
        result(name) retval = {                                        \
            .is_ok = false,                                            \
            .inner = {                                                 \
                .err = err,                                            \
            },                                                         \
        };                                                             \
        return retval;                                                 \
    }                                                                  \
    typed(json_boolean) result_is_ok(name)(result(name) * result) {    \
        return result->is_ok;                                          \
    }                                                                  \
    typed(json_boolean) result_is_err(name)(result(name) * result) {   \
        return !result->is_ok;                                         \
    }                                                                  \
    typed(name) result_unwrap(name)(result(name) * result) {           \
        return result->inner.value;                                    \
    }                                                                  \
    typed(json_error) result_unwrap_err(name)(result(name) * result) { \
        return result->inner.err;                                      \
    }

enum json_type_e {
    JSON_TYPE_STRING = 0,
    JSON_TYPE_NUMBER,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NULL
};

struct json_object_s {
    typed(size) count;
    typed(json_entry) * entries;
};

struct json_array_s {
    typed(size) count;
    typed(json_type) type;
    typed(json_value) * values;
};

union json_value_u {
    typed(json_string) as_string;
    typed(json_number) as_number;
    typed(json_object) * as_object;
    typed(json_array) * as_array;
    typed(json_boolean) as_boolean;
};

struct json_entry_s {
    typed(json_string) key;
    typed(json_type) type;
    typed(json_value) value;
};

enum json_error_e {
    JSON_ERROR_EMPTY = 0,
    JSON_ERROR_INVALID_TYPE,
    JSON_ERROR_INVALID_KEY,
    JSON_ERROR_INVALID_VALUE
};