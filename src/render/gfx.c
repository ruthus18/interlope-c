#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "gfx.h"
#include "./gfx_shader.h"
#include "./camera.h"
#include "../core/cgm.h"
#include "../core/config.h"
#include "../core/log.h"
#include "../core/types.h"
#include "../platform/window.h"


static struct _Gfx {
    Window* window;
    bool stop_;

    struct {
        Shader* object;
        Shader* object_outline;
        Shader* geometry;
    } shaders;

    int outline_id;
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
        "object_outline.vert", "object_outline.frag"
    );
    self.shaders.geometry = gfx_shader_create(
        "geometry.vert", "geometry.frag"
    );
}


static inline
void _destroy_shaders() {
    gfx_shader_destroy(self.shaders.object);
    gfx_shader_destroy(self.shaders.object_outline);
    gfx_shader_destroy(self.shaders.geometry);
}


void gfx_init() {
    self.window = window_get();
    self.stop_ = false;
    self.outline_id = -1;

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
    vec3 color;
} GfxGeometry;


#define VEC3_SIZE (3 * sizeof(f32))
#define VEC2_SIZE (2 * sizeof(f32))


GfxMesh* gfx_mesh_load(const char* name, f32* vtx_buf, u32* ind_buf, u64 vtx_count, u64 ind_count, bool cw) {
    GfxMesh* mesh = malloc(sizeof(GfxMesh));
    if (!mesh) {
        log_error("Failed to allocate memory for GfxMesh");
        return NULL;
    }

    mesh->name = strdup(name ? name : "[unnamed_mesh]");
    if (!mesh->name) {
        log_error("Failed to duplicate mesh name string");
        free(mesh);
        return NULL;
    }

    mesh->vtx_count = vtx_count;
    mesh->ind_count = ind_count;
    mesh->cw = cw;

    gfx_shader_use(self.shaders.object);

    /* ------ VAO ------ */
    glGenVertexArrays(1, &(mesh->vao));
    glBindVertexArray(mesh->vao);

    /* ------ VBO ------ */
    glGenBuffers(1, &(mesh->vbo));
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    GLsizeiptr total_buffer_size = 
        (VEC3_SIZE * vtx_count) + (VEC3_SIZE * vtx_count) + (VEC2_SIZE * vtx_count);

    glBufferData(GL_ARRAY_BUFFER, total_buffer_size, vtx_buf, GL_STATIC_DRAW);

    /* ------ IBO ------ */
    glGenBuffers(1, &(mesh->ibo));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * ind_count, ind_buf, GL_STATIC_DRAW);

    /* ------ Vertex Attributes ------ */
    // Vertex buffer format is planar: (PPP...NNN...TTT...)
    GLintptr pos_offset = 0;
    GLintptr norm_offset = VEC3_SIZE * vtx_count;
    GLintptr uv_offset = (VEC3_SIZE * vtx_count) + (VEC3_SIZE * vtx_count);

    // vtx position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)pos_offset);
    glEnableVertexAttribArray(0);

    // vtx normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)norm_offset);
    glEnableVertexAttribArray(1);

    // vtx texcoord (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)uv_offset);
    glEnableVertexAttribArray(2);

    /* ------ Cleanup ------ */

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    gfx_shader_use(NULL);

    // log_success("Mesh loaded: %s", name);
    return mesh;
}


void gfx_mesh_unload(GfxMesh* mesh){
    if (!mesh) return;

    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ibo);

    free((void*)mesh->name);
    free(mesh);
}


GfxTexture* gfx_texture_load(u8* data, u32 width, u32 height, int gl_format) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));
    if (!texture) {
        log_error("Failed to allocate memory for GfxTexture (DDS)");
        return NULL;
    }
    if (!texture) {
        log_error("Failed to allocate memory for GfxTexture");
        return NULL;
    }

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
    glDeleteTextures(1, &texture->id);
    free(texture);
}


