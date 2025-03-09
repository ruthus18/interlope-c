#include "camera.h"
#include "config.h"
#include "gfx.h"
#include "editor.h"
#include "input_keys.h"
#include "scene.h"
#include "platform/input.h"
#include "platform/time.h"
#include "platform/window.h"


static void on_init__();
static void on_destroy__();
static void on_update__();


int main() {
    window_init();
    gfx_init();
    input_init();

    on_init__();

    while (!gfx_need_stop()) {
        window_poll_events();
        time_update();
        input_update();

        on_update__();

        window_swap_buffers();
        if (!WINDOW_VSYNC)  time_limit_framerate();
    }
    on_destroy__();

    gfx_destroy();
    window_destroy();
    return EXIT_SUCCESS;
}


/* ------ Application Logic ------ */

ObjectsDB* objdb;
Camera* cam;
Scene* scene;

bool is_editor_visible = false;
bool is_cursor_visible = false;


static
void on_init__() {
    cam = camera_create();
    camera_set_position(cam, (vec3){1.25, 1.7, 1.25});
    camera_set_rotation(cam, 0.0, 0.0);

    objdb = objdb_create_from("data/objects.toml");
    scene = scene_create_from("data/scenes/sovsh_demo.toml", objdb);

    editor_init();
    editor_set_scene(scene);

    cursor_set_visible(is_cursor_visible);
}


static
void on_destroy__() {
    editor_destroy();
    camera_destroy(cam);

    scene_destroy(scene);
    objdb_destroy(objdb);
}


static
void on_update__() {
    /* -- Controls -- */
    if (input_is_keyp(IN_KEY_ESC)) {
        gfx_stop();
    }
    else if (input_is_keyp(IN_KEY_F1)) {
        is_editor_visible = !is_editor_visible;
        
        is_cursor_visible = is_editor_visible;
        cursor_set_visible(is_cursor_visible);
    }
    else if (input_is_keyp(IN_KEY_TILDA)) {
        is_cursor_visible = !is_cursor_visible;
        cursor_set_visible(is_cursor_visible);
    }

    /* -- Player movement -- */
    if (!cursor_is_visible()) {
        bool w = input_is_keyrp(IN_KEY_W);
        bool s = input_is_keyrp(IN_KEY_S);
        bool a = input_is_keyrp(IN_KEY_A);
        bool d = input_is_keyrp(IN_KEY_D);

        camera_player_control(cam, w, s, a, d);
    }

    camera_upload_to_gfx(cam);
    scene_draw(scene);

    editor_update(is_editor_visible);
}
