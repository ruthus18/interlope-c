#pragma once
#include <cglm/cglm.h>
#include <ode/ode.h>

#include "types.h"


typedef u32 PhysicsObjectID;

typedef enum {
    PHYSICS_BODY_BOX,
    // Can be extended with SPHERE, CYLINDER, CAPSULE, etc.
} PhysicsBodyType;


void physics_init();
void physics_destroy();

void physics_create_ground();

PhysicsObjectID physics_create_object(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size, f32 mass
);
bool physics_remove_object(PhysicsObjectID id);

bool physics_get_object_position(PhysicsObjectID id, vec3 dest);
bool physics_get_object_rotation(PhysicsObjectID id, vec3 dest);
bool physics_apply_force(PhysicsObjectID id, vec3 force);

void physics_update();


// FIXME Legacy functions for backward compatibility
void physics_create_cube(vec3 pos, vec3 rot, vec3 size, f32 mass);
void physics_get_cube_position(vec3 dest);
void physics_get_cube_rotation(vec3 dest);
