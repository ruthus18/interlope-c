#include <stdbool.h>

#include "camera.h"
#include "config.h"
#include "gfx.h"
#include "log.h"
#include "platform.h"
#include "cgm.h"


static void handle_user_control();
static void on_init__();
static void on_update__();
static void on_destroy__();


void main() {
    gfx_init();
    input_init();

    on_init__();

    while (!gfx_need_stop()) {
        glfwPollEvents();
        time_update();
        input_update();

        on_update__();
    }

    on_destroy__();
    input_destroy();
    gfx_destroy();
}

/* ----------------------- */

Camera* camera;
mat4 m_model;

vec3 pos = {0.0, 1.5, 3.0};


static
void on_init__() {
    camera = camera_create();
    camera_update_gfx_data(camera);

    glm_mat4_identity(m_model);
}


static
void on_update__() {
    if (input_is_keyp(IN_KEY_ESC)) {
        gfx_stop();
    }

    else if (input_is_keyp(IN_KEY_TILDA)) {
        bool cur_visible = cursor_is_visible();
        cursor_set_visible(!cur_visible);
    }

    if (!cursor_is_visible()) {
        camera_player_control(camera);
        camera_update_gfx_data(camera);
    }

    gfx_draw(&(camera->gfxd), pos, m_model);
}


static
void on_destroy__() {
    camera_destroy(camera);
}