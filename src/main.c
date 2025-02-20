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

static void on_assets_load__();
static void on_scene_create__();
static void on_scene_destroy__();


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
Scene* scene;

bool editor_visible = true;


static
void on_init__() {    
    cam = camera_create();
    camera_set_position(cam, (vec3){1.25, 1.7, 1.25});
    camera_set_rotation(cam, 0.0, 0.0);

    on_assets_load__();
    scene = scene_create();
    on_scene_create__();

    editor_init();
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
        editor_visible = !editor_visible;
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

    if (editor_visible)  editor_update();
}


static
void on_destroy__() {
    editor_destroy();
    camera_destroy(cam);

    scene_destroy(scene);
    on_scene_destroy__();
}


/* ------------------------------------------------------------------------ -*/
/* Game Content */
/* ------------------------------------------------------------------------ -*/

Object chair;
Object sovsh_floor;
Object sovsh_wall;
Object sovsh_ceil01;
Object sovsh_ceil02;

static
void on_assets_load__() {
    chair = (Object){
        "chair",
        model_load_file("chair01.glb"),
        texture_load_file("sov_furn02.dds")
    };
    sovsh_floor = (Object){
        "sovsh_floor",
        model_load_file("dungeon/floor_sov01.glb"),
        texture_load_file("dungeon/floor_sov01.dds")
    };
    sovsh_wall = (Object){
        "sovsh_wall",
        model_load_file("dungeon/wall_sov01.glb"),
        texture_load_file("dungeon/wall_sov01.dds")
    };
    sovsh_ceil01 = (Object){
        "sovsh_ceil01",
        model_load_file("dungeon/ceil_sov01.glb"),
        texture_load_file("dungeon/concrete01.dds")
    };
    sovsh_ceil02 = (Object){
        "sovsh_ceil02",
        model_load_file("dungeon/ceil_sov02.glb"),
        texture_load_file("dungeon/concrete01.dds")
    };
}


static
void on_scene_destroy__() {
    gfx_mesh_unload(chair.mesh);
    gfx_texture_unload(chair.texture);
    gfx_mesh_unload(sovsh_floor.mesh);
    gfx_texture_unload(sovsh_floor.texture);
    gfx_mesh_unload(sovsh_wall.mesh);
    gfx_texture_unload(sovsh_wall.texture);
    gfx_mesh_unload(sovsh_ceil01.mesh);
    gfx_texture_unload(sovsh_ceil01.texture);
    gfx_mesh_unload(sovsh_ceil02.mesh);
    gfx_texture_unload(sovsh_ceil02.texture);
}


static
void on_scene_create__() {
    /* Floor */
    scene_add_object(
        scene, &sovsh_floor,
        (vec3){1.25, 0.0, 1.25}, NULL, NULL
    );
    scene_add_object(
        scene, &sovsh_floor,
        (vec3){1.25, 0.0, 3.75}, NULL, NULL
    );
    scene_add_object(
        scene, &sovsh_floor,
        (vec3){3.75, 0.0, 1.25}, NULL, NULL
    );
    scene_add_object(
        scene, &sovsh_floor,
        (vec3){3.75, 0.0, 3.75}, NULL, NULL
    );
    scene_add_object(
        scene, &sovsh_floor,
        (vec3){6.25, 0.0, 1.25}, NULL, NULL
    );
    scene_add_object(
        scene, &sovsh_floor,
        (vec3){6.25, 0.0, 3.75}, NULL, NULL
    );
    /* Ceiling */
    scene_add_object(
        scene, &sovsh_ceil01,
        (vec3){1.25, 2.5, 1.25}, (vec3){0.0, 180.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_ceil01,
        (vec3){1.25, 2.5, 3.75}, (vec3){0.0, -90.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_ceil02,
        (vec3){3.75, 2.5, 1.25}, (vec3){0.0, 90.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_ceil02,
        (vec3){3.75, 2.5, 3.75}, (vec3){0.0, -90.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_ceil01,
        (vec3){6.25, 2.5, 1.25}, (vec3){0.0, 90.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_ceil01,
        (vec3){6.25, 2.5, 3.75}, (vec3){0.0, 0.0, 0.0}, NULL
    );
    /* Walls */
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){0.0, 0.0, 1.25}, (vec3){0.0, -90.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){0.0, 0.0, 3.75}, (vec3){0.0, -90.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){1.25, 0.0, 0.0}, (vec3){0.0, 180.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){3.75, 0.0, 0.0}, (vec3){0.0, 180.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){6.25, 0.0, 0.0}, (vec3){0.0, 180.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){1.25, 0.0, 5.0}, (vec3){0.0, 0.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){3.75, 0.0, 5.0}, (vec3){0.0, 0.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){6.25, 0.0, 5.0}, (vec3){0.0, 0.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){7.5, 0.0, 1.25}, (vec3){0.0, 90.0, 0.0}, NULL
    );
    scene_add_object(
        scene, &sovsh_wall,
        (vec3){7.5, 0.0, 3.75}, (vec3){0.0, 90.0, 0.0}, NULL
    );    
    
    // scene_add_object(
        //     scene, &chair,
        //     (vec3){0.5, 0.0, -3.0}, NULL, NULL
        // );
    }