#pragma once
#include <cglm/cglm.h>

#include "types.h"


/* ------ Objects DB ------ */

typedef struct ObjectsDB ObjectsDB;

ObjectsDB* objdb_create_from(const char* toml_path);
void objdb_destroy(ObjectsDB*);


/* ------ Object ------ */

typedef struct Object Object;

const char* object_get_base_id(Object*);
void object_get_position(Object*, vec3);
void object_get_rotation(Object*, vec3);
void object_set_position(Object*, vec3);
void object_set_rotation(Object*, vec3);


/* ------ Scene ------ */

typedef struct Scene Scene;

Scene* scene_create();
Scene* scene_create_from(const char* toml_path, ObjectsDB*);
void scene_destroy(Scene*);
void scene_draw(Scene*);

u64 scene_get_objects_count(Scene*);
Object* scene_get_object(Scene*, u64 idx);
