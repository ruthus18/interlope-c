#pragma once
#include <cglm/cglm.h>

#include "./object.h"
#include "./objdb.h"
#include "../core/types.h"


#define SCENE_MAX_OBJECTS 1024

typedef struct Scene {
    Object** objects;
    u64 objects_count;
    // Maximum number of objects that can be stored without resizing
    u64 objects_capacity;

    Object* selected_object;
} Scene;

typedef struct Scene_ {
    Object* objects[SCENE_MAX_OBJECTS];
    u64 objects_count;
} Scene_;

Scene* scene_create();
Scene* scene_create_();
void scene_destroy(Scene*);
void scene_destroy_(Scene*);
void scene_add_object(Scene* scene, ObjectRecord* objrec, vec3 pos, vec3 rot, vec3 sc);
void scene_add_object_(Scene_* scene, Object_, vec3 pos, vec3 rot, vec3 sc);

u64 scene_get_objects_count(Scene*);
Object* scene_get_object(Scene*, u64 idx);
Object* scene_find_object(Scene*, const char* base_id);
void scene_set_selected_object(Scene*, Object*);

void scene_update(Scene* scene);
void scene_draw(Scene*);
