#include <stdbool.h>

#include "camera.h"
#include "gfx.h"
#include "log.h"
#include "platform.h"
#include "cgm.h"


static void handle_user_control();


static Camera* camera;

static GfxMesh* meshes[1];
static mat4 mm_models[1];


void main() {
    gfx_init();
    Window* window = gfx_get_window();
    input_init(window);

    /* ------ */
    camera = camera_create();
    camera_set_position(camera, (vec3){0.0, 1.5, 0.0});
    // camera_set_rotation(camera, (vec3){0.0, 0.0, 0.0});

    float plane_vtx[] = {1.0, 0.0, 1.0, -1.0, 0.0, -1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0};
    int plane_ind[] = {0, 1, 2, 0, 3, 1};

    meshes[0] = gfx_load_mesh(plane_vtx, plane_ind, true);
    cgm_model_mat(
        (vec3){0.0, 0.0, 0.0},
        (vec3){0.0, 0.0, 0.0},
        (vec3){1.0, 1.0, 1.0},
        mm_models[0]
    );
    /* ------ */

    while (!gfx_need_stop()) {
        glfwPollEvents();
        time_update();
        input_update();

        handle_user_control();

        gfx_draw(camera, meshes, mm_models);
    }

    gfx_unload_mesh(meshes[0]);

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
