#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
  char *buffer = malloc(len + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Unable to allocate memory for file");
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, len, file);
  buffer[len] = '\0';

  return (const char *)buffer;
}

int main(void) {
  const char *json = read_file("../sample/reddit.json");
  if (json == NULL) {
    return -1;
  }

  clock_t start, end;
  start = clock();
  result(json_element) element_result = json_parse(json);
  end = clock();

  printf("Time taken %fs\n", (double)(end - start) / (double)CLOCKS_PER_SEC);

  free((void *)json);

  if (result_is_err(json_element)(&element_result)) {
    typed(json_error) error = result_unwrap_err(json_element)(&element_result);
    fprintf(stderr, "Error parsing JSON: %s\n", json_error_to_string(error));
    return -1;
  }
  typed(json_element) element = result_unwrap(json_element)(&element_result);

  // json_print(&element, 2);
  json_free(&element);

  return 0;
}
