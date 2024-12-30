#include <stdbool.h>

#include "camera.h"
#include "gfx.h"
#include "log.h"
#include "platform.h"

#include "input2.h"


void handle_input();
void on_update();


static Camera* camera;

static GfxMesh* meshes[1];
static mat4 mm_models[1];


void main() {
    platform_init();
    gfx_init();

    input2_init();

    camera = camera_create();
    camera_set_position(camera, (vec3){0.0, 1.5, 0.0});
    camera_set_rotation(camera, (vec3){-90.0, 0.0, 0.0});

    float plane_vtx[] = {1.0, 0.0, 1.0, -1.0, 0.0, -1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, -0.0, 1.0, -0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0};
    int plane_ind[] = {0, 1, 2, 0, 3, 1};

    meshes[0] = gfx_load_mesh(plane_vtx, plane_ind, true);
    glm_mat4_identity(mm_models[0]);

    while (!platform_should_stop()) {
        platform_draw_frame(&on_update);
    }

    gfx_unload_mesh(meshes[0]);

    camera_destroy(camera);
    gfx_destroy();
    platform_destroy();
}


void on_update() {
    handle_input();
    gfx_draw(camera, meshes, mm_models);
}


void handle_input() {
    if (input2_is_keyp(IN_KEY_ESC)) {
        platform_stop();
    }

    else if (input2_is_keyp(IN_KEY_TILDA)) {
        bool cur_visible = cursor_is_visible();
        cursor_set_visible(!cur_visible);
    }
}
