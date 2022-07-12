#include "json.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef JSON_SCRAPE_WHITESPACE
#define json_scrape_whitespace(arg) json_skip_whitespace(arg)
#else
#define json_scrape_whitespace(arg)
#endif

/**
 * @brief Allocate `count` number of items of `type` in memory
 * and return the pointer to the newly allocated memory
 */
#define allocN(type, count) (type *)malloc((count) * sizeof(type))

/**
 * @brief Allocate an item of `type` in memory and return the
 * pointer to the newly allocated memory
 */
#define alloc(type) allocN(type, 1)

/**
 * @brief Re-allocate `count` number of items of `type` in memory
 * and return the pointer to the newly allocated memory
 */
#define reallocN(ptr, type, count) (type *)realloc(ptr, (count) * sizeof(type))

/**
 * @brief Determines whether a character `ch` is whitespace
 */
#define is_whitespace(ch) (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')

/**
 * @brief Guesses the element type at the start of a string
 */
static result(json_element_type) json_guess_element_type(typed(json_string));

/**
 * @brief Parses a JSON element value {json_element_value_t} based
 * on the `type` parameter passed and moves the string pointer
 * to end of the parsed element
 */
static result(json_element_value) json_parse_element_value(typed(json_string) *, typed(json_element_type));

/**
 * @brief Parses a `String` {json_string_t} and moves the string
 * pointer to the end of the parsed string
 */
static result(json_element_value) json_parse_string(typed(json_string) *);

/**
 * @brief Parses a `Number` {json_number_t} and moves the string
 * pointer to the end of the parsed number
 */
static result(json_element_value) json_parse_number(typed(json_string) *);

/**
 * @brief Parses a `Object` {json_object_t} and moves the string
 * pointer to the end of the parsed object
 */
static result(json_element_value) json_parse_object(typed(json_string) *);

/**
 * @brief Parses a `Array` {json_array_t} and moves the string
 * pointer to the end of the parsed array
 */
static result(json_element_value) json_parse_array(typed(json_string) *);

/**
 * @brief Parses a `Boolean` {json_boolean_t} and moves the string
 * pointer to the end of the parsed boolean
 */
static result(json_element_value) json_parse_boolean(typed(json_string) *);

/**
 * @brief Parses a JSON element {json_element_t} and moves the string
 * pointer to the end of the parsed element
 */
static result(json_entry) json_parse_entry(typed(json_string) *);

/**
 * @brief Prints a JSON element {json_element_t} type
 */
static void json_print_element(typed(json_element) *, int, int);

/**
 * @brief Prints a `String` {json_string_t} type
 */
static void json_print_string(typed(json_string));

/**
 * @brief Prints a `Number` {json_number_t} type
 */
static void json_print_number(typed(json_number));

/**
 * @brief Prints an `Object` {json_object_t} type
 */
static void json_print_object(typed(json_object) *, int, int);

/**
 * @brief Prints an `Array` {json_array_t} type
 */
static void json_print_array(typed(json_array) *, int, int);

/**
 * @brief Prints a `Boolean` {json_boolean_t} type
 */
static void json_print_boolean(typed(json_boolean));

/**
 * @brief Frees a `String` (json_string_t) from memory
 */
static void json_free_string(typed(json_string));

/**
 * @brief Frees an `Object` (json_object_t) from memory
 */
static void json_free_object(typed(json_object) *);

/**
 * @brief Frees an `Array` (json_array_t) from memory
 */
static void json_free_array(typed(json_array) *);

/**
 * @brief Moves a JSON string pointer beyond any whitespace
 */
static void json_skip_whitespace(typed(json_string) *);

/**
 * @brief Moves a JSON string pointer beyond `null` literal
 *
 */
static void json_skip_null(typed(json_string) *);

/**
 * @brief Utility function to convert an escaped string to a formatted string
 */
static result(json_string) json_unescape_string(typed(json_string), typed(size));

/**
 * @brief Find the length of null-terminated string
 */
static typed(size) json_string_len(typed(json_string));

