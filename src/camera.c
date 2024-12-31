#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <cglm/cglm.h>

#include "camera.h"
#include "cgm.h"
#include "config.h"
#include "platform.h"

#include "log.h"


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


Camera* camera_create() {
    Camera* cam = malloc(sizeof(Camera));
    // point to [0;0] if pos on -Z axis
    cam->yaw = -270.0;
    cam->pitch = 0.0;

    glm_vec3_copy((vec3){0.0, 1.7, 0.0}, cam->position);
    glm_vec3_copy((vec3){0.0, 0.0, 1.0},  cam->v_front);

    return cam;
}

void camera_destroy(Camera* cam) {
    free(cam);
}

void camera_set_position(Camera* cam, vec3 pos) {
    glm_vec3_copy(pos, cam->position);
}


void camera_rotate(Camera* cam, double yaw_delta, double pitch_delta) {
    cam->yaw += yaw_delta;
    cam->pitch += pitch_delta;

    /* Clamp extreme values */
    if (cam->yaw > 360.0)   cam->yaw -= 360.0;
    if (cam->yaw < -360.0)  cam->yaw += 360.0;

    if (cam->pitch > 89.0)  cam->pitch = 89.0;
    if (cam->pitch < -89.0) cam->pitch = -89.0;

    /* Update front vector */
    cgm_front_vec(cam->yaw, cam->pitch, cam->v_front);

    if (__DEBUG__LOG_CAMERA_ROTATION)
        log_info("CAM[PITCH:YAW]  %f | %f", cam->yaw, cam->pitch);
}


bool __persp_mat = false;

void camera_update_gfx_data(Camera* cam) {
    if (!__persp_mat) {
        cgm_persp_mat(CAMERA_DEFAULT_FOV, cam->gfxd.m_persp);
        __persp_mat = true;
    }

    cgm_view_mat(cam->position, cam->v_front, cam->gfxd.m_view);
}


void camera_player_control(Camera* cam) {
    vec2 mouse_delta;
    double yaw_delta, pitch_delta;

    input_get_mouse_delta(mouse_delta);

      yaw_delta =  mouse_delta[0] * MOUSE_SENSITIVITY;
    pitch_delta = -mouse_delta[1] * MOUSE_SENSITIVITY;

    camera_rotate(cam, yaw_delta, pitch_delta);
}
