#include "camera.h"
#include "gfx.h"
#include "editor.h"
#include "input_keys.h"
#include "model.h"
#include "scene.h"
#include "texture.h"
#include "platform/input.h"
#include "platform/time.h"


static void on_init__();
static void on_update__();
static void on_destroy__();


int main() {
    gfx_init();
    input_init();

    on_init__();

    while (!gfx_need_stop()) {
        gfx_begin_draw();
        time_update();
        input_update();

        on_update__();
        gfx_end_draw();
    }
    on_destroy__();

    gfx_destroy();
    return EXIT_SUCCESS;
}

/* ----------------------- */

Camera* cam;
Object chair;

Scene* scene;

bool editor_enabled = true;


static
void on_init__() {
    editor_init();

    cam = camera_create();
    camera_set_position(cam, (vec3){0.0, 1.7, 0.0});

    chair = (Object){
        "chair",
        model_load_file("chair01.glb"),
        texture_load_file("sov_furn02.dds")
    };

    scene = scene_create();
    scene_add_object(scene, &chair, (vec3){-0.5, 0.0, -3.0}, NULL, NULL);
    scene_add_object(scene, &chair, (vec3){0.5, 0.0, -3.0}, NULL, NULL);
    editor_set_scene(scene);

    cursor_set_visible(false);
}


static
void on_update__() {
    /* -- Controls -- */
    if (input_is_keyp(IN_KEY_ESC)) {
        gfx_stop();
    }
    else if (input_is_keyp(IN_KEY_TILDA)) {
        bool cur_visible = cursor_is_visible();
        cursor_set_visible(!cur_visible);
    }
    else if (input_is_keyp(IN_KEY_F1)) {
        editor_enabled = !editor_enabled;
    }

    /* -- Player movement -- */
    if (!cursor_is_visible()) {
        bool w = input_is_keyrp(IN_KEY_W);
        bool s = input_is_keyrp(IN_KEY_S);
        bool a = input_is_keyrp(IN_KEY_A);
        bool d = input_is_keyrp(IN_KEY_D);

        camera_player_control(cam, w, s, a, d);
    }

    scene_draw(scene, cam);

    if (editor_enabled)  editor_update();
}


static
void on_destroy__() {
    scene_destroy(scene);

    gfx_mesh_unload(chair.mesh);
    gfx_texture_unload(chair.texture);
    camera_destroy(cam);

    editor_destroy();
}