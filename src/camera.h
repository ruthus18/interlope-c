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

void camera_set_position(Camera*, vec3 pos);
void camera_rotate(Camera*, double yaw_delta, double pitch_delta);
void camera_update_gfx_data(Camera*);
void camera_player_control(Camera*);
