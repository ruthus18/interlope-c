#include <stdbool.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "../config.h"
#include "../gfx.h"
#include "../log.h"
#include "../platform.h"


#define __KEY_EMPTY - 1


Input* self;

void input_init() {
    self = malloc(sizeof(Input));
    self->window = window_get();

    double px, py;
    glfwGetCursorPos(self->window, &px, &py);

    self->mouse_px = px;
    self->mouse_py = py;
    self->mouse_dx = 0.0;
    self->mouse_dy = 0.0;

    for (int i = 0; i < len(self->keyp_storage); i++) {
        self->keyp_storage[i] = __KEY_EMPTY;
    }
}


void input_destroy() {
    free(self);
}


bool __1st_iter = true;

void input_update() {
    double new_px, new_py, new_dx, new_dy;

    glfwGetCursorPos(self->window, &new_px, &new_py);
    new_dx = (new_px - self->mouse_px) / WINDOW_WIDTH;
    new_dy = (new_py - self->mouse_py) / WINDOW_HEIGHT;

    if (!__1st_iter) {
        self->mouse_dx = new_dx;
        self->mouse_dy = new_dy;
    }
    else __1st_iter = false;

    self->mouse_px = new_px;
    self->mouse_py = new_py;
}


/* Is key pressed once
*/
bool input_is_keyp(int key) {
    int state = glfwGetKey(self->window, key);

    if (state == GLFW_PRESS) {
        for (int i = 0; i < len(self->keyp_storage); i++) {

            if (self->keyp_storage[i] == __KEY_EMPTY) {
                self->keyp_storage[i] = key;
                return true;
            }

            if (self->keyp_storage[i] == key) {
                return false;
            }
        }
        log_error("GLFW_PRESS - OUT OF LOOP! MORE THAN 8 KEYS AT ONCE ???");
    }
    else if (state == GLFW_RELEASE) {
        for (int i = 0; i < len(self->keyp_storage); i++) {

            if (self->keyp_storage[i] == key) {
                self->keyp_storage[i] = __KEY_EMPTY;
                return false;
            }
        }
    }
    return false;
}


/* Is key repeated
*/
bool input_is_keyrp(int key) {
    return glfwGetKey(self->window, key) == GLFW_PRESS;
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