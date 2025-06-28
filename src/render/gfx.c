#include <stdlib.h>
#include <string.h>

#define CVECTOR_LINEAR_GROWTH
#include <cvector.h>
#include <cvector_utils.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "gfx.h"
#include "render/camera.h"
#include "render/shader.h"

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
    
    struct ShaderStorage {
        Shader* sky;
        Shader* object;
        Shader* ui;
        Shader* geometry;
    } shaders;

    struct CommandsStorage {
        DrawObjectCommand* object;
        DrawUIElementCommand* ui_element;
        DrawGeometryCommand* geometry;
    } commands;
} self;


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
    cvector(DrawObjectCommand) cmds_obj = NULL;
    cvector_reserve(cmds_obj, 1024);
    self.commands.object = cmds_obj;
    
    cvector(DrawUIElementCommand) cmds_ui = NULL;
    cvector_reserve(cmds_ui, 1024);
    self.commands.ui_element = cmds_ui;
    
    cvector(DrawGeometryCommand) cmds_geom = NULL;
    cvector_reserve(cmds_geom, 1024);
    self.commands.geometry = cmds_geom;
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
    log_info("WIN SIZE: %i x %i", WINDOW_WIDTH, WINDOW_HEIGHT);
    log_info("WIN VSYNC: %i", WINDOW_VSYNC);
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

    glPolygonMode(GL_FRONT_AND_BACK, GFX_WIREFRAME_MODE ? GL_LINE : GL_POLYGON);
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

void gfx_set_camera(Camera* camera) {
    self.camera = camera;
}


/* ------------------------------------------------------------------------- */
/* Rendering Cycle */
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
        f32 screen_x = cmd->pos[0] * WINDOW_WIDTH;
        f32 screen_y = (1.0 - cmd->pos[1]) * WINDOW_HEIGHT;
    
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
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_BLEND);
}


void gfx_draw_geometry() {
    shader_use(self.shaders.geometry);
    glDisable(GL_DEPTH_TEST);
    
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
    
    glEnable(GL_DEPTH_TEST);
}

/* ------------------------------------------------------------------------- */

void gfx_draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(WINDOW_BG_COLOR);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    gfx_draw_objects();
    gfx_draw_geometry();
    gfx_draw_ui_elements();

    _clear_command_storage();
}
