#include <stdlib.h>

#include "core/log.h"
#include "engine.h"
#include "editor/geometry.h"
#include "gameplay/player.h"
#include "platform/input.h"
#include "ui/ui.h"
#include "core/types/map.h"

#define CVECTOR_LINEAR_GROWTH
#include <cvector.h>
#include <cvector_utils.h>


bool is_editor_visible = false;
bool is_cursor_visible = false;


typedef struct Test {
    int num;
} Test;

static
void on_init() {
    ui_enable_fps(true);
    // ui_enable_interaction(true);

    editor_geometry_init();
    cursor_set_visible(is_cursor_visible);


    // cvector(Test) nums = NULL;
    // cvector_push_back(nums, (Test){100});
    // cvector_push_back(nums, (Test){200});
    // cvector_push_back(nums, (Test){300});
    
    // Test* last = cvector_back(nums);
    // cvector_erase(nums, 0);
    // cvector_push_back(nums, (Test){400});

    // log_exit("%i", last->num);
}


static
void on_destroy() {
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
