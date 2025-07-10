#pragma once
#include <ode/ode.h>
#include "cglm/cglm.h"
#include <cvector.h>
#include <cvector_utils.h>

#include "physics/px_object.h"

#include "core/types.h"


typedef struct {
    PxObject* obj;
    f64 dist;
} PxRayTarget;

typedef struct {
    dGeomID geom;
    cvector(PxRayTarget) targets;
} PxRay;

PxRay* px_ray_new(u32 len);
void px_ray_free(PxRay*);

void px_ray_set(PxRay*, vec3 pos, vec3 dir);
void px_ray_get(PxRay*, vec3 dest_pos, vec3 dest_dir);
void px_ray_add_target(PxRay*, PxRayTarget target);
void px_ray_clear_targets(PxRay*);