#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>
#include <toml.h>

#include "gfx.h"
#include "log.h"
#include "object.h"
#include "objdb.h"
#include "texture.h"
#include "scene.h"
#include "types.h"


const u64 _MAX_SCENE_OBJECTS = 1024;


typedef struct Scene {
    Object** objects;
    u64 objects_count;
    // Maximum number of objects that can be stored without resizing
    u64 objects_capacity;

    Object* selected_object;
} Scene;


Scene* scene_create() {
    Scene* scene = malloc(sizeof(Scene));
    if (scene == NULL) {
        log_exit("Failed to allocate memory for Scene");
    }
    
    scene->objects_capacity = _MAX_SCENE_OBJECTS;
    scene->objects = malloc(sizeof(Object*) * scene->objects_capacity);
    if (scene->objects == NULL) {
        free(scene);
        log_exit("Failed to allocate memory for Scene objects array");
    }
    
    scene->objects_count = 0;
    scene->selected_object = NULL;
    return scene;
}


void scene_destroy(Scene* scene) {
    for (u64 i = 0; i < scene->objects_count; i++) {
        object_destroy(scene->objects[i]);
    }
    
    free(scene->objects);
    free(scene);
}


void scene_add_object(Scene* scene, ObjectRecord* objrec, vec3 pos, vec3 rot, vec3 sc) {
    if (scene->objects_count >= scene->objects_capacity) {
        log_error(
            "Cannot add object '%s': Maximum scene object limit (%llu) reached.",
            objrec->id, scene->objects_capacity
        );
        return;
    }
    
    Object* obj = object_create(objrec, pos, rot, sc);
    
    scene->objects[scene->objects_count] = obj;
    scene->objects_count++;
}


u64 scene_get_objects_count(Scene* scene) {
    return scene->objects_count;
}

Object* scene_get_object(Scene* scene, u64 idx) {
    if (idx >= scene->objects_count) {
        return NULL;
    }
    return scene->objects[idx];
}


Object* scene_find_object(Scene* scene, const char* base_id) {
    for (int i = 0; i < scene->objects_count ; i++) {
        if (strcmp(scene->objects[i]->base_id, base_id) == 0) {
            return scene->objects[i];
        }
    }
    return NULL;
}


void scene_set_selected_object(Scene* scene, Object* obj) {
    scene->selected_object = obj;
}


void scene_update(Scene* scene) {
    for (int i = 0; i < scene->objects_count; i++) {
        Object* obj = scene->objects[i];
        object_update(obj);
    }
}


void scene_draw(Scene* scene) {
    gfx_begin_draw_objects();

    for (int i = 0; i < scene->objects_count; i++) {
        Object* obj = scene->objects[i];

        for (int j = 0; j < obj->slots_count; j++) {
            if (obj != scene->selected_object) {
                gfx_draw_object(obj->meshes[j], obj->textures[j], obj->m_models[j]);
            }
            else {
                gfx_draw_object_outlined(obj->meshes[j], obj->textures[j], obj->m_models[j]);
            }
        }
    }
    gfx_end_draw_objects();
}
