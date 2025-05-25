#include <stdlib.h>

#include "./core/config.h"
#include "./world/scene_reader.h"
#include "./world/objdb_reader.h"
#include "./editor/sys_geometry.h"
#include "./editor/ui.h"
#include "./gameplay/player.h"
#include "./platform/input.h"
#include "./platform/time.h"
#include "./platform/window.h"
#include "./render/gfx.h"
#include "./world/scene.h"
#include "./physics.h"


static void game_on_init();
static void game_on_update();
static void game_on_destroy();


int main() {
    window_init();
    input_init();
    gfx_init();
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
Scene* scene;

bool is_editor_visible = false;
bool is_cursor_visible = false;


Object* door;


void _init_sovsh_scene() {
    objdb = objdb_read_toml("data/objects_sov.toml");
    scene = scene_read_toml("data/scenes/sovsh_demo.toml", objdb);

    door = scene_find_object(scene, "sovsh_door_herm01");
}


void _init_physics_scene() {
    objdb = objdb_read_toml("data/objects.toml");
    scene = scene_read_toml("data/scenes/cube_test.toml", objdb);
}


static
void game_on_init() {    
    _init_sovsh_scene();
    // _init_physics_scene();

    player_init((vec3){3.5, 0.0, 3.5}, -135.0, 0.0);

    editor_init();
    editor_set_scene(scene);
    editor_geometry_init();

    cursor_set_visible(is_cursor_visible);
}


static
void game_on_destroy() {    
    editor_geometry_destroy();
    editor_destroy();
    player_destroy();

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
        player_update();
    }

    scene_update(scene);
    scene_draw(scene);

    editor_geometry_draw();
    editor_update(is_editor_visible);
}
