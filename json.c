#define _GNU_SOURCE
#include "json.h"
#define return_JSON_error() { JSON_value err; err.data_type = JSON_error; return err; }

char *JSON_to_string(JSON_value v) {
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
                char *val_str = JSON_to_string(key_val.value);
                len += strlen(val_str);
                if (i != (data.len - 1)) {
                    len += 1; // add comma
                }
                result = realloc(result, len);
                sprintf(result + str_index, "\"%s\":", key_val.key);
                str_index += strlen(key_val.key) + 3;
                memcpy(result + str_index, val_str, strlen(val_str));
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
                char *val_str = JSON_to_string(val);
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
        case JSON_error: {
            result = malloc(6);
            strcpy(result, "error");
        }
    }
    return result;
}

JSON_value parse_object(char **s) {
    JSON_object *result = (JSON_object*)malloc(sizeof(JSON_object));
    result->len = 0;
    result->values = malloc(sizeof(JSON_key_value));
    JSON_value final_result;
    final_result.data_type = JSON_data_type_object;
    final_result.data = result;
    while (**s != '}') {
        while (**s == ' ' || **s == ',') {
            ++*s;
        }
        // parse key
        JSON_value key = parse_json(s);
        if (key.data_type != JSON_data_type_string) {
            free_json_value(final_result);
            return_JSON_error();
        }
        char *k = key.data;
        if (**s != ':') {
            free_json_value(final_result);
            return_JSON_error();
        } else {
            ++*s;
        }
        JSON_value v = parse_json(s);
        if (v.data_type == JSON_error) {
            free_json_value(final_result);
            return v;
        }
        result->values[result->len].key = k;
        result->values[result->len].value = v;
        result->len++;
        result->values = realloc(result->values, sizeof(JSON_key_value) * (result->len + 1));
    }
    if (**s == '}') {
        ++*s;
    }
    return final_result;
}

JSON_value parse_array(char **s) {
    JSON_array *result = (JSON_array*)malloc(sizeof(JSON_array));
    result->len = 0;
    result->values = malloc(sizeof(JSON_value));
    JSON_value final_result;
    final_result.data_type = JSON_data_type_array;
    final_result.data = result;
    while (**s != ']') {
        while (**s == ' ' || **s == ',') {
            ++*s;
        }
        JSON_value x = parse_json(s);
        if (x.data_type == JSON_error) {
            free_json_value(final_result);
            return x;
        }
        result->values[result->len] = x;
        result->len++;
        result->values = realloc(result->values, sizeof(JSON_value) * (result->len + 1));
    }
    if (**s == ']') {
        ++*s;
    }
    return final_result;
}

char *parse_string(char **s) {
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
            ++*s;
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
    for (; ('0' <= **s && **s <= '9') || **s == '.'; ++*s);
    return d;
}

JSON_value parse_json(char **s) {
    JSON_value result;
    for (; **s != '\0'; ++*s) {
        switch (**s) {
            case '{': {
                // object
                ++*s;
                return parse_object(s);
            }
            case '[': { // array
                ++*s;
                return parse_array(s);
            }
            case '"': { // string
                ++*s;
                result.data_type = JSON_data_type_string;
                char *str = parse_string(s);
                result.data = str;
                return result;
            }
            case 'f': { // false
                if (strncmp(*s, "false", 5) == 0) {
                    *s += 5;
                    result.data_type = JSON_data_type_bool;
                    result.data = NULL;
                    return result;
                } else {
                    return_JSON_error();
                }
            }
            case 't': { // true
                if (strncmp(*s, "true", 4) == 0) {
                    *s += 4;
                    result.data_type = JSON_data_type_bool;
                    result.data = NULL + 1;
                    return result;
                } else {
                    return_JSON_error();
                }
            }
            case 'n': { // null
                if (strncmp(*s, "null", 4) == 0) {
                    *s += 4;
                    result.data_type = JSON_data_type_null;
                    return result;
                } else {
                    return_JSON_error();
                }
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

void free_json_value(JSON_value v) {
    switch (v.data_type) {
        case JSON_data_type_string: {
            free(v.data);
            break;
        }
        case JSON_data_type_object: {
            JSON_object data = *((JSON_object*)v.data);
            for (ssize_t i = 0; i < data.len; i++) {
                JSON_key_value kv = data.values[i];
                free(kv.key);
                free_json_value(kv.value);
            }
            free(data.values);
            free(v.data);
            break;
        }
        case JSON_data_type_array: {
            JSON_array data = *((JSON_array*)v.data);
            for (ssize_t i = 0; i < data.len; i++) {
                free_json_value(data.values[i]);
            }
            free(data.values);
            free(v.data);
            break;
        }
        case JSON_data_type_number:
            free(v.data);
            break;
    }
}
