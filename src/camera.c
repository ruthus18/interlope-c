#include <stdlib.h>
#include <cglm/cglm.h>

#include "camera.h"
#include "cgm.h"
#include "config.h"
#include "platform/input.h"

#include "log.h"


Camera* camera_create() {
    Camera* cam = malloc(sizeof(Camera));

    camera_set_position(cam, (vec3){0.0, 0.0, 0.0});
    camera_set_rotation(cam, 270.0, 0.0);

    camera_update_persp_mat(cam);
    camera_update_view_mat(cam);

    return cam;
}

void camera_destroy(Camera* cam) {
    free(cam);
}

void camera_update_persp_mat(Camera* cam) {
    cgm_persp_mat(CAMERA_DEFAULT_FOV, cam->gfxd.m_persp);
}

void camera_update_view_mat(Camera* cam) {
    cgm_view_mat(cam->position, cam->v_front, cam->gfxd.m_view);
}


void camera_set_position(Camera* cam, vec3 pos) {
    glm_vec3_copy(pos, cam->position);
    camera_update_view_mat(cam);
}


void camera_transform(Camera* cam, vec3 pos_delta) {
    glm_vec3_add(cam->position, pos_delta, cam->position);
    camera_update_view_mat(cam);
}


void camera_set_rotation(Camera* cam, double yaw, double pitch) {
    cam->yaw = yaw;
    cam->pitch = pitch;

    if (cam->yaw > 360.0)   cam->yaw -= 360.0;
    if (cam->yaw < -360.0)  cam->yaw += 360.0;

    if (cam->pitch > 89.0)  cam->pitch = 89.0;
    if (cam->pitch < -89.0) cam->pitch = -89.0;

    cgm_front_vec(cam->yaw, cam->pitch, cam->v_front);
    camera_update_view_mat(cam);
}


void camera_rotate(Camera* cam, double yaw_delta, double pitch_delta) {
    cam->yaw += yaw_delta;
    cam->pitch += pitch_delta;

    if (cam->yaw > 360.0)   cam->yaw -= 360.0;
    if (cam->yaw < -360.0)  cam->yaw += 360.0;

    if (cam->pitch > 89.0)  cam->pitch = 89.0;
    if (cam->pitch < -89.0) cam->pitch = -89.0;

    cgm_front_vec(cam->yaw, cam->pitch, cam->v_front);
    camera_update_view_mat(cam);
}


void camera_player_control(Camera* cam, bool w, bool s, bool a, bool d) {
    /* Rotation Handling */

    vec2 mouse_delta;
    double yaw_delta, pitch_delta;

    input_get_mouse_delta(mouse_delta);

      yaw_delta =  mouse_delta[0] * MOUSE_SENSITIVITY;
    pitch_delta = -mouse_delta[1] * MOUSE_SENSITIVITY;

    camera_rotate(cam, yaw_delta, pitch_delta);

    if (__DEBUG__LOG_CAMERA_ROTATION)
        log_info("CAM[YAW:PITCH]  %f | %f", cam->yaw, cam->pitch);

    /* Movement Handling */

    vec3 v_movement_forwd = {cam->v_front[0], 0.0, cam->v_front[2]};
    vec3 v_movement_slide;
    glm_vec3_cross(v_movement_forwd, (vec3){0.0, 1.0, 0.0}, v_movement_slide);

    vec3 v_delta;
    glm_vec3_zero(v_delta);

    if (w)  glm_vec3_add(v_delta, v_movement_forwd, v_delta);
    if (s)  glm_vec3_sub(v_delta, v_movement_forwd, v_delta);
    if (a)  glm_vec3_sub(v_delta, v_movement_slide, v_delta);
    if (d)  glm_vec3_add(v_delta, v_movement_slide, v_delta);

    if (!glm_vec3_eq(v_delta, 0.0)) {
        glm_vec3_normalize(v_delta);
    }
    glm_vec3_scale(v_delta, 0.01 * CAMERA_MOVEMENT_SPEED, v_delta);
    camera_transform(cam, v_delta);

    if (__DEBUG__LOG_CAMERA_POSITION)
        log_info("CAM[POS]  %f  %f  %f", cam->position[0], cam->position[1], cam->position[2]);

    if (__DEBUG__LOG_CAMERA_POSITION_DELTA)
        log_info("CAM[POS Δ]  %f  %f  %f", v_delta[0], v_delta[1], v_delta[2]);
}   
