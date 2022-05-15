#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef JSON_DEBUG
#define log(tag, string, ...) \
    printf(tag "(%s::%s::%d): " string "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define log_info(string, ...) log("INFO", string, ##__VA_ARGS__)
#define log_debug(string, ...) log("DEBUG", string, ##__VA_ARGS__)
#else
#define log
#define log_info
#define log_debug
#endif

#ifdef JSON_SCRAPE_WHITESPACE
#define json_scrape_whitespace(arg) json_skip_whitespace(arg)
#else
#define json_scrape_whitespace(arg)
#endif

#define allocN(x, y) (x *)malloc((y) * sizeof(x))
#define alloc(x) allocN(x, 1)
#define reallocN(x, y, z) (y *)realloc(x, (z) * sizeof(y))
#define is_whitespace(x) x == ' ' || x == '\n' || x == '\r' || x == '\t'

#define VALID 0
#define INVALID -1

static void
json_print_value(json_type_t, json_value_t *, int, int);
static void
json_print_object(json_object_t *, int, int);
static void
json_print_array(json_array_t *, int, int);
static void
json_free_value(json_type_t, json_value_t *);
static void
json_free_object(json_object_t *);
static void
json_free_array(json_array_t *);
static void
json_skip_whitespace(const char **);
static result(size) json_index_of(const char *, char);
static result(json_value) json_parse_value(const char **, json_type_t);
static result(json_value) json_parse_string(const char **);
static result(size) json_string_len(const char *str);
static json_string_t
json_parse_escaped_string(const char *, size_t);
static result(json_value) json_parse_number(const char **);
static result(json_value) json_parse_object(const char **);
static result(json_value) json_parse_array(const char **);
static result(json_value) json_parse_boolean(const char **);
static void
json_skip_null(const char **);
static result(json_entry) json_parse_entry(const char **);
static result(json_type) json_guess_value_type(const char *);

json_object_t *
json_parse(const char *json_str) {
    // TODO: Handle this!
    return json_parse_object(&json_str).inner.value.as_object;
}

void json_print(json_object_t *object, int indent) {
    json_print_object(object, indent, 0);
}

void json_free(json_object_t *object) {
    json_free_object(object);
}

static void
json_print_value(json_type_t type, json_value_t *value, int indent, int indent_level) {
    switch (type) {
    case JSON_TYPE_STRING:
        printf("\"%s\"", value->as_string);
        break;

    case JSON_TYPE_NUMBER:
        printf("%f", value->as_number);
        break;

    case JSON_TYPE_OBJECT:
        json_print_object(value->as_object, indent, indent_level);
        break;

    case JSON_TYPE_ARRAY:
        json_print_array(value->as_array, indent, indent_level);
        break;

    case JSON_TYPE_BOOLEAN:
        printf(value->as_boolean ? "true" : "false");
        break;

    case JSON_TYPE_NULL:
        // Do nothing
        break;
    }
}

static void
json_print_object(json_object_t *object, int indent, int indent_level) {
    printf("{\n");

    for (int i = 0; i < object->count; i++) {
        for (int j = 0; j < indent * (indent_level + 1); j++)
            printf(" ");

        json_entry_t *entry = &object->entries[i];

        printf("\"%s\": ", entry->key);
        json_print_value(entry->type, &entry->value, indent, indent_level + 1);

        if (i != object->count - 1)
            printf(",");
        printf("\n");
    }

    for (int j = 0; j < indent * indent_level; j++)
        printf(" ");
    printf("}");
}

static void
json_print_array(json_array_t *array, int indent, int indent_level) {
    printf("[\n");

    for (int i = 0; i < array->count; i++) {
        for (int j = 0; j < indent * (indent_level + 1); j++)
            printf(" ");
        json_print_value(array->type, &array->values[i], indent, indent_level + 1);

        if (i != array->count - 1)
            printf(",");
        printf("\n");
    }

    for (int i = 0; i < indent * indent_level; i++)
        printf(" ");
    printf("]");
}

static void
json_free_value(json_type_t type, json_value_t *value) {
    switch (type) {
    case JSON_TYPE_STRING:
        free((void *)(value->as_string));
        break;

    case JSON_TYPE_OBJECT:
        json_free_object(value->as_object);
        break;

    case JSON_TYPE_ARRAY:
        json_free_array(value->as_array);
        break;

    case JSON_TYPE_NUMBER:
    case JSON_TYPE_BOOLEAN:
    case JSON_TYPE_NULL:
        // Do nothing
        break;
    }
}

static void
json_free_object(json_object_t *object) {
    if (object == NULL)
        return;

    if (object->count == 0) {
        free(object);
        return;
    }

    for (int i = 0; i < object->count; i++) {
        json_entry_t *entry = &object->entries[i];

        free((void *)entry->key);
        json_free_value(entry->type, &entry->value);
    }

    free(object->entries);
    free(object);
}

static void
json_free_array(json_array_t *array) {
    if (array == NULL)
        return;

    if (array->count == 0) {
        free(array);
        return;
    }

    for (int i = 0; i < array->count; i++)
        json_free_value(array->type, &array->values[i]);

    free(array->values);
    free(array);
}

static void
json_skip_whitespace(const char **str_ptr) {
    while (is_whitespace(**str_ptr))
        (*str_ptr)++;
}

static result(size) json_index_of(const char *str, char ch) {
    size_t pos = 0;

    while (*str != '\0') {
        // We are gonna skip past escape sequences for now
        if (ch == '\\') {
            str += 2;
        }

        if (ch == *str) {
            return result_ok(size)(pos);
        }

        str++;
        pos++;
    }

    return result_err(size)("Not found");
}

static result(json_value)
    json_parse_value(const char **str_ptr, json_type_t type) {
    switch (type) {
    case JSON_TYPE_STRING:
        return json_parse_string(str_ptr);
    case JSON_TYPE_NUMBER:
        return json_parse_number(str_ptr);
    case JSON_TYPE_OBJECT:
        return json_parse_object(str_ptr);
    case JSON_TYPE_ARRAY:
        return json_parse_array(str_ptr);
    case JSON_TYPE_BOOLEAN:
        return json_parse_boolean(str_ptr);
    case JSON_TYPE_NULL:
        json_skip_null(str_ptr);
        return result_err(json_value)("Can't parse null");
    }
}

static result(json_value) json_parse_string(const char **str_ptr) {
    // Skip the first '"' character
    (*str_ptr)++;

    result(size) len_result = json_string_len(*str_ptr);

    if (result_is_err(size)(&len_result))
        return result_err(json_value)("Invalid size");

    const size_t len = result_unwrap(size)(&len_result);

    // Copy the string
    json_string_t output = json_parse_escaped_string(*str_ptr, len);

    // Skip to beyond the string
    (*str_ptr) += len + 1;

    return result_ok(json_value)((json_value_t)output);
}

static result(size) json_string_len(const char *str) {
    size_t len = 0;

    const char *iter = str;
    while (*iter != '\0') {
        if (*iter == '\\')
            iter += 2;

        if (*iter == '"') {
            len = iter - str;
            break;
        }

        iter++;
    }

    if (len == 0)
        return result_err(size)("Invalid size");

    return result_ok(size)(len);
}

static json_string_t
json_parse_escaped_string(const char *str, size_t len) {
    size_t offset = 0;
    const char *iter = str;

    while (iter - str < len) {
        if (*iter == '\\') {
            iter++;
        }

        offset++;
        iter++;
    }

    char *output = allocN(char, offset + 1);
    offset = 0;
    iter = str;

    while (iter - str < len) {
        if (*iter == '\\') {
            iter++;

            switch (*iter) {
            case 'b':
                output[offset] = '\b';
                break;
            case 'f':
                output[offset] = '\f';
                break;
            case 'n':
                output[offset] = '\n';
                break;
            case 'r':
                output[offset] = '\r';
                break;
            case 't':
                output[offset] = '\t';
                break;
            case '"':
                output[offset] = '"';
                break;
            case '\\':
                output[offset] = '\\';
                break;
                // TODO: Check for illegal escape character and return error
            }
        } else {
            output[offset] = *iter;
        }

        offset++;
        iter++;
    }

    output[offset] = '\0';
    return (json_string_t)output;
}

static result(json_value) json_parse_number(const char **str_ptr) {
    json_number_t number = strtod(*str_ptr, (char **)str_ptr);
    // TODO: Handle strtod errors
    return result_ok(json_value)((json_value_t)number);
}

static result(json_value) json_parse_object(const char **str_ptr) {
    // Skip the first '{' character
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    if (**str_ptr == '}')
        return result_err(json_value)("Empty");

    json_entry_t *entries = NULL;
    size_t count = 0;

    while (**str_ptr != '\0') {
        // Skip any accidental whitespace
        json_scrape_whitespace(str_ptr);

        result(json_entry) entry_result = json_parse_entry(str_ptr);

        if (result_is_ok(json_entry)(&entry_result)) {
            count++;
            entries = reallocN(entries, json_entry_t, count);
            const json_entry_t entry = result_unwrap(json_entry)(&entry_result);
            memcpy(&entries[count - 1], &entry, sizeof(json_entry_t));
        }

        // Skip any accidental whitespace
        json_scrape_whitespace(str_ptr);

        if (**str_ptr == '}')
            break;

        // Skip the ',' to move to the next entry
        (*str_ptr)++;
    }

    // Skip the '}' closing brace
    (*str_ptr)++;

    json_object_t *object = alloc(json_object_t);
    object->count = count;
    object->entries = entries;

    return result_ok(json_value)((json_value_t)object);
}

static result(json_value) json_parse_array(const char **str_ptr) {
    // Skip the starting '[' character
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    if (**str_ptr == ']')
        return result_err(json_value)("Empty");

    result_try(json_value, json_type, type, json_guess_value_type(*str_ptr));

    size_t count = 0;
    json_value_t *values = NULL;

    while (**str_ptr != '\0') {
        json_scrape_whitespace(str_ptr);
        result(json_value) value_result = json_parse_value(str_ptr, type);

        if (result_is_ok(json_value)(&value_result)) {
            count++;
            values = reallocN(values, json_value_t, count);
            const json_value_t value = result_unwrap(json_value)(&value_result);
            memcpy(&values[count - 1], &value, sizeof(json_value_t));
        }

        json_scrape_whitespace(str_ptr);

        if (**str_ptr == ']')
            break;

        (*str_ptr)++;
    }

    // Skip the ']' closing array
    (*str_ptr)++;

    json_array_t *array = alloc(json_array_t);
    array->count = count;
    array->type = type;
    array->values = values;

    return result_ok(json_value)((json_value_t)array);
}

static result(json_value) json_parse_boolean(const char **str_ptr) {
    json_boolean_t output;

    switch (**str_ptr) {
    case 't':
        output = true;
        (*str_ptr) += 4;
        break;

    case 'f':
        output = false;
        (*str_ptr) += 5;
        break;
    }

    return result_ok(json_value)((json_value_t)output);
}

static void
json_skip_null(const char **str_ptr) {
    (*str_ptr) += 4;
}

static result(json_entry) json_parse_entry(const char **str_ptr) {
    result_try(json_entry, json_value, key, json_parse_string(str_ptr));
    json_scrape_whitespace(str_ptr);

    // Skip the ':' delimiter
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    result(json_type) type_result = json_guess_value_type(*str_ptr);
    if (result_is_err(json_type)(&type_result)) {
        free((void *)key.as_string);
        return result_map_err(json_entry, json_type, &type_result);
    }
    const json_type_t type = result_unwrap(json_type)(&type_result);

    result(json_value) value_result = json_parse_value(str_ptr, type);
    if (result_is_err(json_value)(&value_result)) {
        free((void *)key.as_string);
        return result_map_err(json_entry, json_value, &value_result);
    }
    const json_value_t value = result_unwrap(json_value)(&value_result);

    json_entry_t entry = {
        .key = key.as_string,
        .type = type,
        .value = value,
    };

    return result_ok(json_entry)(entry);
}

static result(json_type) json_guess_value_type(const char *str) {
    const char first_ch = *str;
    json_type_t type;

    if (first_ch == '"')
        type = JSON_TYPE_STRING;
    else if ((first_ch >= '0' && first_ch <= '9') || first_ch == '+' || first_ch == '-')
        type = JSON_TYPE_NUMBER;
    else if (first_ch == '{')
        type = JSON_TYPE_OBJECT;
    else if (first_ch == '[')
        type = JSON_TYPE_ARRAY;
    else if (first_ch == 't' || first_ch == 'f')
        type = JSON_TYPE_BOOLEAN;
    else if (first_ch == 'n')
        type = JSON_TYPE_NULL;
    else
        return result_err(json_type)("Invalid type");

    return result_ok(json_type)(type);
}