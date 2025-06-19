#include <cglm/cglm.h>

#include "geometry.h"

#include "core/types.h"
#include "render/gfx.h"
#include "render/geometry.h"


static GfxGeometry* axis_geom[3];
static GfxGeometry* cube_geom;


void editor_geometry_init() {
    f32 axis_x[] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    f32 axis_y[] = {0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    f32 axis_z[] = {0.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    axis_geom[0] = gfx_geometry_load(axis_x, 2, (vec3){1.0, 0.0, 0.0});
    axis_geom[1] = gfx_geometry_load(axis_y, 2, (vec3){0.0, 1.0, 0.0});
    axis_geom[2] = gfx_geometry_load(axis_z, 2, (vec3){0.0, 0.0, 1.0});

    // cube_geom = geometry_create_cube(2.0, 2.0, 2.0);
}


void editor_geometry_destroy() {
    gfx_geometry_unload(axis_geom[0]);
    gfx_geometry_unload(axis_geom[1]);
    gfx_geometry_unload(axis_geom[2]);

    // gfx_geometry_unload(cube_geom);
}


void editor_geometry_draw() {
    gfx_begin_draw_geometry();

    gfx_draw_geometry(axis_geom[0], (vec3){0.0, 0.0, 0.0});
    gfx_draw_geometry(axis_geom[1], (vec3){0.0, 0.0, 0.0});
    gfx_draw_geometry(axis_geom[2], (vec3){0.0, 0.0, 0.0});
    // gfx_draw_geometry(cube_geom, (vec3){0.0, 1.0, 0.0});

    gfx_end_draw_geometry();
}
