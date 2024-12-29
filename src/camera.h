#pragma once
#include <cglm/cglm.h>


typedef struct Camera {
    vec3 position;
    vec3 rotation;

    vec3 v_front;
    vec3 v_center;
    vec3 v_up;

    struct {
        mat4 m_persp;
        mat4 m_view;
    } gfx_data;

} Camera;


Camera* camera_create();
void camera_destroy(Camera*);

void camera_set_position(Camera*, vec3);
void camera_set_rotation(Camera*, vec3);
void camera_recalc(Camera* cam);
