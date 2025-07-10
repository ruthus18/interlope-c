#include <stdlib.h>

#include "px_ray.h"
#include "physics/px.h"

#include "core/log.h"


PxRay* px_ray_new(u32 len) {
    PxRay* ray = malloc(sizeof(PxRay));
    memset(ray, 0, sizeof(PxRay));

    ray->geom = dCreateRay(px_get_space(), len);
    cvector_reserve(ray->targets, 8);

    return ray;
}

void px_ray_free(PxRay* ray) {
    dGeomDestroy(ray->geom);
    cvector_free(ray->targets);

    free(ray);
}

void px_ray_set(PxRay* ray, vec3 pos, vec3 dir) {
    dGeomRaySet(ray->geom, pos[0], -pos[2], pos[1], dir[0], -dir[2], dir[1]);
}

void px_ray_get(PxRay* ray, vec3 dest_pos, vec3 dest_dir) {
    dReal pos[3];
    dReal dir[3];
    dGeomRayGet(ray->geom, pos, dir);

    if (dest_pos)   glm_vec3_copy(dest_pos, (vec3){pos[0], -pos[2], pos[1]});
    if (dest_dir)   glm_vec3_copy(dest_dir, (vec3){dir[0], -dir[2], dir[1]});
}

void px_ray_add_target(PxRay* ray, PxRayTarget target) {
    cvector_push_back(ray->targets, target);
}

void px_ray_clear_targets(PxRay* ray) {
    cvector_clear(ray->targets);
}
