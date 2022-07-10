#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

const char *read_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Expected file \"%s\" not found", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(len);

    if (buffer == NULL) {
        fprintf(stderr, "Unable to allocate memory for file");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, len, file);

    return (const char *)buffer;
}

int main() {
    const char *json = read_file("../multidim_arr.json");
    if (json == NULL) {
        return -1;
    }

    json_object_t *object = json_parse(json);
    free((void *)json);

    if (object == NULL) {
        return -1;
    }

    json_print(object, 2);
    json_free(object);

    return 0;
}
