#pragma once
#include <cglm/cglm.h>

#include "object.h"
#include "objects_db.h"
#include "types.h"


typedef struct Scene Scene;

Scene* scene_read_toml(const char* toml_path, ObjectsDB*);
void scene_destroy(Scene*);

u64 scene_get_objects_count(Scene*);
Object* scene_get_object(Scene*, u64 idx);
Object* scene_find_object(Scene*, const char* base_id);
void scene_set_selected_object(Scene*, Object*);

void scene_draw(Scene*);
