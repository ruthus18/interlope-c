#pragma once

#include "./object.h"

#include "../core/types.h"
#include "../database/schemas.h"
#include "../physics.h"


typedef struct ObjectRef {
    // u32 ref_id;
    Object* obj;

    vec3 position;
    vec3 rotation;
    vec3* node_positions;
    vec3* node_rotations;

    PhysicsObjectID physics_id;
} ObjectRef;


ObjectRef* object_ref_create_from_info(ObjectRefInfo*);
void object_ref_create_physics(ObjectRef* self, PhysicsInfo* physics_info);

void object_ref_destroy(ObjectRef*);

void object_ref_update(ObjectRef*);
void object_ref_draw(ObjectRef*);
