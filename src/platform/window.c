#include <assert.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "window.h"

#include "core/log.h"
#include "core/config.h"


static Window* window = NULL;


void window_init() {
    if (!glfwInit()) {
        log_exit("(window_init) GLFW Initialization Error!");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    if (!Config.WINDOW_BORDER) {
        glfwWindowHint(GLFW_DECORATED, false);
    }

    GLFWmonitor* monitor = NULL;
    if (Config.WINDOW_FULLSC)
        monitor = glfwGetPrimaryMonitor();

    window = glfwCreateWindow(
        Config.WINDOW_WIDTH,
        Config.WINDOW_HEIGHT,
        Config.WINDOW_TITLE,
        monitor,
        NULL
    );
    if (!window)  log_exit("Unable to create GLFW window");

    glfwSetWindowPos(window, Config.WINDOW_XPOS, Config.WINDOW_YPOS);
    glfwMakeContextCurrent(window);

    int glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        log_exit("(window_init) GLEW Initialization Error!");
    }

    glfwSwapInterval(Config.WINDOW_VSYNC);
}

void window_destroy() {
    glfwDestroyWindow(window);
    glfwTerminate();
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
