#pragma once

#include "database/schemas.h"
#include "render/gfx.h"


typedef struct ModelNode {
    char name[64];
    GfxMesh* mesh;
    GfxTexture* texture;

    vec3 position;
    vec3 rotation;
} ModelNode;

typedef struct Model {
    char id[MAX_ID_LENGTH];
    ModelNode** nodes;
} Model;


Model* model_create_from_info(ModelInfo*);
void model_destroy(Model*);
