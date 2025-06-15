#pragma once
#include "./object.h"
#include "./scene.h"


void world_init();
void world_destroy();

void world_print();
Object** world_get_objects();
Object* world_find_object(char* id);
Scene* world_get_current_scene();

void world_update();
void world_draw();
