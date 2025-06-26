#include <stdlib.h>

#include "engine.h"
#include "editor/geometry.h"
#include "gameplay/player.h"
#include "platform/input.h"
#include "platform/time.h"
#include "render/gfx.h"

#include "assets/font.h"


bool is_editor_visible = false;
bool is_cursor_visible = false;

GfxUI* ui_data;


static
void on_init() {
    editor_geometry_init();
    cursor_set_visible(is_cursor_visible);

    font_load_default();
    ui_data = gfx_load_ui_data();
}


static
void on_destroy() {
    font_unload_default();
    gfx_unload_ui_data(ui_data);
    editor_geometry_destroy();
}


static
void on_update() {
    if (input_is_keyp(IN_KEY_ESC))
        engine_exit();

    // Switch game/editor mode
    else if (input_is_keyp(IN_KEY_F1)) {
        is_editor_visible = !is_editor_visible;
        is_cursor_visible = is_editor_visible;

        cursor_set_visible(is_cursor_visible);
        player_set_active(!is_cursor_visible);
    }

    // Switch cursor/camera mouse control
    else if (input_is_keyp(IN_KEY_F2)) {
        is_cursor_visible = !is_cursor_visible;

        cursor_set_visible(is_cursor_visible);
        player_set_active(!is_cursor_visible);
    }
}


static
void on_draw() {
    gfx_begin_draw_ui();

    char fps[8];
    int fps_ = time_get_fps();
    sprintf(fps, "%d", fps_);

    gfx_draw_ui(fps, ui_data, (vec2){0.95, 0.04}, (vec3){1.0, 1.0, 0.0});
    gfx_end_draw_ui();

    editor_geometry_draw();
}

/* ------------------------------------------------------------------------- */

int main() {
    engine_set_callback(on_init, ENGINE_ON_INIT);
    engine_set_callback(on_destroy, ENGINE_ON_DESTROY);
    engine_set_callback(on_update, ENGINE_ON_UPDATE);
    engine_set_callback(on_draw, ENGINE_ON_DRAW);

    engine_run();
    return EXIT_SUCCESS;
}
