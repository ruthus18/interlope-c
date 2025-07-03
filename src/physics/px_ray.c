#include <stdlib.h>

#include "px_ray.h"
#include "physics/px.h"

#include "core/log.h"


PxRay* px_ray_new(u32 len) {
    PxRay* ray = malloc(sizeof(PxRay));

    ray->geom = dCreateRay(px_get_space(), len);
    ray->target = NULL;
    return ray;
}

void px_ray_free(PxRay* ray) {
    dGeomDestroy(ray->geom);
    free(ray);
}

void px_ray_set(PxRay* ray, vec3 pos, vec3 dir) {
    dGeomRaySet(ray->geom, pos[0], -pos[2], pos[1], dir[0], -dir[2], dir[1]);
}