GfxGeometry* gfx_geometry_load(f32* lines_buf, u64 vtx_count, vec3 color) {
    GfxGeometry* geom = malloc(sizeof(GfxGeometry));
    
    gfx_shader_use(self.shaders.geometry);
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vtx_count * 3, lines_buf, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    geom->vao = VAO;
    geom->vbo = VBO;
    geom->vtx_count = vtx_count;
    glm_vec3_copy(color, geom->color);

    gfx_shader_use(NULL);
    return geom;
}


void gfx_geometry_unload(GfxGeometry* geom) {
    free(geom);
}


/* ------------------------------------------------------------------------- */
/* Rendering Cycle */
/* ------------------------------------------------------------------------- */


// TODO: save data to gfx, fill vars in shader loop
void gfx_update_camera(Camera* cam) {
    gfx_shader_use(self.shaders.object);
    gfx_uniform_set_mat4(self.shaders.object, "m_persp", cam->m_persp);
    gfx_uniform_set_mat4(self.shaders.object, "m_view", cam->m_view);
    
    gfx_shader_use(self.shaders.object_outline);
    gfx_uniform_set_mat4(self.shaders.object_outline, "m_persp", cam->m_persp);
    gfx_uniform_set_mat4(self.shaders.object_outline, "m_view", cam->m_view);

    gfx_shader_use(self.shaders.geometry);
    gfx_uniform_set_mat4(self.shaders.geometry, "m_persp", cam->m_persp);
    gfx_uniform_set_mat4(self.shaders.geometry, "m_view", cam->m_view);

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


void gfx_draw_object_outlined__enter(GfxMesh* mesh, GfxTexture* texture, mat4 m_model) {
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
}


void gfx_draw_object_outlined__exit(GfxMesh* mesh, GfxTexture* texture, mat4 m_model) {    
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);

    gfx_shader_use(self.shaders.object_outline);
    
    gfx_uniform_set_mat4(self.shaders.object_outline, "m_model", m_model);
    
    // Set the outline thickness - adjust based on mesh size if needed
    // float outline_thickness = 0.02;
    // gfx_uniform_set_float(self.shaders.object_outline, "outline_thickness", outline_thickness);
    
    // Set both gradient colors
    // vec3 outline_color1 = {0.8, 0.25, 0.0};
    // vec3 outline_color2 = {0.0, 0.8, 0.2};
    // gfx_uniform_set_vec3(self.shaders.object_outline, "outline_color1", outline_color1);
    // gfx_uniform_set_vec3(self.shaders.object_outline, "outline_color2", outline_color2);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    
    glFrontFace(mesh->cw ? GL_CW : GL_CCW);
    glDrawElements(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, NULL);
    
    gfx_shader_use(self.shaders.object);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDisable(GL_STENCIL_TEST);
}


void gfx_draw_object(GfxMesh* mesh, GfxTexture* texture, mat4 m_model, i32 id) {
    if (id == self.outline_id) {
        gfx_draw_object_outlined__enter(mesh, texture, m_model);
    }

    gfx_uniform_set_mat4(self.shaders.object, "m_model", m_model);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    glFrontFace(mesh->cw ? GL_CW : GL_CCW);
    glDrawElements(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (id == self.outline_id) {
        gfx_draw_object_outlined__exit(mesh, texture, m_model);
    }
}

void gfx_set_outline_id(i32 value) {
    self.outline_id = value;
}


void gfx_begin_draw_geometry() {
    gfx_shader_use(self.shaders.geometry);
    glDisable(GL_DEPTH_TEST);
}

void gfx_end_draw_geometry() {
    gfx_shader_use(NULL);
}

void gfx_draw_geometry(GfxGeometry* geom, vec3 pos) {
    gfx_uniform_set_vec3(self.shaders.geometry, "v_color", geom->color);

    mat4 m_model;
    cgm_model_mat(pos, (vec3){0.0, 0.0, 0.0}, (vec3){1.0, 1.0, 1.0}, m_model);
    gfx_uniform_set_mat4(self.shaders.geometry, "m_model", m_model);

    glBindVertexArray(geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    glDrawArrays(GL_LINES, 0, geom->vtx_count);
}
