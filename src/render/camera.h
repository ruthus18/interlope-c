#pragma once

#include <cglm/cglm.h>
#include "../core/types.h"


typedef struct Camera {
    vec3 position;
    f64 yaw;
    f64 pitch;

    vec3 v_front;

    mat4 m_persp;
    mat4 m_view;

} Camera;


Camera* camera_create();
void camera_destroy(Camera*);
void camera_update_persp_mat(Camera* cam);
void camera_update_view_mat(Camera* cam);

void camera_get_position(Camera* cam, vec3 dest);
void camera_set_position(Camera*, vec3 pos);

void camera_transform(Camera*, vec3 pos_delta);
void camera_set_rotation(Camera*, f64 yaw, f64 pitch);
void camera_rotate(Camera*, f64 yaw_delta, f64 pitch_delta);

void get_wsad_movement_vec(Camera* cam, bool w, bool s, bool a, bool d, vec3 dest);
void get_orient_vec(vec2 dest);

void camera_player_control(Camera*, bool w, bool s, bool a, bool d);
void camera_upload_to_gfx(Camera*);