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

Scene* scene_create(void);
void scene_destroy(Scene*);
void scene_add_object(Scene* scene, ObjectRecord* objrec, vec3 pos, vec3 rot, vec3 sc);

u64 scene_get_objects_count(Scene*);
Object* scene_get_object(Scene*, u64 idx);
Object* scene_find_object(Scene*, const char* base_id);
void scene_set_selected_object(Scene*, Object*);

void scene_update(Scene* scene);
void scene_draw(Scene*);
