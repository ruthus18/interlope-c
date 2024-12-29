#include <stdio.h>

#include "camera.h"
#include "gfx.h"
#include "platform.h"


static Camera* camera;


static void draw() {
    if (input_is_keyp(IN_KEY_ESC)) {
        platform_stop();
    }
    gfx_draw(camera);
}

void main() {
    platform_init();
    gfx_init();

    camera = camera_create();
    camera_set_position(camera, (vec3){0.0, 1.7, 0.0});

    while (!platform_should_stop()) {
        platform_draw_frame(&draw);
    }

    camera_destroy(camera);
    gfx_destroy();
    platform_destroy();
}
