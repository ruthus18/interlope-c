#include <stdlib.h>

#include "camera.h"
#include "config.h"
#include "gfx.h"
#include "editor.h"
#include "input_keys.h"
#include "physics.h"
#include "scene.h"
#include "platform/input.h"
#include "platform/time.h"
#include "platform/window.h"


static void game_on_init();
static void game_on_update();
static void game_on_destroy();


int main() {
    window_init();
    gfx_init();
    input_init();
    physics_init();

    game_on_init();

    while (!gfx_need_stop()) {
        window_poll_events();
        time_update();
        input_update();
        physics_update();

        game_on_update();

        window_swap_buffers();
        if (!WINDOW_VSYNC)  time_limit_framerate();
    }
    game_on_destroy();

    physics_destroy();
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


Object* door;
// Object* box;


static
void game_on_init() {
    cursor_set_visible(is_cursor_visible);

    cam = camera_create();
    camera_set_position(cam, (vec3){3.5, 1.7, 3.5});
    camera_set_rotation(cam, -135.0, 0.0);

    objdb = objdb_read_toml("data/objects_sov.toml");
    scene = scene_read_toml("data/scenes/sovsh_demo.toml", objdb);

    editor_init();
    editor_set_scene(scene);

    physics_create_ground();

    door = scene_find_object(scene, "sovsh_door_herm01");
    // box = scene_find_object(scene, "box");

    // vec3 box_pos, box_rot;
    // object_get_position(box, box_pos);
    // object_get_rotation(box, box_rot);

    // physics_create_cube(box_pos, box_rot, (vec3){1.0, 1.0, 1.0}, 5.0);
}


static
void game_on_destroy() {
    editor_destroy();
    camera_destroy(cam);

    scene_destroy(scene);
    objdb_destroy(objdb);
}


static float rot_angle = 0.0;
static bool rot_dir = false;

static
void _rotate_door() {
    if (rot_dir)        rot_angle += 0.25;
    else if (!rot_dir)  rot_angle -= 0.25;

    if (rot_angle <= -150.0)    rot_dir = true;
    else if (rot_angle >= 0.0)  rot_dir = false;

    object_set_subm_rotation(door, (vec3){0.0, rot_angle, 0.0}, 0);
}


static
void game_on_update() {
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

    _rotate_door();

    // vec3 cube_pos, cube_rot;
    // physics_get_cube_position(cube_pos);
    // object_set_position(box, cube_pos);

    // physics_get_cube_rotation(cube_rot);
    // object_set_rotation(box, cube_rot);

    // glm_vec3_print(cube_pos, stdout);
    // glm_vec3_print(cube_rot, stdout);

    camera_upload_to_gfx(cam);
    scene_draw(scene);

    editor_update(is_editor_visible);
}
