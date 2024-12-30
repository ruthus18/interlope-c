#include <stdbool.h>
#include <GLFW/glfw3.h>

#include "input2.h"
#include "log.h"
#include "platform.h"


#define len(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

static Window* window;

const int KP_SIZE = 8;
int keyp_storage[8];


void input2_init() {
    window = platform_get()->window;

    for (int i = 0; i < len(keyp_storage); i++) {
        keyp_storage[i] = -1;
    }
}

bool input2_is_keyp(int key) {
    int state = glfwGetKey(window, key);

    if (state == GLFW_PRESS) {
        for (int i = 0; i < KP_SIZE; i++) {

            if (keyp_storage[i] == -1) {
                keyp_storage[i] = key;
                return true;
            }

            if (keyp_storage[i] == key) {
                return false;
            }
        }
        log_error("GLFW_PRESS - OUT OF LOOP");
    }
    else if (state == GLFW_RELEASE) {
        for (int i = 0; i < KP_SIZE; i++) {

            if (keyp_storage[i] == key) {
                keyp_storage[i] = -1;
                return false;
            }
        }
    }
    return false;
}
