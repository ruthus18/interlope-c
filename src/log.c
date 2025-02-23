#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <cglm/cglm.h>

#include "log.h"


void log_greeting(const char* msg, ...) {
    fprintf(stdout, TERM_CYAN_BG);

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
    fprintf(stdout, TERM_GREEN);

    va_list argp;
    va_start(argp, msg);
    vfprintf(stdout, msg, argp);
    va_end(argp);

    fprintf(stdout, "%s\n", TERM_RESET);
}


void log_error(const char* msg, ...) {
    fprintf(stderr, TERM_RED);

    va_list argp;
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);

    fprintf(stderr, "%s\n", TERM_RESET);
}


void log_exit(const char* msg, ...) {
    fprintf(stderr, TERM_RED);

    va_list argp;
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);

    fprintf(stderr, "%s\n", TERM_RESET);

    exit(EXIT_FAILURE);
}
