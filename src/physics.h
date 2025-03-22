#pragma once
#include <cglm/cglm.h>

#include "types.h"


void physics_init();
void physics_destroy();

void physics_create_ground();
void physics_create_cube(vec3 pos, vec3 size, f32 mass);
void physics_get_cube_position(vec3 dest);

void physics_update();
