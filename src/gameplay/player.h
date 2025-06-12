#pragma once

#include "../render/camera.h"
#include "../physics.h"


typedef struct Player {
    Camera* camera;
    PhysicsObjectID physics_id;
    vec3 pos;
    vec2 rot;
    f32 velocity_y;

    bool is_active;
    bool is_grounded;
    bool gravity_enabled;
} Player;


void player_init(vec3 pos, f64 pitch, f64 yaw);
void player_destroy();

void player_set_is_active(bool value);
void player_set_gravity_enabled(bool value);

void player_update();
