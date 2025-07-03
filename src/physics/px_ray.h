#pragma once
#include <ode/ode.h>
#include "cglm/cglm.h"

#include "physics/px_object.h"

#include "core/types.h"


typedef struct {
    dGeomID geom;
    PxObject* target;
} PxRay;

PxRay* px_ray_new(u32 len);
void px_ray_free(PxRay*);
void px_ray_set(PxRay* ray, vec3 pos, vec3 dir);
