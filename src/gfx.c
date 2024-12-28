/*
    gfx.c - Graphics backend

    Resposible for drawing stuff on screen. Incapsulating OpenGL work.
*/
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "file.h"
#include "gfx.h"
#include "log.h"
#include "platform.h"


/* Shaders Routines */

static
bool check_gl_error() {
    bool is_err = false;
    int gl_err = glGetError();

    while (gl_err != GL_NO_ERROR) {
        log_error("OpenGL Error: %s", gl_err);
        is_err = true;
    }
    return is_err;
}

static
void shader_compile(int program, const char* rel_path, int shader_type) {
    uint32_t shader_id = glCreateShader(shader_type);

    const char* path = path_to_shader(rel_path);
    const char* file_buffer = read_text_file(path);

    glShaderSource(shader_id, 1, &file_buffer, NULL);
    free((void*) path);
    free((void*) file_buffer);

    glCompileShader(shader_id);
    check_gl_error();

    int compile_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_ok);

    if (compile_ok == 1) {
        log_success("Shader compiled: %s", rel_path);
        glAttachShader(program, shader_id);
    }
    else {
        log_error("Shader compilation error; file=%s", rel_path);
        log_glshader(shader_id);
    }
}

static
Shader* shader_create(const char* vert_p, const char* frag_p) {
    Shader* shader = malloc(sizeof(Shader));

    int program = glCreateProgram();
    shader_compile(program, vert_p, GL_VERTEX_SHADER);
    shader_compile(program, frag_p, GL_FRAGMENT_SHADER);

    int link_ok;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (link_ok != 1) {
        log_error("GL program linking error");
        log_glprogram(program);
    }

    shader->program_id = program;
    return shader;
}

static
void shader_destroy(Shader* shader) {
    free(shader);
}


/* ------------------------------------------------------------------------ */
/* GFX - Main Rendering Logic */

static GFX* self;


void gfx_init() {
    self = malloc(sizeof(GFX));

    self->shaders.basic = shader_create(
        "basic.vert", "basic.frag"
    );
}

void gfx_destroy() {
    free(self->shaders.basic);
    free(self);
}

GFX* gfx_get() {
    return self;
}

void gfx_draw() {
    
}
