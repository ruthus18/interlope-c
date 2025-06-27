#pragma once
#include <cglm/cglm.h>


typedef struct Shader {
    int program_id;
} Shader;


Shader* shader_new(const char* vert_path, const char* frag_path);
void shader_free(Shader*);
void shader_use(Shader*);

void uniform_set_vec3(Shader*, const char* name, vec3 data);
void uniform_set_mat4(Shader*, const char* name, mat4 data);
void uniform_set_float(Shader*, const char* name, float value);
