/* cgm.h - Computer Graphics Math */
#pragma once
#include <cglm/cglm.h>

#include "./types.h"


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define radian(x) (x / (180.0 / GLM_PI))


void cgm_persp_mat(f32 fov, mat4 dest);
void cgm_view_mat(vec3 pos, vec3 v_front, mat4 dest);
void cgm_model_mat(vec3 pos, vec3 rot, vec3 sc, mat4 dest);

void cgm_rotation_mat(vec3 rot, mat4 dest);
void cgm_front_vec(f64 yaw, f64 pitch, vec3 dest);
