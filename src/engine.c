#include <assert.h>

#include "engine.h"

#include "core/config.h"
#include "core/log.h"
#include "database/db.h"
#include "platform/input.h"
#include "platform/time.h"
#include "platform/window.h"
#include "render/gfx.h"
#include "world/world.h"
#include "physics.h"


static EngineCallback __on_init__ = NULL;
static EngineCallback __on_destroy__;
static EngineCallback __on_update__;
static EngineCallback __on_draw__;


void engine_set_callback(EngineCallback func, EngineCallbackType type) {
    switch (type) {
        case ENGINE_ON_INIT:      __on_init__ = func;     break;
        case ENGINE_ON_DESTROY:   __on_destroy__ = func;  break;
        case ENGINE_ON_UPDATE:    __on_update__ = func;   break;
        case ENGINE_ON_DRAW:      __on_draw__ = func;     break;
        default:                  log_exit("NotImplemented");
    }
}

static
void _validate_callbacks() {
    if (!__on_init__)       log_exit("`__on_init__` callback not set");
    if (!__on_destroy__)    log_exit("`__on_destroy__` callback not set");
    if (!__on_update__)     log_exit("`__on_update__` callback not set");
    if (!__on_draw__)       log_exit("`__on_draw__` callback not set");
}


void engine_run() {
    _validate_callbacks();

    window_init();
    input_init();
    gfx_init();
    physics_init();

    db_init();
    world_init();

    __on_init__();

    while (!gfx_need_stop()) {
        window_poll_events();
        time_update();
        input_update();
        physics_update();
        world_update();
        __on_update__();

        world_draw();   
        __on_draw__();

        window_swap_buffers();
        if (!WINDOW_VSYNC)  time_limit_framerate();
    }
    __on_destroy__();

    world_destroy();
    db_destroy();

    physics_destroy();
    gfx_destroy();
    window_destroy();
}


void engine_exit() {
    gfx_stop();
}