result(json_element) json_parse(
    typed(json_string) json_str) {
    if (json_str == NULL) {
        return result_err(json_element)(JSON_ERROR_EMPTY);
    }

    typed(size) len = json_string_len(json_str);
    if (len == 0) {
        return result_err(json_element)(JSON_ERROR_EMPTY);
    }

    result_try(json_element, json_element_type, type, json_guess_element_type(json_str));
    result_try(json_element, json_element_value, value, json_parse_element_value(&json_str, type));

    const typed(json_element) element = {
        .type = type,
        .value = value,
    };

    return result_ok(json_element)(element);
}

result(json_element_type) json_guess_element_type(
    typed(json_string) str) {
    const char first_ch = *str;
    typed(json_element_type) type;

    if (first_ch == '"')
        type = JSON_ELEMENT_TYPE_STRING;
    else if ((first_ch >= '0' && first_ch <= '9') || first_ch == '+' || first_ch == '-')
        type = JSON_ELEMENT_TYPE_NUMBER;
    else if (first_ch == '{')
        type = JSON_ELEMENT_TYPE_OBJECT;
    else if (first_ch == '[')
        type = JSON_ELEMENT_TYPE_ARRAY;
    else if (first_ch == 't' || first_ch == 'f')
        type = JSON_ELEMENT_TYPE_BOOLEAN;
    else if (first_ch == 'n')
        type = JSON_ELEMENT_TYPE_NULL;
    else
        return result_err(json_element_type)(JSON_ERROR_INVALID_TYPE);

    return result_ok(json_element_type)(type);
}

result(json_element_value) json_parse_element_value(
    typed(json_string) * str_ptr,
    typed(json_element_type) type) {
    switch (type) {
        case JSON_ELEMENT_TYPE_STRING:
            return json_parse_string(str_ptr);
        case JSON_ELEMENT_TYPE_NUMBER:
            return json_parse_number(str_ptr);
        case JSON_ELEMENT_TYPE_OBJECT:
            return json_parse_object(str_ptr);
        case JSON_ELEMENT_TYPE_ARRAY:
            return json_parse_array(str_ptr);
        case JSON_ELEMENT_TYPE_BOOLEAN:
            return json_parse_boolean(str_ptr);
        case JSON_ELEMENT_TYPE_NULL:
            json_skip_null(str_ptr);
            return result_err(json_element_value)(JSON_ERROR_EMPTY);
    }
}

result(json_element_value) json_parse_string(
    typed(json_string) * str_ptr) {
    // Skip the first '"' character
    (*str_ptr)++;

    typed(size) len = json_string_len(*str_ptr);
    if (len == 0) {
        // Skip the end quote
        (*str_ptr)++;
        return result_err(json_element_value)(JSON_ERROR_EMPTY);
    }

    result_try(json_element_value, json_string, output, json_unescape_string(*str_ptr, len));

    // Skip to beyond the string
    (*str_ptr) += len + 1;

    return result_ok(json_element_value)((typed(json_element_value))output);
}

result(json_element_value) json_parse_number(
    typed(json_string) * str_ptr) {
    errno = 0;
    typed(json_number) number = strtod(*str_ptr, (char **)str_ptr);

    if (errno == EINVAL || errno == ERANGE)
        return result_err(json_element_value)(JSON_ERROR_INVALID_VALUE);

    return result_ok(json_element_value)((typed(json_element_value))number);
}

