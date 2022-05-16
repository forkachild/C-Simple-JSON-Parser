#include "example.h"

#include <stdio.h>

#include "json.h"

int main() {
    json_object_t *object = json_parse(json);
    json_print(object, 2);
    json_free(object);
    return 0;
}
