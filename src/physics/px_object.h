#pragma once
#include <ode/ode.h>
#include <cglm/cglm.h>

#include "core/types.h"

// typedef enum {
//     PXOBJ_STATIC,
//     PXOBJ_RIGID,
//     PX_OBJ_KINEMATIC,
// } PxObjectType;

typedef enum {
    PXBODY_BOX,
    PXBODY_CAPSULE,
} PxBodyType;

typedef struct {
    u32 id;
    PxBodyType type;
    dBodyID body;
    dGeomID geom;
} PxObject;

PxObject* px_static_create(PxBodyType type, vec3 pos, vec3 rot, vec3 size);
void px_static_get_position(PxObject* obj, vec3 dest);
void px_static_set_position(PxObject* obj, vec3 pos);
void px_static_get_rotation(PxObject* obj, vec3 dest);

PxObject* px_rigid_create(PxBodyType type, vec3 pos, vec3 rot, vec3 size, f32 mass);
void px_rigid_get_position(PxObject* obj, vec3 dest);
void px_rigid_set_position(PxObject* obj, vec3 pos);
void px_rigid_get_rotation(PxObject* obj, vec3 dest);

PxObject* px_kinematic_create(PxBodyType type, vec3 pos, vec3 rot, vec3 size);
void px_kinematic_get_position(PxObject* obj, vec3 dest);
void px_kinematic_set_position(PxObject* obj, vec3 pos);
void px_kinematic_get_rotation(PxObject* obj, vec3 dest);

void px_object_free(PxObject* obj);
