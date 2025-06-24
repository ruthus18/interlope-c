#pragma once

#include "database/schemas.h"
#include "render/gfx.h"


typedef struct ModelNode {
    char name[64];
    GfxMesh* mesh;
    GfxTexture* texture;

    vec3 position;
    vec3 rotation;
    
    vec3 aabb_min;
    vec3 aabb_max;
} ModelNode;

typedef struct Model {
    // char id[MAX_ID_LENGTH];
    ModelNode** nodes;

    struct {
        vec3 min;
        vec3 max;
        vec3 size;
        vec3 offset;
    } aabb;
} Model;


Model* model_create_from_info(ModelInfo*);
void model_destroy(Model*);
