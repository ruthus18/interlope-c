#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../log.h"


const char* file_read_text(const char* path) {
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
    else {
        log_exit("Unable to read file: %s", path);
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


const char* path_to_model(const char* rel_path) {
    int size = strlen(DIR_MODELS) + strlen(rel_path) + 1;

    char*  path = malloc(size);
    strcpy(path, DIR_MODELS);
    strcat(path, rel_path);
    return path;
}


const char* path_to_texture(const char* rel_path) {
    int size = strlen(DIR_TEXTURES) + strlen(rel_path) + 1;

    char*  path = malloc(size);
    strcpy(path, DIR_TEXTURES);
    strcat(path, rel_path);
    return path;
}
