#pragma once
#include <cglm/cglm.h>

#include "camera.h"
#include "gfx.h"


/* ------ Object ------ */

typedef struct Object {
    const char* id;
    GfxMesh** meshes;
    u64 meshes_count;
    GfxTexture** textures;
    u64 textures_count;
} Object;


Object object_create(const char* id);
void object_destroy(Object*);
void object_load_meshes(Object*, const char* meshes_path);
void object_load_texture(Object*, const char* texture_path);


/* ------ Objects DB ------ */

constexpr u64 _MAX_OBJECTS_DB_SIZE = 1024;

typedef struct ObjectsDB {
    Object objects[_MAX_OBJECTS_DB_SIZE];
    u64 objects_count;
} ObjectsDB;

ObjectsDB objdb_create_from(const char* toml_path);
void objdb_destroy(ObjectsDB*);


typedef struct ObjectInst {
    Object* obj;
    vec3 pos;
    vec3 rot;
    vec3 sc;

    bool is_active;
} ObjectInst;


/* ------ Scene ------ */

constexpr u64 _MAX_SCENE_OBJECTS = 1024;
constexpr u64 _MAX_OBJECT_TEXTURES = 8;

typedef struct Scene {
    ObjectInst objects[_MAX_SCENE_OBJECTS];
    u64 objects_count;

    GfxScene gfxd;
} Scene;


Scene* scene_create();
Scene* scene_create_from(const char* toml_path, ObjectsDB*);
void scene_destroy(Scene*);

void scene_add_object(Scene*, Object*, vec3 pos, vec3 rot, vec3 sc);
void scene_set_camera(Scene*, Camera*);
