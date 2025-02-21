#pragma once
#include <cglm/cglm.h>

#include "camera.h"
#include "gfx.h"


typedef struct Object {
    const char* id;
    GfxMesh** meshes;
    GfxTexture** textures;
    u64 meshes_count;
    u64 textures_count;
} Object;


Object object_create(const char* id);
void object_destroy(Object*);
void object_load_meshes(Object*, const char* meshes_path);
void object_load_texture(Object*, const char* texture_path);


typedef struct ObjectPtr {
    Object* obj;

    vec3 pos;
    vec3 rot;
    vec3 sc;

    bool is_active;
} ObjectPtr;


constexpr u32 __MAX_SCENE_OBJECTS = 1024;

typedef struct Scene {
    ObjectPtr objects[__MAX_SCENE_OBJECTS];
    GfxObject gfx_objects[__MAX_SCENE_OBJECTS];
    u32 objects_cnt;
    u32 gfx_objects_cnt;
} Scene;


Scene* scene_create();
void scene_destroy(Scene*);
void scene_add_object(Scene*, Object* obj, vec3 pos, vec3 rot, vec3 sc);
void scene_draw(Scene*, Camera*);
