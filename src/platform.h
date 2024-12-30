#pragma once
#include <stdbool.h>
#include <GLFW/glfw3.h>

#include "platform/input_keys.h"


typedef GLFWwindow Window;

/* file.c */

const char* file_read_text(const char* path);
const char* path_to_shader(const char* file_path);
const char* path_to_asset(const char* rel_path);

/* input.c */

typedef struct Input {
    Window* window;

    // Mouse cursor position
    double mouse_px;
    double mouse_py;
    // Mouse cursor delta
    double mouse_dx;
    double mouse_dy;

    int keyp_storage[8];
} Input;

void input_init();
void input_destroy();
void input_update();
bool input_is_keyp(int key);
bool input_is_keyrp(int key);
void input_get_mouse_delta(vec2 dest);
bool cursor_is_visible();
void cursor_set_visible(bool visible);

/* time.c */

void time_update();
