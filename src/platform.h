#pragma once
#include <stdbool.h>
#include <GLFW/glfw3.h>


typedef GLFWwindow Window;

typedef struct Input {
    // Mouse cursor position
    double mouse_px;
    double mouse_py;
    // Mouse cursor delta
    double mouse_dx;
    double mouse_dy;
} Input;

typedef struct Platform {
    Window* window;
    Input* input;

    bool should_stop;
} Platform;


#define __on_draw_callback void (*on_draw_callback)()
typedef __on_draw_callback;


void platform_init();
void platform_destroy();
Platform* platform_get();
void platform_draw_frame(__on_draw_callback);
bool platform_should_stop();
void platform_stop();

bool cursor_is_visible();
void cursor_set_visible(bool visible);

bool input_is_keyp(int key);


/* ------------------------------------------------------------------------- */
/* Input Keys */


#define IN_KEY_ESC GLFW_KEY_ESCAPE
#define IN_KEY_TILDA GLFW_KEY_GRAVE_ACCENT
