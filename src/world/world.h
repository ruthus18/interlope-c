#pragma once
#include "world/object.h"
#include "world/scene.h"

#include "physics/px_object.h"


void world_init();
void world_destroy();

void world_print();
Object* world_get_object(char* id);
Scene* world_get_current_scene();

ObjectRef* world_get_oref_by_id(u32 ref_id);
ObjectRef* world_get_oref_by_physics(PxObject* value);
void world_remove_oref(ObjectRef* oref);

void world_update();
void world_draw();
