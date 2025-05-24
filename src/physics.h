#pragma once
#include <cglm/cglm.h>
#include <ode/ode.h>

#include "./core/types.h"


typedef u32 PhysicsObjectID;

typedef enum {
    PHYSICS_BODY_BOX,
    PHYSICS_BODY_CAPSULE,
} PhysicsBodyType;


void physics_init();
void physics_destroy();

PhysicsObjectID physics_create_static_object(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size
);
PhysicsObjectID physics_create_rigid_object(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size, f32 mass
);
bool physics_remove_object(PhysicsObjectID id);

bool physics_get_object_position(PhysicsObjectID id, vec3 dest);
void physics_set_object_position(PhysicsObjectID id, vec3 new_pos);

bool physics_get_object_rotation(PhysicsObjectID id, vec3 dest);
bool physics_apply_force(PhysicsObjectID id, vec3 force);

void physics_update();
