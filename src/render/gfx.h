/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include <cglm/cglm.h>

#include "render/camera.h"
#include "render/shader.h"
#include "render/resource.h"

#include "core/types.h"


/* ------ High-Level Interface ------ */

void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();

void gfx_set_camera(Camera* camera);

void gfx_enqueue_object(GfxMesh* mesh, GfxTexture* texture, mat4 m_model);
void gfx_enqueue_ui_element(char* text, GfxMesh2D* ui_data, vec2 pos, vec3 color);
void gfx_enqueue_geometry(GfxGeometry* geom, vec3 pos);

void gfx_draw();
