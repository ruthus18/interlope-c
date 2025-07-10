#include <stdlib.h>
#include <string.h>

#include <cvector.h>
#include <cvector_utils.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "gfx.h"
#include "graphics/camera.h"
#include "graphics/shader.h"

#include "assets/font.h"
#include "core/cgm.h"
#include "core/config.h"
#include "core/log.h"
#include "core/types.h"
#include "platform/window.h"


typedef struct DrawObjectCommand {
    GfxMesh* mesh;
    GfxTexture* texture;
    mat4 m_model;
} DrawObjectCommand;

typedef struct DrawUIElementCommand {
    char* text;
    GfxMesh2D* ui_data;
    vec2 pos;
    vec3 color;
} DrawUIElementCommand;

typedef struct DrawGeometryCommand {
    GfxGeometry* geom;
    vec3 pos;
} DrawGeometryCommand;


static struct _Gfx {
    Window* window;
    Camera* camera;
    bool _stop;
    GfxSkybox* skybox;

    struct ShaderStorage {
        Shader* sky;
        Shader* object;
        Shader* ui;
        Shader* geometry;
    } shaders;

    struct CommandsStorage {
        cvector(DrawObjectCommand) object;
        cvector(DrawUIElementCommand) ui_element;
        cvector(DrawGeometryCommand) geometry;
    } commands;
} self = {};


/* ------ Shaders ------ */
/* ------------------------------------------------------------------------- */

static inline
void _init_shaders() {
    self.shaders.sky = shader_new(
        "sky", "sky.vert", "sky.frag"
    );
    self.shaders.object = shader_new(
        "object", "object.vert", "object.frag"
    );
    self.shaders.ui = shader_new(
        "ui", "ui.vert", "ui.frag"
    );
    self.shaders.geometry = shader_new(
        "geometry", "geometry.vert", "geometry.frag"
    );
}

static inline
void _destroy_shaders() {
    shader_free(self.shaders.sky);
    shader_free(self.shaders.object);
    shader_free(self.shaders.ui);
    shader_free(self.shaders.geometry);
}

/* ------ Command Storage ------ */
/* ------------------------------------------------------------------------- */

static inline
void _init_command_storage() {
    cvector_reserve(self.commands.object, 1024);
    cvector_reserve(self.commands.ui_element, 1024);
    cvector_reserve(self.commands.geometry, 1024);
}

static inline
void _destroy_command_storage() {
    cvector_free(self.commands.object);
    cvector_free(self.commands.ui_element);
    cvector_free(self.commands.geometry);
}

static inline
void _clear_command_storage() {
    cvector_clear(self.commands.object);
    cvector_clear(self.commands.ui_element);
    cvector_clear(self.commands.geometry);
}

/* ------------------------------------------------------------------------- */

