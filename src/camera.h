#pragma once
#include <cglm/cglm.h>
#include "gfx.h"


typedef struct Camera {
    vec3 position;
    double yaw;
    double pitch;

    vec3 v_front;

    GfxCamera gfxd;  // Camera GFX data

} Camera;


Camera* camera_create();
void camera_destroy(Camera*);
void camera_update_persp_mat(Camera* cam);
void camera_update_view_mat(Camera* cam);

void camera_set_position(Camera*, vec3 pos);
void camera_set_rotation(Camera*, double yaw, double pitch);
void camera_rotate(Camera*, double yaw_delta, double pitch_delta);
void camera_player_control(Camera*);
