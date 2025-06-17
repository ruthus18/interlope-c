#pragma once
#include "cglm/cglm.h"

#include "./limits.h"
#include "../core/types.h"

#define MAX_ID_LENGTH 64
#define MAX_MESH_PATH_LENGTH 256
#define MAX_TEXTURE_PATH_LENGTH 256
#define MAX_TEXTURES 16


typedef enum {
    OBJECT_NULL,
    OBJECT_STATIC,
} ObjectType;

typedef enum {
    PHSHAPE_NULL,
    PHSHAPE_BOX,
    PHSHAPE_AABB,
} PhysicsShape;

// ------

typedef struct ModelInfo {
    char mesh[MAX_MESH_PATH_LENGTH];
    char (*textures)[MAX_TEXTURE_PATH_LENGTH];
    u8 texture_count;

    u8 _nodes_count;
} ModelInfo;


typedef struct PhysicsInfo {
    PhysicsShape shape;
    vec3 size;
    vec3 pos;
    f32 mass;
} PhysicsInfo;


typedef struct ObjectInfo {
    char id[MAX_ID_LENGTH];
    ObjectType type;

    ModelInfo* model;
    PhysicsInfo** physics;
} ObjectInfo;


typedef struct ObjectRefInfo {
    char id[MAX_ID_LENGTH];
    vec3 pos;
    vec3 rot;
} ObjectRefInfo;


typedef struct SceneInfo {
    char id[MAX_ID_LENGTH];
    ObjectRefInfo** object_refs;

    vec3 player_init_pos;
    vec2 player_init_rot;
} SceneInfo;
