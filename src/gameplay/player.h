#pragma once

#include "render/camera.h"
#include "physics.h"


typedef struct Player {
    Camera* camera;
    PhysicsObjectID physics_id;
    vec3 pos;
    vec2 rot;
    f32 velocity_y;

    bool is_active;
    bool is_grounded;
    bool is_ceiled;
    bool is_clipping;
} Player;


void player_init(vec3 pos, vec2 rot);
void player_destroy();
void player_print();

void player_set_is_active(bool value);
void player_set_clipping(bool value);

void player_update();
