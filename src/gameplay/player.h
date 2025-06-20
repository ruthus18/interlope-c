#pragma once

#include "render/camera.h"
#include "physics.h"


typedef struct Player {
    Camera* camera;
    PhysicsObjectID physics_id;
    f32 camera_y_offset;
    f32 physics_y_offset;

    vec3 position;
    vec2 direction;
    vec3 velocity;
    f32 speed;

    vec2 v_input;
    vec3 v_movement;

    bool is_active;
    bool is_colliding;
    bool is_grounded;
    bool is_ceiled;
} Player;


void player_init(vec3 position, vec2 direction);
void player_destroy();
void player_print();

void player_set_active(bool);
void player_set_colliding(bool);

void player_update_physics();
void player_update();
