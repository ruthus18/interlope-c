#include <stdlib.h>

#include "engine.h"
#include "core/log.h"
#include "core/utils.h"
#include "database/db.h"
#include "editor/geometry.h"
#include "editor/ui.h"
#include "gameplay/player.h"
#include "platform/input.h"
#include "world/world.h"


bool is_editor_visible = false;
bool is_cursor_visible = false;


static
void on_init() {
    // editor_init();
    editor_geometry_init();

    cursor_set_visible(is_cursor_visible);
    
    /* --- Working with DB --- */
    // Database* db = db_get();
    
    // log_debug("Objects DB size: %i", count_(db->scene->object_refs));
    // log_debug("Scene size: %i", count_(db->objects));
    
    /* --- Working with world --- */
    Object** objects = world_get_objects();
    Scene* scene = world_get_current_scene();
    
    log_debug("Total objects: %i", count_(objects));
    log_debug("Total object refs: %i", count_(scene->object_refs));

    // ---
    ObjectInfo* floor_info = db_find_object("GridFloor");

    player_set_gravity_enabled(false);
}


static
void on_destroy() {    
    editor_geometry_destroy();
    // editor_destroy();
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
        player_set_is_active(!is_cursor_visible);
    }

    // Switch cursor/camera mouse control
    else if (input_is_keyp(IN_KEY_F2)) {
        is_cursor_visible = !is_cursor_visible;

        cursor_set_visible(is_cursor_visible);
        player_set_is_active(!is_cursor_visible);
    }

    // editor_update(is_editor_visible);
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
