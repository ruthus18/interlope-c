#include <stdlib.h>

#include "./core/config.h"
#include "./editor/sys_geometry.h"
#include "./editor/ui.h"
#include "./platform/input.h"
#include "./platform/time.h"
#include "./platform/window.h"
#include "./render/gfx.h"
#include "./world/world.h"
#include "./physics.h"
#include "gameplay/player.h"


static void __game_on_init__();
static void __game_on_destroy__();
static void __game_on_update__();
static void __game_on_draw__();


int main() {
    window_init();
    input_init();
    gfx_init();
    physics_init();
    world_init();

    __game_on_init__();

    while (!gfx_need_stop()) {
        window_poll_events();
        time_update();
        input_update();
        physics_update();
        world_update();
        __game_on_update__();

        world_draw();   
        __game_on_draw__();

        window_swap_buffers();
        if (!WINDOW_VSYNC)  time_limit_framerate();
    }
    __game_on_destroy__();

    world_destroy();
    physics_destroy();
    gfx_destroy();
    window_destroy();
    return EXIT_SUCCESS;
}


void __exit() {
    gfx_stop();
}


/* ------ Application Logic ------ */


bool is_editor_visible = false;
bool is_cursor_visible = false;


static
void __game_on_init__() {
    editor_init();
    editor_geometry_init();

    cursor_set_visible(is_cursor_visible);
}


static
void __game_on_destroy__() {    
    editor_geometry_destroy();
    editor_destroy();
}


static
void __game_on_update__() {
    if (input_is_keyp(IN_KEY_ESC)) __exit();

    // Switch game/editor mode
    else if (input_is_keyp(IN_KEY_F1)) {
        is_editor_visible = !is_editor_visible;
        is_cursor_visible = is_editor_visible;

        cursor_set_visible(is_cursor_visible);
        player_set_is_active(!is_cursor_visible);
    }

    // Switch cursor/camera mouse control
    else if (input_is_keyp(IN_KEY_F2)) {
        is_cursor_visible = !is_cursor_visible;

        cursor_set_visible(is_cursor_visible);
        player_set_is_active(!is_cursor_visible);
    }
}


static
void __game_on_draw__() {
    editor_geometry_draw();
    editor_update(is_editor_visible);
}
