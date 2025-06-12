#include <stdlib.h>

#include "core/log.h"
#include "database/db.h"
#include "editor/sys_geometry.h"
#include "editor/ui.h"
#include "gameplay/player.h"
#include "platform/input.h"
#include "engine.h"


static void __game_on_init__();
static void __game_on_destroy__();
static void __game_on_update__();
static void __game_on_draw__();


int main() {
    engine_set_callback(__game_on_init__, ENGINECB_ON_INIT);
    engine_set_callback(__game_on_destroy__, ENGINECB_ON_DESTROY);
    engine_set_callback(__game_on_update__, ENGINECB_ON_UPDATE);
    engine_set_callback(__game_on_draw__, ENGINECB_ON_DRAW);

    engine_run();
    return EXIT_SUCCESS;
}

/* ------------------------------------------------------------------------- */

bool is_editor_visible = false;
bool is_cursor_visible = false;


static
void __game_on_init__() {
    editor_init();
    editor_geometry_init();

    cursor_set_visible(is_cursor_visible);

    // ---
    Database* db = db_get();
    log_debug("Objects DB size: %u", db->objects_count);
    log_debug("Scene size: %u", db->scene->object_refs_count);

    ObjectInfo* floor_info = db_get_object_info("GridFloor");

}


static
void __game_on_destroy__() {    
    editor_geometry_destroy();
    editor_destroy();
}


static
void __game_on_update__() {
    if (input_is_keyp(IN_KEY_ESC))
        engine_exit();

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

    editor_update(is_editor_visible);
}


static
void __game_on_draw__() {
    editor_geometry_draw();
}

