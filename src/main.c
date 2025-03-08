#include "camera.h"
#include "gfx.h"
#include "editor.h"
#include "input_keys.h"
#include "scene.h"
#include "platform/input.h"
#include "platform/time.h"


static void on_init__();
static void on_destroy__();
static void on_update__();


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


/* ------ Application Logic ------ */

ObjectsDB objdb;
Camera* cam;
Scene* scene;

bool is_editor_visible = false;


static
void on_init__() {
    cam = camera_create();
    camera_set_position(cam, (vec3){1.25, 1.7, 1.25});
    camera_set_rotation(cam, 0.0, 0.0);

    objdb = objdb_create_from("objects_db.toml");
    scene = scene_create_from("scenes/sovsh_demo.toml", &objdb);
    scene_set_camera(scene, cam);

    editor_init();
    editor_set_scene(scene);

    cursor_set_visible(is_editor_visible);
}


static
void on_destroy__() {
    editor_destroy();
    camera_destroy(cam);

    scene_destroy(scene);
    objdb_destroy(&objdb);
}


static
void on_update__() {
    /* -- Controls -- */
    if (input_is_keyp(IN_KEY_ESC)) {
        gfx_stop();
    }
    else if (input_is_keyp(IN_KEY_TILDA)) {
        is_editor_visible = !is_editor_visible;

        cursor_set_visible(is_editor_visible);
    }

    /* -- Player movement -- */
    if (!cursor_is_visible()) {
        bool w = input_is_keyrp(IN_KEY_W);
        bool s = input_is_keyrp(IN_KEY_S);
        bool a = input_is_keyrp(IN_KEY_A);
        bool d = input_is_keyrp(IN_KEY_D);

        camera_player_control(cam, w, s, a, d);
    }

    gfx_draw(&scene->gfxd);
    editor_update(is_editor_visible);
}
