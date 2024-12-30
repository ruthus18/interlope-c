#include <cglm/cglm.h>

#include "cgm.h"
#include "config.h"


vec3 v_up__x = {1.0, 0.0, 0.0};
vec3 v_up__y = {0.0, 1.0, 0.0};
vec3 v_up__z = {0.0, 1.0, 1.0};


void cgm_persp_mat(float fov, mat4 dest) {
    double aspect = WINDOW_WIDTH / WINDOW_HEIGHT;

    glm_perspective(fov, aspect, 0.1, 1000.0, dest);
}


void cgm_view_mat(vec3 pos, vec3 v_front, mat4 dest) {
    vec3 v_center;

    glm_vec3_add(pos, v_front, v_center);
    glm_lookat(pos, v_center, v_up__y, dest);
}


void cgm_model_mat(vec3 pos, vec3 rot, vec3 sc, mat4 dest) {
    vec3 v_view = {-pos[0], -pos[1], -pos[2]};
    mat4 m_rot;
    cgm_rotation_mat(rot, m_rot);

    glm_mat4_identity(dest);
    glm_translate(dest, v_view);
    glm_mat4_mul(dest, m_rot, dest);
    glm_scale(dest, sc);
}


void cgm_rotation_mat(vec3 rot, mat4 dest) {
    mat4 m_rot__x, m_rot__y, m_rot__z;

    glm_rotate(m_rot__x, radian(rot[0]), v_up__x);
    glm_rotate(m_rot__y, radian(rot[1]), v_up__y);
    glm_rotate(m_rot__z, radian(rot[2]), v_up__z);

    glm_mat4_mul(m_rot__x, m_rot__y, dest);
    glm_mat4_mul(dest, m_rot__z, dest);
}


void cgm_front_vec(double yaw, double pitch, vec3 dest) {
    glm_vec3_copy(
        (vec3){
            cos(radian(yaw) * cos(radian(pitch))),
            sin(radian(pitch)),
            sin(radian(yaw)) * cos(radian(pitch))
        },
        dest
    );
    glm_normalize(dest);
}
