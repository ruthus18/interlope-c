/* cgm.h - Computer Graphics Math */
#pragma once
#include <cglm/cglm.h>


#define radian(x) (x / (180.0 / GLM_PI))
#define sizeof_vec3 (sizeof(float) * 3)

void cgm_persp_mat(float fov, mat4 dest);
void cgm_view_mat(vec3 pos, vec3 v_front, mat4 dest);
void cgm_model_mat(vec3 pos, vec3 rot, vec3 sc, mat4 dest);
void cgm_rotation_mat(vec3 rot, mat4 dest);
void cgm_front_vec(double yaw, double pitch, vec3 dest);
