#include <stdbool.h>

#include "camera.h"
#include "config.h"
#include "gfx.h"
#include "log.h"
#include "platform.h"
#include "cgm.h"


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

Camera* cam;
mat4 m_model;
GfxMesh* mesh;


static
void on_init__() {
    float vtx_buf[] = {
        // vertices
        0.0, 0.0, 0.0,    0.0, 0.5, 0.0,    0.5, 0.5, 0.0,
        // normals
        1.0, 0.0, 0.0,    1.0, 0.0, 0.0,    1.0, 0.0, 0.0,
        // texcoords
        0.0, 0.0,         0.0, 0.0,         0.0, 0.0
    };
    int ind_buf[] = {};
    int vtx_count = 3;
    int ind_count = 0;
    mesh = gfx_mesh_load("cube", vtx_buf, ind_buf, vtx_count, ind_count, false);

    cam = camera_create();
    camera_set_position(cam, (vec3){0.0, 1.7, 0.0});

    cgm_model_mat((vec3){0.0, 1.0, 3.0}, NULL, NULL, m_model);
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
        bool w = input_is_keyrp(IN_KEY_W);
        bool s = input_is_keyrp(IN_KEY_S);
        bool a = input_is_keyrp(IN_KEY_A);
        bool d = input_is_keyrp(IN_KEY_D);

        camera_player_control(cam, w, s, a, d);
    }

    gfx_draw(&(cam->gfxd), mesh, m_model);
}


static
void on_destroy__() {
    gfx_mesh_unload(mesh);
    camera_destroy(cam);
}