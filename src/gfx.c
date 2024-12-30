/*
    gfx.c - Graphics backend

    Resposible for drawing stuff on screen. Incapsulating OpenGL work.
*/
#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

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

    if (compile_ok != 1) {
        log_error("Shader compilation error; file=%s", rel_path);
        log_glshader(shader_id);
    }

    glAttachShader(program, shader_id);
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

    int validation_ok;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_ok);
    if (validation_ok != 1) {
        log_error("GL program validation error");
        log_glprogram(program);
    }

    shader->program_id = program;
    return shader;
}

static
void shader_destroy(Shader* shader) {
    free(shader);
}

static
void shader_use(Shader* shader) {
    int program = 0;
    if (shader != NULL)
        program = shader->program_id;

    glUseProgram(program);
}

static
void uniform_set_mat4(Shader* shader, const char* name, mat4 data) {
    int uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == -1) {
        log_error("Not found shader uniform: %f", name);
        exit(0);
    }
    glUniformMatrix4fv(uniform_id, 1, GL_FALSE, (float*)data);
}


/* ------------------------------------------------------------------------ */
/* GFX - Main Rendering Logic */

static Gfx* self;


void gfx_init() {
    self = malloc(sizeof(Gfx));

    self->shaders.object = shader_create(
        "object.vert", "object.frag"
    );

    glPointSize(6);
    glLineWidth(2);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void gfx_destroy() {
    free(self->shaders.object);
    free(self);
}

Gfx* gfx_get() {
    return self;
}

GfxMesh* gfx_load_mesh(float* vertices, int* indices, bool cw) {
    GfxMesh* gfx_mesh = malloc(sizeof(GfxMesh));

    shader_use(self->shaders.object);

    // -- VAO --
    glGenVertexArrays(1, &(gfx_mesh->vao));
    glBindVertexArray(gfx_mesh->vao);

    // -- VBO --
    glGenBuffers(1, &(gfx_mesh->vbo));
    glBindBuffer(GL_ARRAY_BUFFER, gfx_mesh->vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW
    );

    // -- IBO --
    glGenBuffers(1, &(gfx_mesh->ibo));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gfx_mesh->ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW
    );

    // -- Attributes --
    // Vertex buffer format: (v1, v2, ..., vn1, vn2, ..., vt1, vt2, ...)
    //
    // More about formats: https://stackoverflow.com/a/39684775
    //
    GLintptr v_offset  = sizeof(float) * 0;
    GLintptr vn_offset = sizeof(float) * 3;
    GLintptr vt_offset = sizeof(float) * 5;

    // attr = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)v_offset);
    glEnableVertexAttribArray(0);

    // attr = 1
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void*)vn_offset);
    glEnableVertexAttribArray(1);

    // attr = 2
    glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, (void*)vt_offset);
    glEnableVertexAttribArray(2);

    // -- Cleanup --

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    shader_use(NULL);

    gfx_mesh->cw = cw;
    return gfx_mesh;
}

void gfx_unload_mesh(GfxMesh* gfx_mesh) {
    free(gfx_mesh);
}


#define COLOR_BG (float)29 / 255, (float)32 / 255, (float)33 / 255, 1.0

static int face_orient;

void gfx_draw(Camera* camera, GfxMesh** meshes, mat4* mm_models) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(COLOR_BG);

    // ----------------
    shader_use(self->shaders.object);

    camera_recalc(camera);
    uniform_set_mat4(self->shaders.object, "m_persp", camera->gfx_data.m_persp);
    uniform_set_mat4(self->shaders.object, "m_view", camera->gfx_data.m_view);


    for (int i = 0; i <= (sizeof(meshes) / sizeof(GfxMesh)); i++) {

        uniform_set_mat4(self->shaders.object, "m_model", mm_models[i]);

        glBindVertexArray(meshes[i]->vao);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[i]->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i]->ibo);

        if (meshes[i]->cw)  face_orient = GL_CW;
        else                face_orient = GL_CCW;

        glFrontFace(face_orient);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    }

    // ----------------
    shader_use(NULL); // Cleanup
}
