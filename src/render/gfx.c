#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "gfx.h"
#include "render/shader.h"
#include "render/camera.h"

#include "assets/font.h"
#include "core/cgm.h"
#include "core/config.h"
#include "core/log.h"
#include "core/types.h"
#include "platform/window.h"


static struct _Gfx {
    Window* window;
    bool stop_;

    struct {
        Shader* sky;
        Shader* object;
        Shader* ui;
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
    log_info("GLFW VERSION: %u.%u.%u", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
    log_info("VIDEO DEVICE: %s (%s)", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    log_info("WIN SIZE: %i x %i", WINDOW_WIDTH, WINDOW_HEIGHT);
    log_info("WIN VSYNC: %i", WINDOW_VSYNC);
    log_info("------");
}


static inline
void _init_shaders() {
    self.shaders.sky = shader_new(
        "sky.vert", "sky.frag"
    );
    self.shaders.object = shader_new(
        "object.vert", "object.frag"
    );
    self.shaders.ui = shader_new(
        "ui.vert", "ui.frag"
    );
    self.shaders.geometry = shader_new(
        "geometry.vert", "geometry.frag"
    );
}


static inline
void _destroy_shaders() {
    shader_free(self.shaders.sky);
    shader_free(self.shaders.object);
    shader_free(self.shaders.ui);
    shader_free(self.shaders.geometry);
}


void gfx_init() {
    self.window = window_get();
    self.stop_ = false;

    _log_startup_info();
    _init_shaders();

    glPointSize(6);
    glLineWidth(2);

    glPolygonMode(GL_FRONT_AND_BACK, GFX_WIREFRAME_MODE ? GL_LINE : GL_POLYGON);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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


typedef struct GfxUI {
    u32 vao;
    u32 vbo;
    mat4 persp_mat;
} GfxUI;


#define VEC3_SIZE (3 * sizeof(f32))
#define VEC2_SIZE (2 * sizeof(f32))


GfxMesh* gfx_load_mesh(
    const char* name, f32* vtx_buf, u32* ind_buf, u64 vtx_count, u64 ind_count, bool cw
) {
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

    shader_use(self.shaders.object);

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
    shader_use(NULL);

    // log_success("Mesh loaded: %s", name);
    return mesh;
}


void gfx_unload_mesh(GfxMesh* mesh){
    if (!mesh) return;

    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ibo);

    free((void*)mesh->name);
    free(mesh);
}


GfxTexture* gfx_load_texture(u8* data, u32 width, u32 height, int gl_format) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));
    if (!texture) {
        log_error("Failed to allocate memory for GfxTexture");
        return NULL;
    }

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


GfxTexture* gfx_load_dds_texture(u8* data, u32 width, u32 height, i32 gl_format, u32 mipmap_cnt, u32 block_size) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));
    if (!texture) {
        log_error("Failed to allocate memory for GfxTexture (DDS)");
        return NULL;
    }

    shader_use(self.shaders.object);
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
    shader_use(NULL);
    return texture;
}

GfxTexture* gfx_load_font_texture(u32 width, u32 height, void* data) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));
    if (!texture) {
        log_error("Failed to allocate memory for GfxTexture (Font)");
        return NULL;
    }
    shader_use(self.shaders.ui);
    
    glGenTextures(1, &(texture->id));
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    shader_use(NULL);
    return texture;
}


void gfx_unload_texture(GfxTexture* texture) {
    if (!texture)  return;

    glDeleteTextures(1, &texture->id);
    free(texture);
}


GfxGeometry* gfx_load_geometry(f32* lines_buf, u64 vtx_count, vec3 color) {
    GfxGeometry* geom = malloc(sizeof(GfxGeometry));
    u32 VAO, VBO;
    
    shader_use(self.shaders.geometry);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

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

    shader_use(NULL);
    return geom;
}

void gfx_unload_geometry(GfxGeometry* geom) {
    free(geom);
}


GfxUI* gfx_load_ui_data() {
    GfxUI* ui_data = malloc(sizeof(GfxUI));
    u32 VAO, VBO;

    shader_use(self.shaders.ui);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ui_data->vao = VAO;
    ui_data->vbo = VBO;
    glm_ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, -1.0f, 1.0f, ui_data->persp_mat);

    shader_use(NULL);
    return ui_data;
}

