#pragma once
#include <cglm/cglm.h>


typedef struct Shader {
    int program_id;
} Shader;


Shader* shader_create(const char* vert_p, const char* frag_p);
void shader_destroy(Shader*);
void shader_use(Shader*);

void uniform_set_vec3(Shader*, const char* name, vec3 data);
void uniform_set_mat4(Shader*, const char* name, mat4 data);
