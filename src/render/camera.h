#pragma once

#include <cglm/cglm.h>
#include "core/types.h"


typedef struct Camera {
    vec3 position;
    vec2 direction;

    vec3 v_front;
    mat4 m_persp;
    mat4 m_view;
} Camera;


Camera* camera_create(vec3 position, vec2 direction);
void camera_destroy(Camera*);

void camera_set_position(Camera*, vec3 pos);
void camera_set_rotation(Camera*, vec2 direction);
