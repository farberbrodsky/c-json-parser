#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _GNU_SOURCE

enum {
    JSON_data_type_string,
    JSON_data_type_number,
    JSON_data_type_object,
    JSON_data_type_array,
    JSON_data_type_boolean,
    JSON_data_type_null
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

char * to_string(JSON_value v) {
    char *result;
    switch (v.data_type) {
        case JSON_data_type_string: {
            result = malloc(strlen(v.data) + 3);
            result[0] = '"';
            result[strlen(v.data) + 1] = '"';
            result[strlen(v.data) + 2] = '\0';
            memcpy(result + 1, v.data, strlen(v.data));
            break;
        }
        case JSON_data_type_number: {
            asprintf(&result, "%f", *((double *)v.data));
            break;
        }
        case JSON_data_type_object: {
            JSON_object data = *((JSON_object*)v.data);
            ssize_t len = 3; // {}\0
            result = malloc(len);
            ssize_t str_index = 1;
            result[0] = '{';
            for (int i = 0; i < data.len; i++) {
                JSON_key_value key_val = data.values[i];
                len += strlen(key_val.key) + 3; // "key":
                char *val_str = to_string(key_val.value);
                len += strlen(val_str);
                if (i != (data.len - 1)) {
                    len += 1; // add comma
                }
                result = realloc(result, len);
                sprintf(result + str_index, "\"%s\":", key_val.key);
                str_index += strlen(key_val.key) + 3;
                strcpy(result + str_index, val_str);
                str_index += strlen(val_str);
                if (i != (data.len - 1)) {
                    result[str_index] = ',';
                    str_index += 1;
                }
                free(val_str);
            }
            result[str_index] = '}';
            result[str_index + 1] = '\0';
            break;
        }
        case JSON_data_type_array: {
            break;
        }
        case JSON_data_type_boolean: {
            break;
        }
        case JSON_data_type_null: {
            break;
        }
    }
    return result;
}

// TODO: function to free a json value

int main() {
    JSON_value my_first_str;
    my_first_str.data = malloc(6);
    strcpy(my_first_str.data, "world");
    my_first_str.data_type = JSON_data_type_string;

    JSON_key_value first_key_value;
    first_key_value.key = malloc(6);
    strcpy(first_key_value.key, "Hello");
    first_key_value.value = my_first_str;
    JSON_key_value *my_key_values = malloc(sizeof(JSON_key_value) * 2);

    my_key_values[0] = first_key_value;
    my_key_values[1] = first_key_value;

    JSON_object my_obj;
    my_obj.len = 2;
    my_obj.values = my_key_values;

    JSON_value my_value;
    my_value.data_type = JSON_data_type_object;
    my_value.data = &my_obj;

    printf("%s\n", to_string(my_value));
    return 0;
}
