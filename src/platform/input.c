#include <GLFW/glfw3.h>

#include "input.h"
#include "platform/window.h"

#include "core/config.h"
#include "core/log.h"
#include "core/types.h"


#define __KEY_EMPTY -1

#define KEYP_STORAGE_SIZE 8

static struct Input {
    Window* window;

    // Mouse cursor position
    f64 mouse_px;
    f64 mouse_py;
    // Mouse cursor delta
    f64 mouse_dx;
    f64 mouse_dy;

    // Storage for key pressing state
    int keyp_storage[KEYP_STORAGE_SIZE];
} self;


void input_init() {
    self.window = window_get();

    glfwGetCursorPos(self.window, &self.mouse_px, &self.mouse_py);
    self.mouse_dx = 0.0;
    self.mouse_dy = 0.0;

    for (int i = 0; i < KEYP_STORAGE_SIZE; i++) {
        self.keyp_storage[i] = __KEY_EMPTY;
    }
}

void input_destroy() {}


static bool __1st_iter = true;

void input_update() {
    double new_px, new_py, new_dx, new_dy;

    glfwGetCursorPos(self.window, &new_px, &new_py);
    new_dx = (new_px - self.mouse_px) / Config.WINDOW_WIDTH;
    new_dy = (new_py - self.mouse_py) / Config.WINDOW_HEIGHT;

    if (!__1st_iter) {
        self.mouse_dx = new_dx;
        self.mouse_dy = new_dy;
    }
    else __1st_iter = false;

    self.mouse_px = new_px;
    self.mouse_py = new_py;
}


/* Is key pressed once
*/
bool input_is_keyp(int key) {
    int state = glfwGetKey(self.window, key);

    if (state == GLFW_PRESS) {
        for (int i = 0; i < KEYP_STORAGE_SIZE; i++) {

            if (self.keyp_storage[i] == __KEY_EMPTY) {
                self.keyp_storage[i] = key;
                return true;
            }

            if (self.keyp_storage[i] == key) {
                return false;
            }
        }
        log_error("GLFW_PRESS - OUT OF LOOP! MORE THAN 8 KEYS AT ONCE ???");
    }
    else if (state == GLFW_RELEASE) {
        for (int i = 0; i < KEYP_STORAGE_SIZE; i++) {

            if (self.keyp_storage[i] == key) {
                self.keyp_storage[i] = __KEY_EMPTY;
                return false;
            }
        }
    }
    return false;
}


/* Is key repeated
*/
bool input_is_keyrp(int key) {
    return glfwGetKey(self.window, key) == GLFW_PRESS;
}


void input_get_mouse_delta(vec2 dest) {
    vec2 delta = {self.mouse_dx, self.mouse_dy};
    glm_vec2_copy(delta, dest);
}


bool cursor_is_visible() {
    return glfwGetInputMode(self.window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}


void cursor_set_visible(bool visible) {
    int mode;
    if (visible) mode = GLFW_CURSOR_NORMAL;
    else         mode = GLFW_CURSOR_DISABLED;

    glfwSetInputMode(self.window, GLFW_CURSOR, mode);
}
