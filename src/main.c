#include <stdlib.h>

#include "core/log.h"
#include "editor/geometry.h"
#include "gameplay/player.h"
#include "platform/input.h"
#include "ui/ui.h"
#include "engine.h"

#include "assets/texture.h"
#include "graphics/gfx.h"


bool is_editor_visible = false;
bool is_cursor_visible = false;

GfxSkybox* skybox;

static
void on_init() {
    editor_geometry_init();
    cursor_set_visible(is_cursor_visible);
    ui_enable_fps(true);
    /* ------------------- */

    skybox = texture_load_skybox(
        "skybox/clear_right.dds",
        "skybox/clear_left.dds",
        "skybox/clear_up.dds",
        "skybox/clear_down.dds",
        "skybox/clear_front.dds",
        "skybox/clear_back.dds"
    );
    gfx_set_skybox(skybox);
}


static
void on_destroy() {
    editor_geometry_destroy();
    gfx_unload_skybox(skybox);
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
