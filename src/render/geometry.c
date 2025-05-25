
#include "../core/types.h"
#include "./gfx.h"


GfxGeometry* geometry_create_cube(f64 size_x, f64 size_y, f64 size_z) {
    f64 wx = size_x / 2;
    f64 wy = size_y / 2;
    f64 wz = size_z / 2;

    // int vtx_count = 18;
    f32 data[] = {
        // bottom
        -wx, -wy, -wz,    wx, -wy, -wz,
        -wx, -wy,  wz,    wx, -wy,  wz,
        -wx, -wy, -wz,   -wx, -wy,  wz,
         wx, -wy, -wz,    wx, -wy,  wz,
        // up
        -wx,  wy, -wz,    wx,  wy, -wz,
        -wx,  wy,  wz,    wx,  wy,  wz,
        -wx,  wy, -wz,   -wx,  wy,  wz,
         wx,  wy, -wz,    wx,  wy,  wz,
        // sides
        -wx, -wy, -wz,   -wx,  wy, -wz,
         wx, -wy, -wz,    wx,  wy, -wz,
        -wx, -wy,  wz,   -wx,  wy,  wz,
         wx, -wy,  wz,    wx,  wy,  wz,
    };
    int vtx_count = sizeof(data) / sizeof(f32) / 3;
    return gfx_geometry_load(data, vtx_count, (vec3){1.0, 1.0, 0.0});
}
