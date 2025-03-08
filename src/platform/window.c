#include <assert.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "window.h"
#include "../config.h"


static Window* window = NULL;


void window_init() {
    if (!glfwInit()) {
        printf("window_init: GLFW Initialization Error!\n");
        exit(EXIT_FAILURE);
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

    window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        WINDOW_TITLE,
        monitor,
        NULL
    );
    glfwSetWindowPos(window, WINDOW_XPOS, WINDOW_YPOS);
    glfwMakeContextCurrent(window);

    int glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        printf("window_init: GLEW Initialization Error!\n");
        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(WINDOW_VSYNC);
}

void window_destroy() {
    // TODO
}


Window* window_get() {
    assert(window != NULL);
    return window;
}

void window_poll_events() { 
    glfwPollEvents();
}

void window_swap_buffers() {
    glfwSwapBuffers(window);
}
