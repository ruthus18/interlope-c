#include <stdbool.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "../config.h"
#include "../log.h"
#include "../platform.h"


int keyp_storage[8];  // TODO: place inside `Input`


Input* self;

void input_init(Window* window) {
    self = malloc(sizeof(Input));
    self->window = window;

    double px, py;
    glfwGetCursorPos(window, &px, &py);

    self->mouse_px = px;
    self->mouse_py = py;
    self->mouse_dx = 0.0;
    self->mouse_dy = 0.0;

    for (int i = 0; i < len(keyp_storage); i++) {
        keyp_storage[i] = -1;
    }
}


void input_destroy() {
    free(self);
}


bool __1st_iter = true;
double __new_px, __new_py, __new_dx, __new_dy;

void input_update() {
    glfwGetCursorPos(self->window, &__new_px, &__new_py);

    __new_dx = (__new_px - self->mouse_px) / WINDOW_WIDTH;
    __new_dy = (__new_py - self->mouse_py) / WINDOW_HEIGHT;

    if (!__1st_iter) {
        self->mouse_dx = __new_dx;
        self->mouse_dy = __new_dy;
    }
    else __1st_iter = false;

    self->mouse_px = __new_px;
    self->mouse_py = __new_py;
}


bool input_is_keyp(int key) {
    int state = glfwGetKey(self->window, key);

    if (state == GLFW_PRESS) {
        for (int i = 0; i < len(keyp_storage); i++) {

            if (keyp_storage[i] == -1) {
                keyp_storage[i] = key;
                return true;
            }

            if (keyp_storage[i] == key) {
                return false;
            }
        }
        log_error("GLFW_PRESS - OUT OF LOOP! MORE THAN 8 KEYS AT ONCE ???");
    }
    else if (state == GLFW_RELEASE) {
        for (int i = 0; i < len(keyp_storage); i++) {

            if (keyp_storage[i] == key) {
                keyp_storage[i] = -1;
                return false;
            }
        }
    }
    return false;
}


bool input_is_keyrp(int key) {

}


void input_get_mouse_delta(vec2 dest) {
    vec2 delta = {self->mouse_dx, self->mouse_dy};
    glm_vec2_copy(delta, dest);
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