void gfx_unload_ui_data(GfxUI* ui_data) {
    free(ui_data);
}


/* ------------------------------------------------------------------------- */
/* Rendering Cycle */
/* ------------------------------------------------------------------------- */


// TODO: save data to gfx, fill vars in shader loop
void gfx_update_camera(Camera* cam) {
    shader_use(self.shaders.object);
    uniform_set_mat4(self.shaders.object, "m_persp", cam->m_persp);
    uniform_set_mat4(self.shaders.object, "m_view", cam->m_view);

    shader_use(self.shaders.geometry);
    uniform_set_mat4(self.shaders.geometry, "m_persp", cam->m_persp);
    uniform_set_mat4(self.shaders.geometry, "m_view", cam->m_view);

    shader_use(NULL);
}


void gfx_begin_draw_objects() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(WINDOW_BG_COLOR);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    shader_use(self.shaders.object);
}

void gfx_end_draw_objects() {
    shader_use(NULL);
}


void gfx_draw_object(GfxMesh* mesh, GfxTexture* texture, mat4 m_model, i32 id) {
    uniform_set_mat4(self.shaders.object, "m_model", m_model);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    glFrontFace(mesh->cw ? GL_CW : GL_CCW);
    glDrawElements(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void gfx_set_outline_id(i32 value) {
    self.outline_id = value;
}

/* ------------------------------------------------------------------------- */

void gfx_begin_draw_ui() {
    shader_use(self.shaders.ui);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void gfx_end_draw_ui() {
    glEnable(GL_BLEND);
    shader_use(NULL);
}

void gfx_draw_ui(char* text, GfxUI* ui_data, vec2 pos, vec3 color) {
    Font* font = font_get_default();
    f32 scale = 0.5;
    
    // Convert normalized coordinates [0.0-1.0] to screen pixels (top-left origin)
    f32 screen_x = pos[0] * WINDOW_WIDTH;
    f32 screen_y = (1.0 - pos[1]) * WINDOW_HEIGHT;

    uniform_set_mat4(self.shaders.ui, "projection", ui_data->persp_mat);
    uniform_set_vec3(self.shaders.ui, "text_color", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(ui_data->vao);

    for (int i = 0; i < strlen(text); i++) {
        char ch = text[i];
        Glyph* glyph = &font->chars[(unsigned char)ch];

        f32 xpos = screen_x + glyph->bearing[0] * scale;
        f32 ypos = screen_y - (glyph->size[1] - glyph->bearing[1]) * scale;
        
        f32 w = glyph->size[0] * scale;
        f32 h = glyph->size[1] * scale;

        f32 verticles[6][4] = {
            {xpos,      ypos + h,   0.0, 0.0},
            {xpos,      ypos,       0.0, 1.0},
            {xpos + w,  ypos,       1.0, 1.0},

            {xpos,      ypos + h,   0.0, 0.0},
            {xpos + w,  ypos,       1.0, 1.0},
            {xpos + w,  ypos + h,   1.0, 0.0}
        };
        glBindTexture(GL_TEXTURE_2D, glyph->texture->id);
        glBindBuffer(GL_ARRAY_BUFFER, ui_data->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verticles), verticles);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        screen_x += (glyph->advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/* ------------------------------------------------------------------------- */

void gfx_begin_draw_geometry() {
    shader_use(self.shaders.geometry);
    glDisable(GL_DEPTH_TEST);
}

void gfx_end_draw_geometry() {
    glEnable(GL_DEPTH_TEST);
    shader_use(NULL);
}

void gfx_draw_geometry(GfxGeometry* geom, vec3 pos) {
    uniform_set_vec3(self.shaders.geometry, "v_color", geom->color);
    
    mat4 m_model;
    cgm_model_mat(pos, (vec3){0.0, 0.0, 0.0}, (vec3){1.0, 1.0, 1.0}, m_model);
    uniform_set_mat4(self.shaders.geometry, "m_model", m_model);
    
    glBindVertexArray(geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    glDrawArrays(GL_LINES, 0, geom->vtx_count);
}

/* ------------------------------------------------------------------------- */
