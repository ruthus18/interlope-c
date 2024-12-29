#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <cglm/cglm.h>

#include "camera.h"
#include "config.h"


#define __CAMERA_FOV 75.0
#define radian(x) (x / (180.0 / GLM_PI))


Camera* camera_create() {
    Camera* camera = malloc(sizeof(Camera));

    glm_vec3_zero(camera->position);
    glm_vec3_zero(camera->rotation);
    glm_vec3_copy((vec3){0.0, 1.0, 0.0}, camera->v_up);
    return camera;
}

void camera_destroy(Camera* cam) {
    free(cam);
}

void camera_set_position(Camera* cam, vec3 pos) {
    glm_vec3_copy(cam->position, pos);
}

void camera_set_rotation(Camera* cam, vec3 rot) {
    glm_vec3_copy(cam->rotation, rot);
}


bool __first = true;

void camera_recalc(Camera* cam) {
    if (__first) {
        glm_perspective(
            radian(__CAMERA_FOV),
            WINDOW_ASPECT,
            0.1, 1000.0,
            cam->gfx_data.m_persp
        );
        __first = false;
    }

    float yaw = cam->rotation[1];  // yaw
    float pitch = cam->rotation[0];  // pitch

    // Front vector
    glm_vec3_copy(
        (vec3){
            cos(radian(yaw) * cos(radian(pitch))),
            sin(radian(pitch)),
            sin(radian(yaw)) * cos(radian(pitch))
        },
        cam->v_front
    );
    glm_normalize(cam->v_front);

    // Center vector
    glm_vec3_add(cam->position, cam->v_front, cam->v_center);

    // Final view matrix
    glm_lookat(cam->position, cam->v_center, cam->v_up, cam->gfx_data.m_view);
}
