#pragma once
#include <stdbool.h>
#include <GLFW/glfw3.h>

#include "input_keys.h"


typedef GLFWwindow Window;


// file.c

const char* read_text_file(const char* path);
const char* path_to_shader(const char* file_path);
const char* path_to_asset(const char* rel_path);

// input.c

typedef struct Input {
    Window* window;

    // Mouse cursor position
    double mouse_px;
    double mouse_py;
    // Mouse cursor delta
    double mouse_dx;
    double mouse_dy;
} Input;

void input_init(Window* window);
void input_destroy();
void input_update();
bool input_is_keyp(int key);
bool input_is_keyrp(int key);
void input_get_mouse_delta(vec2 dest);
bool cursor_is_visible();
void cursor_set_visible(bool visible);

// time.c

void time_update();













// platform.c -- Main Interface

// typedef struct Platform {
//     Window* window;
//     Input* input;

//     bool should_stop;
// } Platform;

// #define __on_draw_callback void (*on_draw_callback)()
// typedef __on_draw_callback;


// void platform_init();
// void platform_destroy();
// Platform* platform_get();
// void platform_draw_frame(__on_draw_callback);
// bool platform_should_stop();
// void platform_stop();
