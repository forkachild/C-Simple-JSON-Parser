#include "json.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_types.h"

#ifdef JSON_SCRAPE_WHITESPACE
#define json_scrape_whitespace(arg) json_skip_whitespace(arg)
#else
#define json_scrape_whitespace(arg)
#endif

#define allocN(type, count) (type *)malloc((count) * sizeof(type))
#define alloc(type) allocN(type, 1)
#define reallocN(ptr, type, count) (type *)realloc(ptr, (count) * sizeof(type))
#define is_whitespace(ch) (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')

static void json_print_value(typed(json_type), typed(json_value) *, int, int);
static void json_print_object(typed(json_object) *, int, int);
static void json_print_array(typed(json_array) *, int, int);
static void json_free_value(typed(json_type), typed(json_value) *);
static void json_free_object(typed(json_object) *);
static void json_free_array(typed(json_array) *);
static void json_skip_whitespace(typed(json_string) *);
static void json_skip_null(typed(json_string) *);
static result(size) json_index_of(typed(json_string), char);
static result(json_value) json_parse_value(typed(json_string) *, typed(json_type));
static result(json_value) json_parse_string(typed(json_string) *);
static result(size) json_string_len(typed(json_string));
static result(json_string) json_parse_escaped_string(typed(json_string), typed(size));
static result(json_value) json_parse_number(typed(json_string) *);
static result(json_value) json_parse_object(typed(json_string) *);
static result(json_value) json_parse_array(typed(json_string) *);
static result(json_value) json_parse_boolean(typed(json_string) *);
static result(json_entry) json_parse_entry(typed(json_string) *);
static result(json_type) json_guess_value_type(typed(json_string));

typed(json_object) * json_parse(typed(json_string) json_str) {
    result(json_value) output_result = json_parse_object(&json_str);

    if (result_is_err(json_value)(&output_result)) {
        fprintf(stderr, "Error parsing JSON: %s", result_unwrap_err(json_value)(&output_result));
        return NULL;
    }

    return result_unwrap(json_value)(&output_result).as_object;
}

void json_print(typed(json_object) * object, int indent) {
    json_print_object(object, indent, 0);
}

void json_free(typed(json_object) * object) {
    json_free_object(object);
}

