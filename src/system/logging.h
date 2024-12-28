#pragma once

// Terminal colors
#define TERM_RED      "\033[0;31m"
#define TERM_GREEN    "\033[0;32m"
#define TERM_CYAN_BG  "\033[0;104m"
#define TERM_RESET    "\033[0;0m"
// More: https://en.wikipedia.org/wiki/ANSI_escape_code#In_C

void log_greeting(const char* msg, ...);
void log_info(const char* msg, ...);
void log_success(const char* msg, ...);
void log_error(const char* msg, ...);

void print_engine_info();

/* OpenGL related */
void log_glshader(uint32_t shader);
void log_glprogram(uint32_t program);
