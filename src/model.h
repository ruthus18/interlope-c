#pragma once
#include "gfx.h"
#include "types.h"


typedef struct Model {
    GfxMesh** meshes;
    vec3* local_positions;
    char** names;
    u64 slots_count;
} Model;


Model* model_read(const char* model_relpath);
void model_destroy(Model*);
