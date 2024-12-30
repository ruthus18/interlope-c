/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include "camera.h"
#include "platform.h"


typedef struct GfxMesh {
    int vao;
    int vbo;
    int ibo;

    size_t vtx_count;
    size_t ind_count;

    bool cw;  // vertex ordering (1 = clockwise ; 0 = counterwise)
} GfxMesh;

// typedef struct GfxScene {
//     struct {
//         GfxMesh* gfx_data;
//         char* name;
//     } meshes[16];

//     struct {
//         int mesh_idx;    // mesh index inside `meshes`
//         mat4 m_model;    // result model matrix
//     } pholders[128];

//     size_t size;
// } GfxScene;

typedef struct Shader {
    int program_id;
} Shader;

typedef struct Gfx {
    Window* window;
    bool stop_;

    struct {
        Shader* object;
    } shaders;
} Gfx;

void window_init();
Window* window_get();

void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();
void gfx_draw(Camera* camera, GfxMesh* mesh, mat4 model_mat);
// void gfx_draw(Camera* camera, GfxScene* scene);

GfxMesh* gfx_mesh_load(
    float* vtx_buf, int* ind_buf, size_t vtx_count, bool cw, char* name
);
void gfx_mesh_unload(GfxMesh*);
