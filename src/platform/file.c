#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

#include "core/log.h"


const char* _file_read_text(const char* path) {
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


const char* _path_construct(const char* base, const char* child) {
    int size = strlen(base) + strlen(child) + 1;

    char*  path = malloc(size);
    strcpy(path, base);
    strcat(path, child);
    return path;
}
