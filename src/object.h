#pragma once
#include <cglm/cglm.h>

#include "gfx.h"
#include "model.h"
#include "types.h"


typedef enum ObjectType {
    ObjectType_UNKNOWN,
    ObjectType_RIGID_BODY,
} ObjectType;


/* ObjectDB record */
typedef struct ObjectBase {
    char id[64];
    
    GfxMesh** meshes;
    u64 meshes_count;
    GfxTexture** textures;
    u64 textures_count;
    
    char** names;
    vec3* local_positions;
    vec3* local_rotations;
    
    ObjectType type;
    
    Model* __model;
} ObjectBase;


ObjectBase objbase_create(const char* id);
void objbase_destroy(ObjectBase* obj);
void objbase_load_meshes(ObjectBase* obj, const char* meshes_path);
void objbase_load_texture(ObjectBase* obj, const char* texture_path);


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


const char* object_get_base_id(Object*);
const char* object_get_type_string(Object*);
void object_get_position(Object*, vec3);
void object_get_rotation(Object*, vec3);
void object_set_position(Object*, vec3);
void object_set_rotation(Object*, vec3);
void object_set_rotation_mat(Object* obj, mat4 m_rot);
void object_set_subm_rotation(Object*, vec3, u32 slot_idx);
