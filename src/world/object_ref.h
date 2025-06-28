#pragma once

#include "object.h"

#include "core/types.h"
#include "database/schemas.h"
#include "physics/physics.h"


typedef struct ObjectRef {
    u32 ref_id;
    Object* obj;

    vec3 position;
    vec3 rotation;
    vec3* node_positions;
    vec3* node_rotations;

    PhysicsObject** physics;
} ObjectRef;


ObjectRef* object_ref_new(ObjectRefInfo*);
void object_ref_create_physics(ObjectRef* self, PhysicsInfo** physics_infos);

void object_ref_free(ObjectRef*);

void object_ref_update(ObjectRef*);
void object_ref_draw(ObjectRef*);

// ObjectRef ID system
ObjectRef* object_ref_find_by_id(u32 ref_id);
u32 object_ref_get_next_id(void);
