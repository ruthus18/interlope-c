#pragma once
#include <cglm/cglm.h>

#include "types.h"


/* ------ Objects DB ------ */

typedef struct ObjectsDB ObjectsDB;

ObjectsDB* objdb_create_from(const char* toml_path);  // TODO: objdb_load
void objdb_destroy(ObjectsDB*);


/* ------ Object ------ */

typedef struct Object Object;

const char* object_get_base_id(Object*);
void object_get_position(Object*, vec3);
void object_get_rotation(Object*, vec3);
void object_set_position(Object*, vec3);
void object_set_rotation(Object*, vec3);
void object_set_subm_rotation(Object*, vec3, u32 slot_idx);


/* ------ Scene ------ */

typedef struct Scene Scene;

// Scene* scene_create();
Scene* scene_create_from(const char* toml_path, ObjectsDB*);  // TODO: scene_load
void scene_destroy(Scene*);

u64 scene_get_objects_count(Scene*);
Object* scene_get_object(Scene*, u64 idx);
Object* scene_find_object(Scene*, const char* base_id);
void scene_set_selected_object(Scene*, Object*);

void scene_draw(Scene*);
