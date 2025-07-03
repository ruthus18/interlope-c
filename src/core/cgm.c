#include <cglm/cglm.h>

#include "cgm.h"
#include "core/config.h"
#include "core/types.h"


void cgm_persp_mat(f32 fov, mat4 dest) {
    f64 aspect = (f64)Config.WINDOW_WIDTH / (f64)Config.WINDOW_HEIGHT;
    glm_perspective(radian(fov), aspect, 0.01, 1000.0, dest);
}


void cgm_view_mat(vec3 pos, vec3 v_front, mat4 dest) {
    vec3 v_center;
    glm_vec3_add(pos, v_front, v_center);
    glm_lookat(pos, v_center, V_UP_Y, dest);
}


void cgm_model_mat(vec3 pos, vec3 rot, vec3 sc, mat4 dest) {
    glm_mat4_identity(dest);

    if (pos != NULL) {
        glm_translate(dest, pos);
    }
    if (rot != NULL) {
        mat4 m_rot;
        cgm_rotation_mat(rot, m_rot);
        glm_mat4_mul(dest, m_rot, dest);
    }
    if (sc != NULL) {
        glm_scale(dest, sc);
    }
}


void cgm_rotation_mat(vec3 rot, mat4 dest) {
    mat4 m_rot_x, m_rot_y, m_rot_z;

    glm_mat4_identity(m_rot_x);
    glm_mat4_identity(m_rot_y);
    glm_mat4_identity(m_rot_z);

    glm_rotate(m_rot_x, radian(rot[0]), V_UP_X);
    glm_rotate(m_rot_y, radian(rot[1]), V_UP_Y);
    glm_rotate(m_rot_z, radian(rot[2]), V_UP_Z);

    glm_mat4_mul(m_rot_x, m_rot_y, dest);
    glm_mat4_mul(dest, m_rot_z, dest);
}


void cgm_front_vec(f64 yaw, f64 pitch, vec3 dest) {
    glm_vec3_copy(
        (vec3){
            cos(radian(yaw)) * cos(radian(pitch)),
            sin(radian(pitch)),
            sin(radian(yaw)) * cos(radian(pitch))
        },
        dest
    );
    glm_normalize(dest);
}
