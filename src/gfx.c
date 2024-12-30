#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "config.h"
#include "cgm.h"
#include "gfx.h"
#include "log.h"
#include "platform.h"


/* ------------------------------------------------------------------------ */
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
    const char* file_buffer = file_read_text(path);

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

    log_success("Shader created: %s | %s", vert_p, frag_p);
    shader->program_id = program;
    return shader;
}

static
void shader_destroy(Shader* shader) {
    free(shader);
}

static
void shader_use(Shader* shader) {
    glUseProgram((shader != NULL) ? shader->program_id : 0);
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


void window_init() {
    if (!glfwInit()) {
        printf("GLFW Initialization Error!\n");
        exit(0);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (!WINDOW_BORDER) {
        glfwWindowHint(GLFW_DECORATED, false);
    }

    GLFWmonitor* monitor = NULL;
    if (WINDOW_FULLSC)
        monitor = glfwGetPrimaryMonitor();

    Window* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        WINDOW_TITLE,
        monitor,
        NULL
    );
    glfwSetWindowPos(window, WINDOW_XPOS, WINDOW_YPOS);
    glfwMakeContextCurrent(window);

    int glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        printf("GLEW Initialization Error!\n");
        exit(0);
    }

    glfwSwapInterval(WINDOW_VSYNC);
    self->window = window;
}

Window* window_get() { return self->window; }


static inline
void log_startup_info() {
    log_greeting("======  Interlope Engine  ======");
    log_info("ENGINE VERSION: %s", ENGINE_VERSION);
    log_info("OPENGL VERSION: %s", glGetString(GL_VERSION));
    log_info("GLEW VERSION: %s", glewGetString(GLEW_VERSION));
    log_info("GLFW VERSION: %u.%u.%u", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR);
    log_info("VIDEO DEVICE: %s (%s)", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    log_info("WIN SIZE: %i x %i", WINDOW_WIDTH, WINDOW_HEIGHT);
    log_info("WIN VSYNC: %i", WINDOW_VSYNC);
    log_info("------");
}


static inline
void init_shaders() {
    self->shaders.object = shader_create(
        "object.vert", "object.frag"
    );
}


void gfx_init() {
    self = malloc(sizeof(Gfx));
    self->stop_ = false;

    window_init();
    log_startup_info();
    init_shaders();

    glPointSize(6);
    glLineWidth(2);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

}


void gfx_destroy() {
    free(self->shaders.object);
    glfwDestroyWindow(self->window);

    glfwTerminate();
    free(self);
}


bool gfx_need_stop() {
    glfwWindowShouldClose(self->window) || self->stop_;
}

void gfx_stop() {
    self->stop_ = true;
    log_info("Enfine stopped");
}


GfxMesh* gfx_mesh_load(
    float* vtx_buf, int* ind_buf, size_t vtx_count, bool cw, char* name
) {
    GfxMesh* mesh = malloc(sizeof(GfxMesh));
    mesh->vtx_count = vtx_count;
    mesh->ind_count = len(ind_buf);
    mesh->cw = cw;

    shader_use(self->shaders.object);

    /* -- VAO -- */
    glGenVertexArrays(1, &(mesh->vao));
    glBindVertexArray(mesh->vao);

    /* -- VBO -- */
    glGenBuffers(1, &(mesh->vbo));
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vtx_buf), vtx_buf, GL_STATIC_DRAW
    );

    /* -- IBO -- */
    glGenBuffers(1, &(mesh->ibo));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(ind_buf), ind_buf, GL_STATIC_DRAW
    );

    /* -- Vertex Attributes -- */
    // Vertex buffer format: (v1, v2, ..., vn1, vn2, ..., vt1, vt2, ...)
    //
    // More about formats: https://stackoverflow.com/a/39684775

    GLintptr v_offset  = 0;
    GLintptr vn_offset = sizeof_vec3 * vtx_count;
    GLintptr vt_offset = sizeof_vec3 * vtx_count * 2;

    // attr = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)v_offset);
    glEnableVertexAttribArray(0);

    // attr = 1
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void*)vn_offset);
    glEnableVertexAttribArray(1);

    // attr = 2
    glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, (void*)vt_offset);
    glEnableVertexAttribArray(2);

    /* -- Cleanup -- */

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    shader_use(NULL);

    log_success("Mesh loaded: %s", name);
    return mesh;
}


void gfx_mesh_unload(GfxMesh* gfx_mesh) {
    free(gfx_mesh);
}


void gfx_draw(Camera* camera, GfxMesh* mesh, mat4 model_mat) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(WINDOW_BG_COLOR);

    /* ---------------- */
    /* OBJECT SHADER */
    shader_use(self->shaders.object);

    camera_update_gfx_data(camera);

    // log_info("MODEL:");
    // glm_mat4_print(model_mat, stdout);
    // log_info("VIEW:");
    // glm_mat4_print(camera->gfx_data.m_view, stdout);

    uniform_set_mat4(self->shaders.object, "m_persp", camera->gfx_data.m_persp);
    uniform_set_mat4(self->shaders.object, "m_view", camera->gfx_data.m_view);

    // Current model state
    // GfxMesh* mesh;
    // mat4 m_model;

    // for (int i = 0; i <= len(scene->pholders); i++) {
        /* Set current model state */
        // mesh = scene->meshes[scene->pholders[i].mesh_idx].gfx_data;
        // glm_mat4_copy(scene->pholders[i].m_model, m_model);

        /* Draw current model */
        
        uniform_set_mat4(self->shaders.object, "m_model", model_mat);

        glBindVertexArray(mesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

        glFrontFace(mesh->cw ? GL_CW : GL_CCW);
        glDrawElements(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, NULL);
    // }

    /* ---------------- */
    /* CLEANUP */
    shader_use(NULL);

    glfwSwapBuffers(self->window);
}
