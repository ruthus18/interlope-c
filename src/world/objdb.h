#pragma once

#include "../render/gfx.h"
#include "../assets/model.h"


typedef enum ObjectType {
    ObjectType_UNKNOWN,
    ObjectType_PLAYER,
    ObjectType_STATIC,
    ObjectType_RIGID_BODY,
    ObjectType_DOOR,
} ObjectType;


typedef struct PhysicsProperties {
    bool has_physics;           // Whether this object has physics enabled
    float mass;                 // Mass in kg (0 for static objects)
    vec3 collision_size;        // Size of the collision box/sphere
    char collision_type[32];    // "box", "sphere", etc.
} PhysicsProperties;

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
    PhysicsProperties physics;  // Physics properties
    
    Model* __model;
} ObjectRecord;

typedef struct ObjectsDB {
    ObjectRecord objects[1024]; // Using a define in the .c file
    u64 objects_count;
} ObjectsDB;


ObjectRecord objrec_create(
    const char* id,
    ObjectType type,
    const char* model_path,
    const char** texture_paths,
    const PhysicsProperties* physics
);
void objrec_destroy(ObjectRecord* obj);
void objrec_load_model(ObjectRecord* obj, const char* model_path);
void objrec_load_texture(ObjectRecord* obj, const char* texture_path);

ObjectsDB* objdb_create(void);
void objdb_destroy(ObjectsDB*);
ObjectRecord* objdb_find(ObjectsDB* objdb, const char* base_id);