result(json_element_value) json_parse_object(
    typed(json_string) * str_ptr) {
    // Skip the first '{' character
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    if (**str_ptr == '}') {
        // Skip the end '}'
        (*str_ptr)++;
        return result_err(json_element_value)(JSON_ERROR_EMPTY);
    }

    typed(json_entry) *entries = NULL;
    typed(size) count = 0;

    while (**str_ptr != '\0') {
        // Skip any accidental whitespace
        json_scrape_whitespace(str_ptr);

        result(json_entry) entry_result = json_parse_entry(str_ptr);

        if (result_is_ok(json_entry)(&entry_result)) {
            count++;
            entries = reallocN(entries, typed(json_entry), count);
            typed(json_entry) entry = result_unwrap(json_entry)(&entry_result);
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
        return result_err(json_element_value)(JSON_ERROR_EMPTY);

    // Skip the '}' closing brace
    (*str_ptr)++;

    typed(json_object) *object = alloc(typed(json_object));
    object->count = count;
    object->entries = entries;

    return result_ok(json_element_value)((typed(json_element_value))object);
}

result(json_element_value) json_parse_array(
    typed(json_string) * str_ptr) {
    // Skip the starting '[' character
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    // Unfortunately the array is empty
    if (**str_ptr == ']') {
        // Skip the end ']'
        (*str_ptr)++;
        return result_err(json_element_value)(JSON_ERROR_EMPTY);
    }

    typed(size) count = 0;
    typed(json_element) *elements = NULL;

    while (**str_ptr != '\0') {
        json_scrape_whitespace(str_ptr);

        // Guess the type
        result(json_element_type) type_result = json_guess_element_type(*str_ptr);
        if (result_is_ok(json_element_type)(&type_result)) {
            typed(json_element_type) type = result_unwrap(json_element_type)(&type_result);

            // Parse the value based on guessed type
            result(json_element_value) value_result = json_parse_element_value(str_ptr, type);
            if (result_is_ok(json_element_value)(&value_result)) {
                typed(json_element_value) value = result_unwrap(json_element_value)(&value_result);

                count++;
                elements = reallocN(elements, typed(json_element), count);
                elements[count - 1].type = type;
                elements[count - 1].value = value;
            }

            json_scrape_whitespace(str_ptr);
        }

        // Reached the end
        if (**str_ptr == ']')
            break;

        // Skip the ','
        (*str_ptr)++;
    }

    // Skip the ']' closing array
    (*str_ptr)++;

    if (count == 0) {
        return result_err(json_element_value)(JSON_ERROR_EMPTY);
    }

    typed(json_array) *array = alloc(typed(json_array));
    array->count = count;
    array->elements = elements;

    return result_ok(json_element_value)((typed(json_element_value))array);
}

result(json_element_value) json_parse_boolean(
    typed(json_string) * str_ptr) {
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

    return result_ok(json_element_value)((typed(json_element_value))output);
}

result(json_entry) json_parse_entry(
    typed(json_string) * str_ptr) {
    result_try(json_entry, json_element_value, key, json_parse_string(str_ptr));
    json_scrape_whitespace(str_ptr);

    // Skip the ':' delimiter
    (*str_ptr)++;

    json_scrape_whitespace(str_ptr);

    result(json_element_type) type_result = json_guess_element_type(*str_ptr);
    if (result_is_err(json_element_type)(&type_result)) {
        free((void *)key.as_string);
        return result_map_err(json_entry, json_element_type, &type_result);
    }
    typed(json_element_type) type = result_unwrap(json_element_type)(&type_result);

    result(json_element_value) value_result = json_parse_element_value(str_ptr, type);
    if (result_is_err(json_element_value)(&value_result)) {
        free((void *)key.as_string);
        return result_map_err(json_entry, json_element_value, &value_result);
    }
    typed(json_element_value) value = result_unwrap(json_element_value)(&value_result);

    typed(json_entry) entry = {
        .key = key.as_string,
        .element = {
            .type = type,
            .value = value,
        },
    };

    return result_ok(json_entry)(entry);
}

void json_print(
    typed(json_element) * element,
    int indent) {
    json_print_element(element, indent, 0);
}

void json_print_element(
    typed(json_element) * element,
    int indent,
    int indent_level) {
    switch (element->type) {
        case JSON_ELEMENT_TYPE_STRING:
            json_print_string(element->value.as_string);
            break;
        case JSON_ELEMENT_TYPE_NUMBER:
            json_print_number(element->value.as_number);
            break;
        case JSON_ELEMENT_TYPE_OBJECT:
            json_print_object(element->value.as_object, indent, indent_level);
            break;
        case JSON_ELEMENT_TYPE_ARRAY:
            json_print_array(element->value.as_array, indent, indent_level);
            break;
        case JSON_ELEMENT_TYPE_BOOLEAN:
            json_print_boolean(element->value.as_boolean);
            break;
        case JSON_ELEMENT_TYPE_NULL:
            break;
            // Do nothing
    }
}

void json_print_string(
    typed(json_string) string) {
    printf("\"%s\"", string);
}

void json_print_number(
    typed(json_number) number) {
    printf("%f", number);
}

void json_print_object(
    typed(json_object) * object,
    int indent,
    int indent_level) {
    printf("{\n");

    for (int i = 0; i < object->count; i++) {
        for (int j = 0; j < indent * (indent_level + 1); j++)
            printf(" ");

        typed(json_entry) *entry = &object->entries[i];

        printf("\"%s\": ", entry->key);
        json_print_element(&entry->element, indent, indent_level + 1);

        if (i != object->count - 1)
            printf(",");
        printf("\n");
    }

    for (int j = 0; j < indent * indent_level; j++)
        printf(" ");
    printf("}");
}

void json_print_array(
    typed(json_array) * array,
    int indent,
    int indent_level) {
    printf("[\n");

    for (int i = 0; i < array->count; i++) {
        typed(json_element) element = array->elements[i];
        for (int j = 0; j < indent * (indent_level + 1); j++)
            printf(" ");
        json_print_element(&element, indent, indent_level + 1);

        if (i != array->count - 1)
            printf(",");
        printf("\n");
    }

    for (int i = 0; i < indent * indent_level; i++)
        printf(" ");
    printf("]");
}

void json_print_boolean(
    typed(json_boolean) boolean) {
    printf("%s", boolean ? "true" : "false");
}

void json_free(
    typed(json_element) * element) {
    switch (element->type) {
        case JSON_ELEMENT_TYPE_STRING:
            json_free_string(element->value.as_string);
            break;

        case JSON_ELEMENT_TYPE_OBJECT:
            json_free_object(element->value.as_object);
            break;

        case JSON_ELEMENT_TYPE_ARRAY:
            json_free_array(element->value.as_array);
            break;

        case JSON_ELEMENT_TYPE_NUMBER:
        case JSON_ELEMENT_TYPE_BOOLEAN:
        case JSON_ELEMENT_TYPE_NULL:
            // Do nothing
            break;
    }
}

void json_free_string(
    typed(json_string) string) {
    free((void *)string);
}

void json_free_object(
    typed(json_object) * object) {
    if (object == NULL)
        return;

    if (object->count == 0) {
        free(object);
        return;
    }

    for (int i = 0; i < object->count; i++) {
        typed(json_entry) *entry = &object->entries[i];

        free((void *)entry->key);
        json_free(&entry->element);
    }

    free(object->entries);
    free(object);
}

void json_free_array(
    typed(json_array) * array) {
    if (array == NULL)
        return;

    if (array->count == 0) {
        free(array);
        return;
    }

    // Recursively free each element in the array
    for (int i = 0; i < array->count; i++) {
        typed(json_element) element = array->elements[i];
        json_free(&element);
    }

    // Lastly free
    free(array->elements);
    free(array);
}

typed(json_string) json_error_to_string(
    typed(json_error) error) {
    switch (error) {
        case JSON_ERROR_EMPTY:
            return "Empty";
        case JSON_ERROR_INVALID_KEY:
            return "Invalid key";
        case JSON_ERROR_INVALID_TYPE:
            return "Invalid type";
        case JSON_ERROR_INVALID_VALUE:
            return "Invalid value";

        default:
            return "Unknown error";
    }
}

void json_skip_whitespace(
    typed(json_string) * str_ptr) {
    while (is_whitespace(**str_ptr))
        (*str_ptr)++;
}

void json_skip_null(
    typed(json_string) * str_ptr) {
    (*str_ptr) += 4;
}

typed(size) json_string_len(
    typed(json_string) str) {
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

    return len;
}

result(json_string) json_unescape_string(
    typed(json_string) str,
    typed(size) len) {
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
                    return result_err(json_string)(JSON_ERROR_INVALID_VALUE);
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

define_result_type(json_element_type);
define_result_type(json_element_value);
define_result_type(json_element);
define_result_type(json_entry);
define_result_type(json_string);
define_result_type(size);