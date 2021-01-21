#define _GNU_SOURCE
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

struct {
    JSON_value* values;
    ssize_t len;
} typedef JSON_array;

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
            asprintf(&result, "%g", *((double *)v.data));
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
            JSON_array data = *((JSON_array*)v.data);
            ssize_t len = 3; // []\0
            result = malloc(len);
            ssize_t str_index = 1;
            result[0] = '[';
            for (int i = 0; i < data.len; i++) {
                JSON_value val = data.values[i];
                char *val_str = to_string(val);
                len += strlen(val_str);
                if (i != (data.len - 1)) {
                    len += 1; // add comma
                }
                result = realloc(result, len);
                memcpy(result + str_index, val_str, strlen(val_str));
                str_index += strlen(val_str);
                if (i != (data.len - 1)) {
                    result[str_index] = ',';
                    str_index += 1;
                }
                free(val_str);
            }
            result[str_index] = ']';
            result[str_index + 1] = '\0';
            break;
        }
        case JSON_data_type_bool: {
            if (v.data == NULL + 1) {
                result = malloc(5);
                strcpy(result, "true");
            } else if (v.data == NULL) {
                result = malloc(6);
                strcpy(result, "false");
            }
            break;
        }
        case JSON_data_type_null: {
            result = malloc(5);
            strcpy(result, "null");
        }
    }
    return result;
}

JSON_object parse_object(char **s) {
}

JSON_array parse_array(char **s) {
}

char * parse_string(char **s) {
    char *result = malloc(strlen(*s) + 1);
    char *current_char = result;
    for (; **s != '\0'; ++*s) {
        if (**s == '\\') {
            switch (*(++*s)) {
                case '\\':
                    *current_char = '\\';
                    break;
                case '"':
                    *current_char = '"';
                    break;
                case 'n':
                    *current_char = '\n';
                    break;
            }
        } else if (**s == '"') {
            current_char++;
            *current_char = '\0';
            return result;
        } else {
            *current_char = **s;
        }
        current_char++;
    }
    *current_char = '\0';
    return result;
}

double parse_number(char **s) {
    double d;
    sscanf(*s, "%lf", &d);
    for (; **s == '0' || **s == '1' || **s == '2' || **s == '3' || **s == '4' || **s == '5' || **s == '6' || **s == '7' || **s == '8' || **s == '9' || **s == '.'; ++*s);
    return d;
}

JSON_value parse_json(char **s) {
    JSON_value result;
    for (; **s != '\0'; ++*s) {
        switch (**s) {
            case '{': {
                // object
                ++*s;
                result.data_type = JSON_data_type_object;
                JSON_object obj = parse_object(s);
                memcpy(result.data, &obj, sizeof(JSON_object));
                return result;
            }
            case '[': { // array
                ++*s;
                result.data_type = JSON_data_type_array;
                JSON_array arr = parse_array(s);
                memcpy(result.data, &arr, sizeof(JSON_array));
                return result;
            }
            case '"': { // string
                ++*s;
                result.data_type = JSON_data_type_string;
                char *str = parse_string(s);
                result.data = str;
                return result;
            }
            case 'f': { // false
                result.data_type = JSON_data_type_bool;
                result.data = NULL;
                return result;
            }
            case 't': { // true
                result.data_type = JSON_data_type_bool;
                result.data = NULL + 1;
                return result;
            }
            case 'n': { // null
                result.data_type = JSON_data_type_null;
                return result;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': // number
                result.data_type = JSON_data_type_number;
                double d = parse_number(s);
                result.data = malloc(sizeof(double));
                memcpy(result.data, &d, sizeof(double));
                return result;
        }
    }
    // return a NULL if this isn't valid
    result.data_type = JSON_data_type_null;
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

    char *x = "123.4";
    JSON_value parse_example = parse_json(&x);

    printf("%s\n", to_string(parse_example));
    return 0;
}
