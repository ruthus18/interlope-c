#include <assert.h>
#include "window.h"


static Window* self = NULL;


void window_set(Window* win) {
    self = win;
}

Window* window_get() {
    assert(self != NULL);
    return self;
}
