#pragma once
#include "../core/types.h"
#include "../render/gfx.h"


typedef struct Model {
    GfxMesh** meshes;
    vec3* local_positions;
    vec3* local_rotations;
    char** names;
    u64 slots_count;
} Model;


Model* model_read(const char* model_relpath);
void model_destroy(Model*);
