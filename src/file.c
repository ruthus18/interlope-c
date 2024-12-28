#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"


const char* read_text_file(const char* path) {
    char* buffer = 0;
    long len;
    FILE * file = fopen(path, "rb");

    if (file) {
        fseek(file, 0, SEEK_END);
        len = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = (char*) malloc((len+1) * sizeof(char));
        if (buffer)
            fread(buffer, sizeof(char), len, file);
        
        fclose(file);
    }
    buffer[len] = '\0';
    return buffer;
}

const char* path_to_shader(const char* rel_path) {
    int size = strlen(DIR_SHADERS) + strlen(rel_path) + 1;

    char*  path = malloc(size);
    strcpy(path, DIR_SHADERS);
    strcat(path, rel_path);
    return path;
}

const char* path_to_asset(const char* rel_path) {
    int size = strlen(DIR_ASSETS) + strlen(rel_path) + 1;

    char*  path = malloc(size);
    strcpy(path, DIR_ASSETS);
    strcat(path, rel_path);
    return path;
}
