#pragma once


typedef void (*EngineCallback)(void);

typedef enum EngineCallbackType {
    ENGINECB_ON_INIT,
    ENGINECB_ON_DESTROY,
    ENGINECB_ON_UPDATE,
    ENGINECB_ON_DRAW,
} EngineCallbackType;


void engine_set_callback(EngineCallback func, EngineCallbackType type);

void engine_run();
void engine_exit();
