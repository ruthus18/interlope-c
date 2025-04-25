#pragma once
#include <cglm/cglm.h>

#include "gfx.h"
#include "objdb.h"
#include "types.h"


/* Scene object instance */
typedef struct Object {
    const char* base_id;
    // TODO: need consistent naming (pos -> position, rot -> ..., sc -> ...)
    vec3 pos;
    vec3 rot;
    vec3 sc;
    bool is_active;

    GfxMesh** meshes;
    GfxTexture** textures;
    u16 slots_count;

    vec3* local_positions;
    vec3* local_rotations;
    mat4* m_models;

    ObjectType type;
} Object;


// Object lifetime management
Object* object_create(ObjectRecord* objrec, vec3 pos, vec3 rot, vec3 sc);
void object_destroy(Object* obj);

// Object getters/setters
const char* object_get_base_id(Object*);
const char* object_get_type_string(Object*);
void object_get_position(Object*, vec3);
void object_get_rotation(Object*, vec3);
void object_set_position(Object*, vec3);
void object_set_rotation(Object*, vec3);
void object_set_subm_rotation(Object*, vec3, u32 slot_idx);
