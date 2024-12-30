#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "log.h"
#include "config.h"


void log_greeting(const char* msg, ...) {
    fprintf(stdout, "%s", TERM_CYAN_BG);

    va_list argp;
    va_start(argp, msg);
    vfprintf(stdout, msg, argp);
    va_end(argp);

    fprintf(stdout, "%s\n", TERM_RESET);
}


void log_info(const char* msg, ...) {
    va_list argp;
    va_start(argp, msg);
    vfprintf(stdout, msg, argp);
    va_end(argp);
    fprintf(stdout, "\n");
}


void log_success(const char* msg, ...) {
    fprintf(stdout, "%s[*] ", TERM_GREEN);

    va_list argp;
    va_start(argp, msg);
    vfprintf(stdout, msg, argp);
    va_end(argp);

    fprintf(stdout, "%s\n", TERM_RESET);
}


void log_error(const char* msg, ...) {
    fprintf(stderr, "%s[X] ", TERM_RED);

    va_list argp;
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);

    fprintf(stderr, "%s\n", TERM_RESET);
}

void log_glshader(uint32_t shader) {
    int log_len = 0;
    int ch = 0;
    char *log;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        log = (char*) malloc(log_len);

        glGetShaderInfoLog(shader, log_len, &ch, log);
        log_info("Shader log: %s", log);
        free(log);
    }
}

void log_glprogram(uint32_t program) {
    int log_len = 0;
    int ch = 0;
    char *log;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        log = (char*) malloc(log_len);

        glGetProgramInfoLog(program, log_len, &ch, log);
        log_info("Program log: %s", log);
        free(log);
    }
}


void log_mat4(mat4 src) {
    int i,j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            printf("%f   ", src[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
