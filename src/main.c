#include <stdbool.h>

#include "camera.h"
#include "config.h"
#include "gfx.h"
#include "log.h"
#include "platform.h"
#include "cgm.h"


static void handle_user_control();


static Camera* camera;

static GfxMesh* meshes[1];
static mat4 mm_models[1];


float cube_vtx_buf[] = {
    -1.0, 2.0, 1.0, -1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 2.0, -1.0, 1.0, 0.0, -1.0, -1.0, 0.0, -1.0, 1.0, 2.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 2.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0, -1.0, -1.0, 2.0, -1.0, 1.0, 2.0, 1.0, 1.0, 2.0, -1.0, -1.0, 2.0, -1.0, 1.0, 2.0, -1.0, 1.0, 2.0, 1.0, -1.0, 2.0, 1.0, 1.0, 0.0, 1.0, -1.0, 2.0, 1.0,
    -1.0, -0.0, -0.0, -1.0, -0.0, -0.0, -1.0, -0.0, -0.0, -0.0, -0.0, -1.0, -0.0, -0.0, -1.0, -0.0, -0.0, -1.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -1.0, -0.0, -0.0, -1.0, -0.0, -0.0, -1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -1.0, -0.0, -0.0, -0.0, -0.0, -1.0, 1.0, -0.0, -0.0, -0.0, -0.0, 1.0, -0.0, -1.0, -0.0, -0.0, 1.0, -0.0,
    1.0, 0.0, 0.0, 1,0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
};
int cube_ind_buf[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 0, 18, 1, 3, 19, 4, 6, 20, 7, 9, 21, 10, 12, 22, 13, 15, 23, 16
};
int cube_vtx_count = 24;


void main() {
    gfx_init();
    input_init();

    log_info("%i", len(cube_vtx_buf));

    /* ------ */
    camera = camera_create();
    camera_set_position(camera, (vec3){0.0, 1.7, 0.0});

    // float vtx_buf[] = {};
    // int ind_buf[] = {};
    // int vtx_count = 0;

    GfxMesh* mesh;
    mat4 model_mat;

    mesh = gfx_mesh_load(cube_vtx_buf, cube_ind_buf, cube_vtx_count, false, "cube");
    cgm_model_mat(
        (vec3){0.0, -3.0, 0.0},  // pos
        (vec3){0.0, 0.0, 0.0},  // rot
        (vec3){1.0, 1.0, 1.0},  // sc
        model_mat
    );
    // GfxScene* scene = gfx_scene_create();
    /* ------ */

    while (!gfx_need_stop()) {
        glfwPollEvents();
        time_update();
        input_update();

        handle_user_control();

        gfx_draw(camera, mesh, model_mat);
    }

    gfx_mesh_unload(mesh);
    // gfx_scene_destroy(scene);
    camera_destroy(camera);

    input_destroy();
    gfx_destroy();
}


static
void handle_user_control() {
    if (input_is_keyp(IN_KEY_ESC)) {
        gfx_stop();
    }

    else if (input_is_keyp(IN_KEY_TILDA)) {
        bool cur_visible = cursor_is_visible();
        cursor_set_visible(!cur_visible);
    }

    if (!cursor_is_visible()) {
        camera_player_control(camera);
    }
}