static void json_print_value(typed(json_type) type, typed(json_value) * value, int indent, int indent_level) {
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

static void json_print_object(typed(json_object) * object, int indent, int indent_level) {
    printf("{\n");

    for (int i = 0; i < object->count; i++) {
        for (int j = 0; j < indent * (indent_level + 1); j++)
            printf(" ");

        typed(json_entry) *entry = &object->entries[i];

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

static void json_print_array(typed(json_array) * array, int indent, int indent_level) {
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

static void json_free_value(typed(json_type) type, typed(json_value) * value) {
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

static void json_free_object(typed(json_object) * object) {
    if (object == NULL)
        return;

    if (object->count == 0) {
        free(object);
        return;
    }

    for (int i = 0; i < object->count; i++) {
        typed(json_entry) *entry = &object->entries[i];

        free((void *)entry->key);
        json_free_value(entry->type, &entry->value);
    }

    free(object->entries);
    free(object);
}

static void json_free_array(typed(json_array) * array) {
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

static void json_skip_whitespace(typed(json_string) * str_ptr) {
    while (is_whitespace(**str_ptr))
        (*str_ptr)++;
}

static void json_skip_null(typed(json_string) * str_ptr) {
    (*str_ptr) += 4;
}

static result(size) json_index_of(typed(json_string) str, char ch) {
    typed(size) pos = 0;

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

static result(json_value) json_parse_value(typed(json_string) * str_ptr, typed(json_type) type) {
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

static result(json_value) json_parse_string(typed(json_string) * str_ptr) {
    // Skip the first '"' character
    (*str_ptr)++;

    result_try(json_value, size, len, json_string_len(*str_ptr));
    result_try(json_value, json_string, output, json_parse_escaped_string(*str_ptr, len));

    // Skip to beyond the string
    (*str_ptr) += len + 1;

    return result_ok(json_value)((typed(json_value))output);
}

static result(size) json_string_len(typed(json_string) str) {
    typed(size) len = 0;

    typed(json_string) iter = str;
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

static result(json_string) json_parse_escaped_string(typed(json_string) str, typed(size) len) {
    typed(size) offset = 0;
    typed(json_string) iter = str;

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
                default:
                    return result_err(json_string)("Invalid escape character");
            }
        } else {
            output[offset] = *iter;
        }

        offset++;
        iter++;
    }

    output[offset] = '\0';
    return result_ok(json_string)((typed(json_string))output);
}

static result(json_value) json_parse_number(typed(json_string) * str_ptr) {
    errno = 0;
    typed(json_number) number = strtod(*str_ptr, (char **)str_ptr);

    if (errno == EINVAL)
        return result_err(json_value)("Invalid number");

    if (errno == ERANGE)
        return result_err(json_value)("Out of range");

    return result_ok(json_value)((typed(json_value))number);
}

static result(json_value) json_parse_object(typed(json_string) * str_ptr) {
    // Skip the first '{' character
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    if (**str_ptr == '}')
        return result_err(json_value)("Empty");

    typed(json_entry) *entries = NULL;
    typed(size) count = 0;

    while (**str_ptr != '\0') {
        // Skip any accidental whitespace
        json_scrape_whitespace(str_ptr);

        result(json_entry) entry_result = json_parse_entry(str_ptr);

        if (result_is_ok(json_entry)(&entry_result)) {
            count++;
            entries = reallocN(entries, typed(json_entry), count);
            const typed(json_entry) entry = result_unwrap(json_entry)(&entry_result);
            memcpy(&entries[count - 1], &entry, sizeof(typed(json_entry)));
        }

        // Skip any accidental whitespace
        json_scrape_whitespace(str_ptr);

        if (**str_ptr == '}')
            break;

        // Skip the ',' to move to the next entry
        (*str_ptr)++;
    }

    if (entries == NULL)
        return result_err(json_value)("No non-null entries");

    // Skip the '}' closing brace
    (*str_ptr)++;

    typed(json_object) *object = alloc(typed(json_object));
    object->count = count;
    object->entries = entries;

    return result_ok(json_value)((typed(json_value))object);
}

static result(json_value) json_parse_array(typed(json_string) * str_ptr) {
    // Skip the starting '[' character
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    if (**str_ptr == ']')
        return result_err(json_value)("Empty array");

    result_try(json_value, json_type, type, json_guess_value_type(*str_ptr));

    typed(size) count = 0;
    typed(json_value) *values = NULL;

    while (**str_ptr != '\0') {
        json_scrape_whitespace(str_ptr);
        result(json_value) value_result = json_parse_value(str_ptr, type);

        if (result_is_ok(json_value)(&value_result)) {
            count++;
            values = reallocN(values, typed(json_value), count);
            const typed(json_value) value = result_unwrap(json_value)(&value_result);
            memcpy(&values[count - 1], &value, sizeof(typed(json_value)));
        }

        json_scrape_whitespace(str_ptr);

        if (**str_ptr == ']')
            break;

        (*str_ptr)++;
    }

    // Skip the ']' closing array
    (*str_ptr)++;

    typed(json_array) *array = alloc(typed(json_array));
    array->count = count;
    array->type = type;
    array->values = values;

    return result_ok(json_value)((json_value_t)array);
}

static result(json_value) json_parse_boolean(typed(json_string) * str_ptr) {
    typed(json_boolean) output;

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

static result(json_entry) json_parse_entry(typed(json_string) * str_ptr) {
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
    const typed(json_type) type = result_unwrap(json_type)(&type_result);

    result(json_value) value_result = json_parse_value(str_ptr, type);
    if (result_is_err(json_value)(&value_result)) {
        free((void *)key.as_string);
        return result_map_err(json_entry, json_value, &value_result);
    }
    const typed(json_value) value = result_unwrap(json_value)(&value_result);

    typed(json_entry) entry = {
        .key = key.as_string,
        .type = type,
        .value = value,
    };

    return result_ok(json_entry)(entry);
}

static result(json_type) json_guess_value_type(typed(json_string) str) {
    const char first_ch = *str;
    typed(json_type) type;

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

define_result_type(json_value);
define_result_type(json_entry);
define_result_type(json_string);
define_result_type(json_type);
define_result_type(size);