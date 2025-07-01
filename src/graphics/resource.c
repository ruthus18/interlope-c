#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "resource.h"

#include "core/config.h"
#include "core/log.h"


#define VEC3_SIZE (3 * sizeof(f32))
#define VEC2_SIZE (2 * sizeof(f32))


/* ------ GfxMesh ------ */
/* ------------------------------------------------------------------------- */

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

/* ------ GfxMesh2D ------ */
/* ------------------------------------------------------------------------- */

GfxMesh2D* gfx_load_mesh_2d() {
    GfxMesh2D* ui_data = malloc(sizeof(GfxMesh2D));
    u32 VAO, VBO;

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
    return ui_data;
}

void gfx_unload_mesh_2d(GfxMesh2D* ui_data) {
    free(ui_data);
}

/* ------ GfxTexture ------ */
/* ------------------------------------------------------------------------- */

GfxTexture* gfx_load_texture(u8* data, u32 width, u32 height, i32 gl_format, u32 mipmap_cnt, u32 block_size) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));
    if (!texture) {
        log_error("Failed to allocate memory for GfxTexture (DDS)");
        return NULL;
    }

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

    return texture;
}

GfxTexture* gfx_load_font_texture(u32 width, u32 height, void* data) {
    GfxTexture* texture = malloc(sizeof(GfxTexture));
    if (!texture) {
        log_error("Failed to allocate memory for GfxTexture (Font)");
        return NULL;
    }

    glGenTextures(1, &(texture->id));
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

void gfx_unload_texture(GfxTexture* texture) {
    if (!texture)  return;

    glDeleteTextures(1, &texture->id);
    free(texture);
}

/* ------ GfxGeometry ------ */
/* ------------------------------------------------------------------------- */

GfxGeometry* gfx_load_geometry(f32* lines_buf, u64 vtx_count, vec3 color) {
    GfxGeometry* geom = malloc(sizeof(GfxGeometry));
    u32 VAO, VBO;
    
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
    return geom;
}

void gfx_unload_geometry(GfxGeometry* geom) {
    free(geom);
}
