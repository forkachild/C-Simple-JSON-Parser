#include <stdio.h>
#include <stdlib.h>

struct SubData {
    int value;
};

struct Data {
    int value;
    struct SubData *sub_data;
};

int main() {
    struct SubData *sub_data = (struct SubData *)malloc(sizeof(struct SubData));
    sub_data->value = 5;
    struct Data *data = (struct Data *)malloc(sizeof(struct Data));
    data->value = 1;
    data->sub_data = sub_data;

    printf("Before: %d\n", data->sub_data[0].value);

    struct SubData *temp = &data->sub_data[0];
    temp->value = 6;

    printf("After: %d\n", data->sub_data[0].value);

    return 0;
}
