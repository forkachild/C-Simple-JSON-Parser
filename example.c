#include "example.h"

#include <stdio.h>
#include <string.h>

#include "json.h"

int main(int argc, char **argv) {
    if (argc == 2) {
        json_object_t *object = json_parse(argv[1]);
        json_print(object, 2);
        json_free(object);
    } else {
        json_object_t *object = json_parse(json);

        json_print(object, 2);
        json_object_t *data = object->entries[1].value.as_object;
        json_array_t *parts = data->entries[1].value.as_array;

        if (parts->count != data->entries[0].value.as_number) {
            return -1;
        }

        for (int i = 0; i < parts->count; i++) {
            json_object_t *part = parts->values[i].as_object;

            for (size_t j = 0; j < part->count - 1; j++) {
                if (!strcmp("DataSheetUrl", part->entries[j].key)) {
                    printf("Datasheet url: %s\n", part->entries[j].value.as_string);
                }
            }
        }

        json_free(object);
    }

    return 0;
}
