#include <cglm/cglm.h>

#include "geometry.h"

#include "core/types.h"
#include "graphics/gfx.h"
#include "graphics/geometry.h"


static GfxGeometry* axis_geom[3];
static GfxGeometry* cube_geom;


void editor_geometry_init() {
    f32 axis_x[] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    f32 axis_y[] = {0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    f32 axis_z[] = {0.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    axis_geom[0] = gfx_load_geometry(axis_x, 2, (vec3){1.0, 0.0, 0.0});
    axis_geom[1] = gfx_load_geometry(axis_y, 2, (vec3){0.0, 1.0, 0.0});
    axis_geom[2] = gfx_load_geometry(axis_z, 2, (vec3){0.0, 0.0, 1.0});

    cube_geom = geometry_create_cube(2.0, 2.0, 2.0);
}


void editor_geometry_destroy() {
    gfx_unload_geometry(axis_geom[0]);
    gfx_unload_geometry(axis_geom[1]);
    gfx_unload_geometry(axis_geom[2]);

    gfx_unload_geometry(cube_geom);
}


void editor_geometry_draw() {
    gfx_enqueue_geometry(axis_geom[0], (vec3){0.0, 0.0, 0.0});
    gfx_enqueue_geometry(axis_geom[1], (vec3){0.0, 0.0, 0.0});
    gfx_enqueue_geometry(axis_geom[2], (vec3){0.0, 0.0, 0.0});

    gfx_enqueue_geometry(cube_geom, (vec3){-3.0, 1.0, 0.0});
}
