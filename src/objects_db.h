#pragma once

#include "gfx.h"
#include "model.h"


typedef enum ObjectType {
    ObjectType_UNKNOWN,
    ObjectType_PLAYER,
    ObjectType_STATIC,
    ObjectType_RIGID_BODY,
    ObjectType_DOOR,
} ObjectType;


typedef struct ObjectRecord {
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
} ObjectRecord;


ObjectRecord objrec_create(
    const char* id,
    ObjectType type,
    const char* model_path,
    const char** texture_paths
);
void objrec_destroy(ObjectRecord* obj);
void objrec_load_model(ObjectRecord* obj, const char* model_path);
void objrec_load_texture(ObjectRecord* obj, const char* texture_path);


typedef struct ObjectsDB ObjectsDB;

ObjectsDB* objdb_read_toml(const char* toml_path);
void objdb_destroy(ObjectsDB*);
ObjectRecord* objdb_find(ObjectsDB* objdb, const char* base_id);
