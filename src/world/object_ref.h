#pragma once

#include "object.h"

#include "core/types.h"
#include "database/schemas.h"
#include "physics/px.h"


typedef struct ObjectRef {
    u32 ref_id;
    Object* obj;

    vec3 position;
    vec3 rotation;
    vec3* node_positions;
    vec3* node_rotations;

    PxObject** physics;
} ObjectRef;


ObjectRef* object_ref_new(ObjectRefInfo*);

void object_ref_free(ObjectRef*);

void object_ref_update(ObjectRef*);
void object_ref_draw(ObjectRef*);
