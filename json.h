#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

enum {
    JSON_data_type_string,
    JSON_data_type_number,
    JSON_data_type_object,
    JSON_data_type_array,
    JSON_data_type_bool,
    JSON_data_type_null,
    JSON_error // return type when there's an error
} typedef JSON_data_type;

struct {
    JSON_data_type data_type;
    void *data;
} typedef JSON_value;

struct {
    char* key;
    JSON_value value;
} typedef JSON_key_value;

struct {
    JSON_key_value* values;
    ssize_t len;
} typedef JSON_object;

struct {
    JSON_value* values;
    ssize_t len;
} typedef JSON_array;

char *JSON_to_string(JSON_value);
JSON_value parse_json(char **);
void free_json_value(JSON_value v);
