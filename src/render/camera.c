#include <stdlib.h>
#include <cglm/cglm.h>

#include "camera.h"

#include "core/cgm.h"
#include "core/config.h"
#include "core/types.h"
#include "platform/time.h"

#define CAMERA_DEFAULT_FOV      75.0


static inline
void _update_persp_mat(Camera* cam) {
    cgm_persp_mat(CAMERA_DEFAULT_FOV, cam->m_persp);
}

static inline
void _update_view_mat(Camera* cam) {
    cgm_view_mat(cam->position, cam->v_front, cam->m_view);
}

Camera* camera_create(vec3 position, vec2 direction) {
    Camera* cam = malloc(sizeof(Camera));

    if (position)     glm_vec3_copy(position, cam->position);
    else              glm_vec3_zero(cam->position);

    if (direction)    glm_vec2_copy(direction, cam->direction);
    else              glm_vec2_zero(cam->direction);

    _update_persp_mat(cam);
    _update_view_mat(cam);

    return cam;
}

void camera_destroy(Camera* cam) {
    free(cam);
}


void camera_set_position(Camera* cam, vec3 pos) {
    glm_vec3_copy(pos, cam->position);
    _update_view_mat(cam);
}

void camera_set_rotation(Camera* cam, vec2 direction) {
    glm_vec2_copy(direction, cam->direction);
    
    cgm_front_vec(cam->direction[0], cam->direction[1], cam->v_front);
    _update_view_mat(cam);
}
