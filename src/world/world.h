#pragma once
#include "./scene.h"


void world_init();
void world_destroy();

Scene* world_get_current_scene();
void world_update();
void world_draw();
