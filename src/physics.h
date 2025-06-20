#pragma once
#include <cglm/cglm.h>
#include <ode/ode.h>

#include "./core/types.h"

#define PHYSICS_GRAVITY -9.81


typedef u32 PhysicsObjectID;

typedef enum {
    PHYSICS_BODY_BOX,
    PHYSICS_BODY_CAPSULE,
} PhysicsBodyType;


void physics_init();
void physics_destroy();
void physics_update();

PhysicsObjectID physics_create_static_object(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size
);
PhysicsObjectID physics_create_rigid_object(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size, f32 mass
);
PhysicsObjectID physics_create_kinematic_object(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size
);
bool physics_remove_object(PhysicsObjectID id);

bool physics_get_object_position(PhysicsObjectID id, vec3 dest);
bool physics_get_object_rotation(PhysicsObjectID id, vec3 dest);

void physics_set_object_position(PhysicsObjectID id, vec3 new_pos);
void physics_set_kinematic_position(PhysicsObjectID id, vec3 pos);

// ---------------
void player_physics_set(PhysicsObjectID id);
bool player_physics_is_grounded();
bool player_physics_is_ceiled();
void player_physics_transform(vec3 transform);
