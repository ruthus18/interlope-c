#include <stdlib.h>
#include <cglm/cglm.h>

#include "camera.h"

#include "core/cgm.h"
#include "core/config.h"
#include "core/types.h"
#include "platform/time.h"


Camera* camera_create() {
    Camera* cam = malloc(sizeof(Camera));

    glm_vec3_copy((vec3){0.0, 0.0, 0.0}, cam->position);
    cam->pitch = 0.0;
    cam->yaw = 0.0;

    camera_update_persp_mat(cam);
    camera_update_view_mat(cam);

    return cam;
}

void camera_destroy(Camera* cam) {
    free(cam);
}

void camera_update_persp_mat(Camera* cam) {
    cgm_persp_mat(CAMERA_DEFAULT_FOV, cam->m_persp);
}

void camera_update_view_mat(Camera* cam) {
    cgm_view_mat(cam->position, cam->v_front, cam->m_view);
}


void camera_get_position(Camera* cam, vec3 dest) {
    glm_vec3_copy(cam->position, dest);
}


void camera_set_position(Camera* cam, vec3 pos) {
    glm_vec3_copy(pos, cam->position);
    camera_update_view_mat(cam);
}


void camera_transform(Camera* cam, vec3 pos_delta) {
    glm_vec3_add(cam->position, pos_delta, cam->position);
    camera_update_view_mat(cam);
}


void camera_set_rotation(Camera* cam, f64 yaw, f64 pitch) {
    cam->yaw = yaw;
    cam->pitch = pitch;

    if (cam->yaw > 360.0)   cam->yaw -= 360.0;
    if (cam->yaw < -360.0)  cam->yaw += 360.0;

    if (cam->pitch > 89.0)  cam->pitch = 89.0;
    if (cam->pitch < -89.0) cam->pitch = -89.0;

    cgm_front_vec(cam->yaw, cam->pitch, cam->v_front);
    camera_update_view_mat(cam);
}


void camera_rotate(Camera* cam, f64 yaw_delta, f64 pitch_delta) {
    cam->yaw += yaw_delta;
    cam->pitch += pitch_delta;

    if (cam->yaw > 360.0)   cam->yaw -= 360.0;
    if (cam->yaw < -360.0)  cam->yaw += 360.0;

    if (cam->pitch > 89.0)  cam->pitch = 89.0;
    if (cam->pitch < -89.0) cam->pitch = -89.0;

    cgm_front_vec(cam->yaw, cam->pitch, cam->v_front);
    camera_update_view_mat(cam);
}


/*
    IMPORTANT: You should call `camera_player_rotate` first to update front vector
*/
void camera_player_transform(Camera* cam, bool w, bool s, bool a, bool d) {
    vec3 v_delta;
    cgm_wsad_vec(cam->v_front, w, s, a, d, v_delta);

    glm_vec3_scale(v_delta, time_get_dt() * CAMERA_MOVEMENT_SPEED * 0.5, v_delta);
    camera_transform(cam, v_delta);
}


void camera_player_rotate(Camera* cam, vec2 mouse_dt) {
    double yaw_delta =  mouse_dt[0] * CAMERA_SENSITIVITY;
    double pitch_delta = -mouse_dt[1] * CAMERA_SENSITIVITY;

    camera_rotate(cam, yaw_delta, pitch_delta);
}
