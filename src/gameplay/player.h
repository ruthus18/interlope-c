#pragma once

#include "graphics/camera.h"
#include "physics/px.h"
#include "world/object_ref.h"


typedef struct Player {
    Camera* camera;
    f32 camera_y_offset;

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

    ObjectRef* interactor_oref;
} Player;


void player_init(vec3 position, vec2 direction);
void player_destroy();
void player_print();

void player_set_active(bool);
void player_set_colliding(bool);

void player_update_physics();
void player_update();
