#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <cjson/cJSON.h>
#include "configuration.h"

// Function to load configuration from JSON file
int load_config(const char* filename, Config* config) {
    char* buffer = NULL;
    int file_size;
    cJSON* json;

    // Read the JSON file
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening JSON file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        perror("Memory allocation error");
        return -1;
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    // Parse the JSON
    json = cJSON_Parse(buffer);
    free(buffer);

    if (!json) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        return -1;
    }

    // Populate config structure
    cJSON* port_json = cJSON_GetObjectItemCaseSensitive(json, "port");
    cJSON* address_json = cJSON_GetObjectItemCaseSensitive(json, "address");

    if (cJSON_IsNumber(port_json)) {
        config->port = port_json->valueint;
    } else {
        cJSON_Delete(json);
        return -1;
    }

    if (cJSON_IsString(address_json) && (address_json->valuestring != NULL)) {
        strncpy(config->address, address_json->valuestring, sizeof(config->address));
    } else {
        cJSON_Delete(json);
        return -1;
    }

    cJSON_Delete(json);
    return 0;
}