#pragma once


typedef void (*EngineCallback)(void);

typedef enum EngineCallbackType {
    ENGINE_ON_INIT,
    ENGINE_ON_DESTROY,
    ENGINE_ON_UPDATE,
    ENGINE_ON_DRAW,
} EngineCallbackType;


void engine_set_callback(EngineCallback func, EngineCallbackType type);

void engine_run();
void engine_exit();
