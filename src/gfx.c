#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "config.h"
#include "gfx.h"
#include "gfx_shader.h"
#include "log.h"
#include "platform/time.h"
#include "platform/window.h"
#include "types.h"


#define vec3_size (3 * sizeof(f32))
#define vec2_size (2 * sizeof(f32))


static struct _Gfx {
    Window* window;
    bool stop_;

    struct {
        Shader* object;
    } shaders;

} self;


static inline
Window* _create_window() {
    if (!glfwInit()) {
        printf("GLFW Initialization Error!\n");
        exit(EXIT_FAILURE);
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

    window_set(window);
    return window;
}


static inline
void _log_startup_info() {
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
void _init_shaders() {
    self.shaders.object = shader_create(
        "object.vert", "object.frag"
    );
}


void gfx_init() {
    self.window = _create_window();
    self.stop_ = false;

    _log_startup_info();
    _init_shaders();

    glPointSize(6);
    glLineWidth(2);

    glPolygonMode(GL_FRONT_AND_BACK, GFX_WIREFRAME_MODE ? GL_LINE : GL_POLYGON);
}


void gfx_destroy() {
    free(self.shaders.object);
    glfwDestroyWindow(self.window);
    glfwTerminate();
}


GfxMesh* gfx_mesh_load(
    const char* name,
    f32* vtx_buf,
    u32* ind_buf,
    u64 vtx_count,
    u64 ind_count,
    bool cw
) {
    GfxMesh* mesh = malloc(sizeof(GfxMesh));
    mesh->name = name;
    mesh->vtx_count = vtx_count;
    mesh->ind_count = ind_count;
    mesh->cw = cw;

    shader_use(self.shaders.object);

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * ind_count, ind_buf, GL_STATIC_DRAW);

    /* -- Vertex Attributes -- */
    // Vertex buffer format: (v1, v2, ..., vn1, vn2, ..., vt1, vt2, ...)
    //
    // More about formats: https://stackoverflow.com/a/39684775
    GLintptr v_offset = 0;
    GLintptr vn_offset = (vec3_size * vtx_count);
    GLintptr vt_offset = vec3_size * vtx_count * 2;

    // vtx position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, vec3_size, (void*)v_offset);
    glEnableVertexAttribArray(0);

    // vtx normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, vec3_size, (void*)vn_offset);
    glEnableVertexAttribArray(1);

    // vtx texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, false, vec2_size, (void*)vt_offset);
    glEnableVertexAttribArray(2);

    /* -- Cleanup -- */

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    shader_use(NULL);

    log_success("Mesh loaded: %s", name);
    return mesh;
}


void gfx_mesh_unload(GfxMesh* mesh){
    free(mesh);
}


GfxTexture* gfx_texture_load(
    u8* data,
    u32 width,
    u32 height,
    int gl_format
) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));

    shader_use(self.shaders.object);
    glGenTextures(1, &(texture->id));
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    shader_use(NULL);
    return texture;
}


GfxTexture* gfx_texture_load_dds(
    u8* data,
    u32 width,
    u32 height,
    i32 gl_format,
    u32 mipmap_cnt,
    u32 block_size
) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));

    shader_use(self.shaders.object);
    glGenTextures(1, &(texture->id));
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_cnt-1);

    unsigned int offset = 0;
    unsigned int size = 0;
    unsigned int w = width;
    unsigned int h = height;

    for (unsigned int i=0; i < mipmap_cnt; i++) {
        if(w == 0 || h == 0) {
            // discard any odd mipmaps 0x1 0x2 resolutions
            mipmap_cnt--;
            continue;
        }
        size = ((w+3)/4) * ((h+3)/4) * block_size;
        glCompressedTexImage2D(GL_TEXTURE_2D, i, gl_format, w, h, 0, size, data + offset);
        offset += size;
        w /= 2;
        h /= 2;
    }

    shader_use(NULL);
    return texture;
}


void gfx_texture_unload(GfxTexture* texture) {
    free(texture);
}


void gfx_begin_draw() { 
    glfwPollEvents();
}


static bool __persp_mat_set = false;


void gfx_draw(GfxCamera* camera, GfxMesh* mesh, GfxTexture* texture, mat4 m_model) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(WINDOW_BG_COLOR);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    /* ---------------- */
    /* OBJECT SHADER */
    shader_use(self.shaders.object);

    // glBindTexture(GL_TEXTURE_2D, texture->id);

    if (!__persp_mat_set) {
        uniform_set_mat4(self.shaders.object, "m_persp", camera->m_persp);
        __persp_mat_set = true;
    }
    uniform_set_mat4(self.shaders.object, "m_view", camera->m_view);
    uniform_set_mat4(self.shaders.object, "m_model", m_model);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    glFrontFace(mesh->cw ? GL_CW : GL_CCW);
    glDrawElements(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, NULL);

    /* ---------------- */
    /* CLEANUP */
    // glBindTexture(GL_TEXTURE_2D, 0);
    shader_use(NULL);
}


void gfx_end_draw() {
    glfwSwapBuffers(self.window);
    if (!WINDOW_VSYNC)  time_limit_framerate();
}

bool gfx_need_stop() {
    return glfwWindowShouldClose(self.window) || self.stop_;
}

void gfx_stop() {
    self.stop_ = true;
    log_info("Engine stopped. Bye!");
}
