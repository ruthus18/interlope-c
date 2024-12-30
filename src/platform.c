/*
    platform.c - handling 
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "config.h"
#include "log.h"
#include "platform.h"
#include "time.h"


/* Window Implementation */

static
Window* window_create() {
    if (!glfwInit()) {
        printf("GLFW Initialization Error!\n");
        exit(0);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (!WINDOW_BORDER) {
        glfwWindowHint(GLFW_DECORATED, false);
    }

    GLFWmonitor* monitor = NULL;
    if (WINDOW_FULLSC)
        monitor = glfwGetPrimaryMonitor();

    Window* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        WINDOW_TITLE,
        monitor,
        NULL
    );
    printf("Window created\n");

    glfwSetWindowPos(window, WINDOW_XPOS, WINDOW_YPOS);
    glfwMakeContextCurrent(window);

    int glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        printf("GLEW Initialization Error!\n");
        exit(0);
    }

    glfwSwapInterval(WINDOW_VSYNC);
    return window;
}

static
void window_destroy(Window* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

/* ------------------------------------------------------------------------ */
/* Input Implementation */

static
Input* input_create(Window* window) {
    Input* input = malloc(sizeof(Input));

    double px, py;
    glfwGetCursorPos(window, &px, &py);

    input->mouse_px = px;
    input->mouse_py = py;
    input->mouse_dx = 0.0;
    input->mouse_dy = 0.0;
    return input;
}

static
void input_destroy(Input* input) {
    free(input);
}


bool __skip = true;
double __new_px, __new_py, __new_dx, __new_dy;

static
void input_update(Window* window, Input* input) {
    glfwGetCursorPos(window, &__new_px, &__new_py);

    __new_dx = (__new_px - input->mouse_px) / WINDOW_WIDTH;
    __new_dx = (__new_py - input->mouse_py) / WINDOW_HEIGHT;

    if (!__skip) {
        input->mouse_dx = __new_dx;
        input->mouse_dy = __new_dy;
    }
    else __skip = false;

    input->mouse_px = __new_px;
    input->mouse_py = __new_py;
}

static
bool _input_is_keyp(Window* window, int key) {
    return (glfwGetKey(window, key) == GLFW_PRESS);
}


/* ------------------------------------------------------------------------ */
/* Platform Implementation */

static Platform* self;


static
void _log_platform_info() {
    log_greeting("======  Interlope Engine  ======");
    log_info("ENGINE VERSION: %s", ENGINE_VERSION);
    log_info("OPENGL VERSION: %s", glGetString(GL_VERSION));
    log_info("GLEW VERSION: %s", glewGetString(GLEW_VERSION));
    log_info("GLFW VERSION: %u.%u.%u", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR);
    log_info("VIDEO DEVICE: %s (%s)", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    log_info("WIN SIZE: %i x %i", WINDOW_WIDTH, WINDOW_HEIGHT);
    log_info("WIN VSYNC: %i", WINDOW_VSYNC);
    log_info("------");
}

void platform_init() {
    self = malloc(sizeof(Platform));
    self->window = window_create();
    self->input = input_create(self->window);
    _log_platform_info();
}

void platform_destroy() {
    input_destroy(self->input);
    window_destroy(self->window);
    free(self);
}

Platform* platform_get() {
    return self;
}

void platform_draw_frame(__on_draw_callback) {
    time_update();
    glfwPollEvents();
    input_update(self->window, self->input);

    on_draw_callback();
    glfwSwapBuffers(self->window);
}

bool platform_should_stop() {
    glfwWindowShouldClose(self->window) || self->should_stop;
}

void platform_stop() {
    self->should_stop = true;
}

bool cursor_is_visible() {
    return glfwGetInputMode(self->window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

void cursor_set_visible(bool visible) {
    int mode;
    if (visible) mode = GLFW_CURSOR_NORMAL;
    else         mode = GLFW_CURSOR_DISABLED;

    glfwSetInputMode(self->window, GLFW_CURSOR, mode);
}


/* ------------------------------------------------------------------------ */

bool input_is_keyp(int key) {
    return _input_is_keyp(self->window, key);
}
