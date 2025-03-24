#pragma once
#include <cglm/cglm.h>


typedef struct Shader {
    int program_id;
} Shader;


Shader* gfx_shader_create(const char* vert_path, const char* frag_path);
void gfx_shader_destroy(Shader*);
void gfx_shader_use(Shader*);

void gfx_uniform_set_vec3(Shader*, const char* name, vec3 data);
void gfx_uniform_set_mat4(Shader*, const char* name, mat4 data);