static inline
void _log_startup_info() {
    log_greeting("======  Interlope Engine  ======");
    log_info("ENGINE VERSION: %s", ENGINE_VERSION);
    log_info("OPENGL VERSION: %s", glGetString(GL_VERSION));
    log_info("GLEW VERSION: %s", glewGetString(GLEW_VERSION));
    log_info("GLFW VERSION: %u.%u.%u", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
    log_info("VIDEO DEVICE: %s (%s)", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    log_info("WIN SIZE: %i x %i", Config.WINDOW_WIDTH, Config.WINDOW_HEIGHT);
    log_info("WIN VSYNC: %i", Config.WINDOW_VSYNC);
    log_info("------");
}


void gfx_init() {
    self.window = window_get();
    self._stop = false;

    _log_startup_info();
    _init_shaders();
    _init_command_storage();

    glPointSize(6);
    glLineWidth(2);

    glPolygonMode(GL_FRONT_AND_BACK, Config.GRAPHICS_WIREFRAME ? GL_LINE : GL_POLYGON);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void gfx_destroy() {
    _destroy_shaders();
    _destroy_command_storage();

    glfwDestroyWindow(self.window);
    glfwTerminate();
}


bool gfx_need_stop() {
    return glfwWindowShouldClose(self.window) || self._stop;
}

void gfx_stop() {
    self._stop = true;
    log_info("Engine stopped. Bye!");
}

void gfx_set_camera(Camera* camera) { self.camera = camera; }
void gfx_set_skybox(GfxSkybox* skybox) { self.skybox = skybox; }


/* ------------------------------------------------------------------------- */
/* Draw Commands Interface */
/* ------------------------------------------------------------------------- */

void gfx_enqueue_object(GfxMesh* mesh, GfxTexture* texture, mat4 m_model) {
    DrawObjectCommand cmd;
    cmd.mesh = mesh;
    cmd.texture = texture;
    glm_mat4_copy(m_model, cmd.m_model);

    cvector_push_back(self.commands.object, cmd);
}

void gfx_enqueue_ui_element(char* text, GfxMesh2D* ui_data, vec2 pos, vec3 color) {
    DrawUIElementCommand cmd;
    cmd.text = text;
    cmd.ui_data = ui_data;
    glm_vec2_copy(pos, cmd.pos);
    glm_vec3_copy(color, cmd.color);

    cvector_push_back(self.commands.ui_element, cmd);
}

void gfx_enqueue_geometry(GfxGeometry* geom, vec3 pos) {
    DrawGeometryCommand cmd;
    cmd.geom = geom;
    glm_vec3_copy(pos, cmd.pos);

    cvector_push_back(self.commands.geometry, cmd);
}


/* ------------------------------------------------------------------------- */
/* Rendering Cycle */
/* ------------------------------------------------------------------------- */


void gfx_draw_sky() {
    if (!self.skybox)  return;
    shader_use(self.shaders.sky);
    shader_set_mat4(self.shaders.sky, "m_persp", self.camera->m_persp);

    mat4 m_view;
    cgm_view_mat((vec3){0.0}, self.camera->v_front, m_view);
    shader_set_mat4(self.shaders.sky, "m_view", m_view);

    glDepthMask(GL_FALSE);
    glBindVertexArray(self.skybox->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self.skybox->vbo);

    glBindTexture(GL_TEXTURE_CUBE_MAP, self.skybox->texture->id);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthMask(GL_TRUE);
}


void gfx_draw_objects() {
    shader_use(self.shaders.object);
    shader_set_mat4(self.shaders.object, "m_persp", self.camera->m_persp);
    shader_set_mat4(self.shaders.object, "m_view", self.camera->m_view);

    DrawObjectCommand* cmd;

    cvector_for_each_in(cmd, self.commands.object) {
        shader_set_mat4(self.shaders.object, "m_model", cmd->m_model);
        
        glBindVertexArray(cmd->mesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, cmd->mesh->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd->mesh->ibo);
        
        glBindTexture(GL_TEXTURE_2D, cmd->texture->id);
        
        glFrontFace(cmd->mesh->cw ? GL_CW : GL_CCW);
        glDrawElements(GL_TRIANGLES, cmd->mesh->ind_count, GL_UNSIGNED_INT, NULL);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


void gfx_draw_ui_elements() {
    shader_use(self.shaders.ui);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Font* font = font_get_default();
    f32 scale = 0.5;

    DrawUIElementCommand* cmd;

    cvector_for_each_in(cmd, self.commands.ui_element) {
        f32 screen_x = cmd->pos[0] * Config.WINDOW_WIDTH;
        f32 screen_y = (1.0 - cmd->pos[1]) * Config.WINDOW_HEIGHT;
    
        shader_set_mat4(self.shaders.ui, "projection", cmd->ui_data->persp_mat);
        shader_set_vec3(self.shaders.ui, "text_color", cmd->color);
        
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(cmd->ui_data->vao);

        for (int i = 0; i < strlen(cmd->text); i++) {
            char ch = cmd->text[i];
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
            glBindBuffer(GL_ARRAY_BUFFER, cmd->ui_data->vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verticles), verticles);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
            screen_x += (glyph->advance >> 6) * scale;
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);

    /* --- Center Point --- */
    shader_use(NULL);
    glBegin(GL_POINTS);
    glColor3f(1.0, 1.0, 0.0);
    glVertex2f(0.0, 0.0);
    glEnd();
}


void gfx_draw_geometry() {
    shader_use(self.shaders.geometry);

    shader_set_mat4(self.shaders.geometry, "m_persp", self.camera->m_persp);
    shader_set_mat4(self.shaders.geometry, "m_view", self.camera->m_view);
    
    DrawGeometryCommand* cmd;

    cvector_for_each_in(cmd, self.commands.geometry) {
        shader_set_vec3(self.shaders.geometry, "v_color", cmd->geom->color);
        
        mat4 m_model;
        cgm_model_mat(cmd->pos, (vec3){0.0, 0.0, 0.0}, (vec3){1.0, 1.0, 1.0}, m_model);
        shader_set_mat4(self.shaders.geometry, "m_model", m_model);
        
        glBindVertexArray(cmd->geom->vao);
        glBindBuffer(GL_ARRAY_BUFFER, cmd->geom->vbo);
        glDrawArrays(GL_LINES, 0, cmd->geom->vtx_count);
    }   
}

/* ------------------------------------------------------------------------- */

#define BG_COLOR (f32)29 / 255, (f32)32 / 255, (f32)33 / 255, 1.0

void gfx_draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(BG_COLOR);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    gfx_draw_sky();
    gfx_draw_objects();
    gfx_draw_geometry();
    gfx_draw_ui_elements();

    _clear_command_storage();
}
