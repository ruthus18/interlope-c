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
#include "types.h"
#include "platform/window.h"


static struct _Gfx {
    Window* window;
    bool stop_;

    struct {
        Shader* object;
        Shader* object_outline;
        Shader* geometry;
    } shaders;
} self;


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
    self.shaders.object = gfx_shader_create(
        "object.vert", "object.frag"
    );
    self.shaders.object_outline = gfx_shader_create(
        "object.vert", "object_outline.frag"
    );
    self.shaders.geometry = gfx_shader_create(
        "geometry.vert", "geometry.frag"
    );
}


static inline
void _destroy_shaders() {
    free(self.shaders.object);
    free(self.shaders.object_outline);
    free(self.shaders.geometry);
}


void gfx_init() {
    self.window = window_get();
    self.stop_ = false;

    _log_startup_info();
    _init_shaders();

    glPointSize(6);
    glLineWidth(2);

    glPolygonMode(GL_FRONT_AND_BACK, GFX_WIREFRAME_MODE ? GL_LINE : GL_POLYGON);
}


void gfx_destroy() {
    _destroy_shaders();
    glfwDestroyWindow(self.window);
    glfwTerminate();
}


bool gfx_need_stop() {
    return glfwWindowShouldClose(self.window) || self.stop_;
}

void gfx_stop() {
    self.stop_ = true;
    log_info("Engine stopped. Bye!");
}


/* ------------------------------------------------------------------------- */
/* Rendering Resources */
/* ------------------------------------------------------------------------- */

typedef struct GfxMesh {
    const char* name;
    u32 vao;
    u32 vbo;
    u32 ibo;

    u64 vtx_count;
    u64 ind_count;
    
    bool cw;  // vertex ordering (1 = clockwise ; 0 = counterwise)
} GfxMesh;


typedef struct GfxTexture {
    unsigned int id;
} GfxTexture;


typedef struct GfxGeometry {
    u32 vao;
    u32 vbo;
    u64 vtx_count;
} GfxGeometry;


#define VEC3_SIZE (3 * sizeof(f32))
#define VEC2_SIZE (2 * sizeof(f32))


GfxMesh* gfx_mesh_load(const char* name, f32* vtx_buf, u32* ind_buf, u64 vtx_count, u64 ind_count, bool cw) {
    GfxMesh* mesh = malloc(sizeof(GfxMesh));
    mesh->name = name;
    mesh->vtx_count = vtx_count;
    mesh->ind_count = ind_count;
    mesh->cw = cw;

    gfx_shader_use(self.shaders.object);

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
    GLintptr vn_offset = (VEC3_SIZE * vtx_count);
    GLintptr vt_offset = VEC3_SIZE * vtx_count * 2;

    // vtx position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, VEC3_SIZE, (void*)v_offset);
    glEnableVertexAttribArray(0);

    // vtx normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, VEC3_SIZE, (void*)vn_offset);
    glEnableVertexAttribArray(1);

    // vtx texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, false, VEC2_SIZE, (void*)vt_offset);
    glEnableVertexAttribArray(2);

    /* -- Cleanup -- */

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    gfx_shader_use(NULL);

    // log_success("Mesh loaded: %s", name);
    return mesh;
}


void gfx_mesh_unload(GfxMesh* mesh){
    free(mesh);
}


GfxTexture* gfx_texture_load(u8* data, u32 width, u32 height, int gl_format) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));

    gfx_shader_use(self.shaders.object);
    glGenTextures(1, &(texture->id));
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    gfx_shader_use(NULL);
    return texture;
}


GfxTexture* gfx_texture_load_dds(u8* data, u32 width, u32 height, i32 gl_format, u32 mipmap_cnt, u32 block_size) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));

    gfx_shader_use(self.shaders.object);
    glGenTextures(1, &(texture->id));
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

    // log_success("Texture loaded: %s", texture_relpath);
    gfx_shader_use(NULL);
    return texture;
}


void gfx_texture_unload(GfxTexture* texture) {
    free(texture);
}


GfxGeometry* gfx_geometry_load(f32* vtx_buf, u64 vtx_count) {
    GfxGeometry* geom = malloc(sizeof(GfxGeometry));
    
    // TODO ...

    return geom;
}


void gfx_geometry_unload(GfxGeometry* geom) {
    free(geom);
}


/* ------------------------------------------------------------------------- */
/* Rendering Cycle */
/* ------------------------------------------------------------------------- */


void gfx_update_camera(mat4 m_persp, mat4 m_view) {
    gfx_shader_use(self.shaders.object);
    gfx_uniform_set_mat4(self.shaders.object, "m_persp", m_persp);
    gfx_uniform_set_mat4(self.shaders.object, "m_view", m_view);
    
    gfx_shader_use(self.shaders.object_outline);
    gfx_uniform_set_mat4(self.shaders.object_outline, "m_persp", m_persp);
    gfx_uniform_set_mat4(self.shaders.object_outline, "m_view", m_view);

    gfx_shader_use(NULL);
}


void gfx_begin_draw_objects() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(WINDOW_BG_COLOR);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    gfx_shader_use(self.shaders.object);
}

void gfx_end_draw_objects() {
    gfx_shader_use(NULL);
}


void gfx_draw_object(GfxMesh* mesh, GfxTexture* texture, mat4 m_model) {

    gfx_uniform_set_mat4(self.shaders.object, "m_model", m_model);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    glFrontFace(mesh->cw ? GL_CW : GL_CCW);
    glDrawElements(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
}


void gfx_draw_object_outlined(GfxMesh* mesh, GfxTexture* texture, mat4 m_model) {
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    gfx_draw_object(mesh, texture, m_model);
    
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);

    mat4 m_model_outline;
    glm_mat4_copy(m_model, m_model_outline);
    glm_scale(m_model_outline, (vec3){1.01, 1.01, 1.01});
    
    gfx_shader_use(self.shaders.object_outline);
    //
    gfx_uniform_set_mat4(self.shaders.object_outline, "m_model", m_model_outline);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    
    glFrontFace(mesh->cw ? GL_CW : GL_CCW);
    glDrawElements(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, NULL);
    //
    
    gfx_shader_use(self.shaders.object);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}
