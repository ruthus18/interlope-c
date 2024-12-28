#include <stdio.h>

#include "gfx.h"
#include "platform.h"



static
void draw() {
    if (input_is_keyp(IL_KEY_ESC)) {
        platform_stop();
    }

    gfx_draw();
}


void main() {
    platform_init();
    platform_log_info();
    gfx_init();

    while (!platform_should_stop()) {
        platform_draw_frame(&draw);
    }

    gfx_destroy();
    platform_destroy();
}
