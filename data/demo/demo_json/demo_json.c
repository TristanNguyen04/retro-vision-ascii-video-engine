#include "parsers/json.h"

#include <stdio.h>

void print_json_object(const JsonObject * obj){
    size_t i;

    if(obj == NULL){
        return;
    }

    printf("pair_count: %lu\n", (unsigned long) obj->count);

    for(i = 0U; i < obj->count; i++){
        printf("pair[%lu]\n", (unsigned long) i);
        printf("    key: %s\n", obj->pairs[i].key);

        if(obj->pairs[i].type == JSON_VALUE_STRING){
            printf("    type: string\n");
            printf("    value: %s\n", obj->pairs[i].string_value);
        } else if(obj->pairs[i].type == JSON_VALUE_INT) {
            printf("    type: int\n");
            printf("    value: %d\n", obj->pairs[i].int_value);
        } else {
            printf("    type: unknown\n");
        }
    }
}

int main(int argc, char **argv) {
    JsonError err;
    JsonObject json_obj;

    if (argc != 2) {
        printf("Usage: %s <json_file>\n", argv[0]);
        return 1;
    }

    json_init(&json_obj);

    err = json_load(argv[1], &json_obj);

    printf("error: %s\n", json_error_string(err));

    if (err == JSON_OK) {
        print_json_object(&json_obj);
    }

    json_free(&json_obj);
    return (err == JSON_OK) ? 0 : 1;
}