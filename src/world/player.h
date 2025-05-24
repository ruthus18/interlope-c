#pragma once

#include "../render/camera.h"
#include "../physics.h"


typedef struct Player {
    Camera* camera;
    PhysicsObjectID physics_id;
    vec3 pos;
    vec2 rot;
} Player;


void player_init(vec3 pos, f64 pitch, f64 yaw);
void player_destroy();

Camera* player_get_camera();

void player_update();
