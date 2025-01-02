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


#define __NOT_FOUND -1


static
void uniform_set_mat4(Shader* shader, const char* name, mat4 data) {
    int uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == __NOT_FOUND) {
        log_error("Not found shader uniform: %s", name);
        gfx_stop();
    }
    glUniformMatrix4fv(uniform_id, 1, false, (float*)data);
}


static
void uniform_set_vec3(Shader* shader, const char* name, vec3 data) {
    int uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == __NOT_FOUND) {
        log_error("Not found shader uniform: %s", name);
        gfx_stop();
    }
    glUniform3f(uniform_id, data[0], data[1], data[2]);
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
    self->shaders.objects = shader_create(
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

    glPolygonMode(GL_FRONT_AND_BACK, GFX_WIREFRAME_MODE ? GL_POLYGON : GL_LINE);
}


#define vec3_size (3 * sizeof(float))
#define vec2_size (2 * sizeof(float))


GfxMesh* gfx_mesh_load(char* id, float* vtx_buf, int* ind_buf, int vtx_count, int ind_count, bool cw) {
    GfxMesh* mesh = malloc(sizeof(GfxMesh));
    mesh->id = id;
    mesh->vtx_count = vtx_count;
    mesh->ind_count = ind_count;
    mesh->cw = cw;

    shader_use(self->shaders.objects);

    /* -- VAO -- */
    glGenVertexArrays(1, &(mesh->vao));
    glBindVertexArray(mesh->vao);

    /* -- VBO -- */
    glGenBuffers(1, &(mesh->vbo));
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * vtx_count, vtx_buf, GL_STATIC_DRAW);

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
    GLintptr v_offset = 0;
    GLintptr vn_offset = vec3_size * vtx_count;
    GLintptr vt_offset = vec3_size * vtx_count * 2;

    // vtx position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, vec3_size, (void*)v_offset);
    glEnableVertexAttribArray(0);

    // vtx normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, vec3_size, (void*)vn_offset);
    glEnableVertexAttribArray(1);

    // vtx texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, false, vec2_size, (void*)vn_offset);
    glEnableVertexAttribArray(2);

    /* -- Cleanup -- */

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    shader_use(NULL);

    log_success("Mesh loaded: %s", id);
    return mesh;
}


void gfx_mesh_unload(GfxMesh* mesh){
    free(mesh);
}


void gfx_destroy() {
    free(self->shaders.objects);
    glfwDestroyWindow(self->window);

    glfwTerminate();
    free(self);
}


#define __GFX_FAILBACK_SHADER_ENABLED true


void gfx_draw(GfxCamera* camera, GfxMesh* mesh, mat4 m_model) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(WINDOW_BG_COLOR);

    /* ---------------- */
    /* FAILBACK SHADER */
    #if __GFX_FAILBACK_SHADER_ENABLED

    shader_use(self->shaders.objects);
    glBindVertexArray(mesh->vao);

    uniform_set_mat4(self->shaders.objects, "m_persp", camera->m_persp);
    uniform_set_mat4(self->shaders.objects, "m_view", camera->m_view);
    uniform_set_mat4(self->shaders.objects, "m_model", m_model);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    #endif
    /* ---------------- */
    /* CLEANUP */
    shader_use(NULL);
    glfwSwapBuffers(self->window);
}


bool gfx_need_stop() {
    glfwWindowShouldClose(self->window) || self->stop_;
}

void gfx_stop() {
    self->stop_ = true;
    log_info("Engine stopped");
}
