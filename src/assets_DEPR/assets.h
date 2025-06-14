#pragma once

#include "./model.h"
#include "./texture.h"
#include "../render/gfx.h"
#include "../core/config.h"
#include "../database/schemas.h"


// TODO: validation of "limit reached"
#define ASSETS_MAX_MESHES 1024
#define ASSETS_MAX_TEXTURES 1024
#define ASSETS_MAX_MODELS 1024
#define ASSETS_MAX_PATH_LEN 128


typedef struct ModelNode_ {
    char name[64];
    GfxMesh* mesh;
    GfxTexture* texture;
} ModelNode_;


typedef struct Model_ {
    char id[MEM_MAX_ID_LENGTH];

    ModelNode_** nodes;
    u8 nodes_count;
} Model_;


typedef struct AssetStorage {
    Model_ models[ASSETS_MAX_MODELS];
    u16 models_count;

    // TBD: next, it can be extended with reuse of meshes, textures, fonts...
} AssetStorage;


Model_* assets_model_init();
void assets_model_destroy(Model_*);


Model_* assets_load_model(ModelInfo* info);
void assets_unload_model(Model_* model);
