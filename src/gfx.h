#pragma once
#include "camera.h"
#include "platform.h"


typedef struct Shader {
    int program_id;
} Shader;


typedef struct GfxMesh {
    int vao;
    int vbo;
    int ibo;
    bool cw;
} GfxMesh;


typedef struct Gfx {
    struct {
        Shader* object;
    } shaders;
} Gfx;


void gfx_init();
void gfx_destroy();
Gfx* gfx_get();

GfxMesh* gfx_load_mesh(float* vertices, int* indices, bool cw);
void gfx_unload_mesh(GfxMesh*);
void gfx_draw(Camera* camera, GfxMesh** meshes, mat4* mm_models);
