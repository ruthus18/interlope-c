#pragma once
#include <cglm/cglm.h>

#include "camera.h"
#include "gfx.h"


typedef struct Object {
    const char* id;
    GfxMesh* mesh;
    GfxTexture* texture;
} Object;


typedef struct ObjectPtr {
    Object* obj;

    vec3 pos;
    vec3 rot;
    vec3 sc;
    mat4 m_model;

    bool is_active;
} ObjectPtr;


constexpr unsigned int __MAX_SCENE_OBJECTS = 128;

typedef struct Scene {
    ObjectPtr objects[__MAX_SCENE_OBJECTS];
    unsigned int objects_cnt;
} Scene;


Scene* scene_create();
void scene_destroy(Scene*);
void scene_add_object(Scene* scene, Object* obj, vec3 pos, vec3 rot, vec3 sc);
void scene_draw(Camera*, Scene*);
