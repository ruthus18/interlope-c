#pragma once
#include <cglm/cglm.h>
#include <ode/ode.h>

#include "core/types.h"

#define PHYSICS_GRAVITY -9.81


typedef u32 PhysicsObjectID;

typedef enum {
    PHYSICS_BODY_BOX,
    PHYSICS_BODY_CAPSULE,
} PhysicsBodyType;

typedef struct PhysicsObject {
    PhysicsObjectID id;
    PhysicsBodyType type;
    dBodyID body;
    dGeomID geom;
    bool in_use;
} PhysicsObject;

/* ------------------------------------------------------------------------- */

void px_init();
void px_destroy();
void px_update();

PhysicsObject* px_static_create(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size
);
PhysicsObject* px_rigid_create(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size, f32 mass
);
PhysicsObject* px_kinematic_create(
    PhysicsBodyType type, vec3 pos, vec3 rot, vec3 size
);
bool px_remove_object(PhysicsObjectID id);

void px_static_get_position(PhysicsObject* obj, vec3 dest);
void px_static_set_position(PhysicsObject* obj, vec3 pos);

void px_rigid_get_position(PhysicsObject* obj, vec3 dest);
void px_rigid_set_position(PhysicsObject* obj, vec3 pos);
void px_rigid_get_rotation(PhysicsObject* obj, vec3 dest);

void px_kinematic_get_position(PhysicsObject* obj, vec3 dest);
void px_kinematic_set_position(PhysicsObject* obj, vec3 pos);
void px_kinematic_get_rotation(PhysicsObject* obj, vec3 dest);

dSpaceID px_get_space();

// ---------------

PhysicsObject* px_get_object(PhysicsObjectID id);
