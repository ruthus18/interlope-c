#include <math.h>

#include "px_player.h"
#include "physics/px.h"
#include "physics/px_object.h"
#include "physics/px_ray.h"

#include "core/log.h"


static struct PxPlayer {
    dSpaceID space;
    PxObject* obj;
    PxRay* interact_ray;

    bool is_grounded;
    bool is_ceiled;
    bool near_wall;

    f32 y_offset;
} self = {};


void px_player_init(vec3 pos, vec3 rot, f32 width, f32 height) {
    self.y_offset = height / 2;
    
    self.space = px_get_space();
    self.obj = px_kinematic_create(
        PXBODY_CAPSULE,
        (vec3){pos[0], pos[1] + self.y_offset, pos[2]},
        rot,
        (vec3){width / 2, height, 0.0}
    );
    self.interact_ray = px_ray_new(2.0);
}

void px_player_destroy() {
    px_ray_free(self.interact_ray);
}

bool px_player_get_grounded()  { return self.is_grounded; }
bool px_player_get_ceiled()    { return self.is_ceiled; }
cvector(PxRayTarget) px_player_get_interact_target() { return self.interact_ray->targets; }

void px_player_set_position(vec3 pos) {
    px_kinematic_set_position(
        self.obj,
        (vec3){pos[0], pos[1] + self.y_offset, pos[2]}
    );
}

void px_player_set_interact_ray(vec3 pos, vec3 dir) {
    px_ray_set(self.interact_ray, pos, dir);
}

/* ------------------------------------------------------------------------- */

static
void _wall_collision_callback(void* data, dGeomID geom1, dGeomID geom2) {
    dGeomID player_geom = self.obj->geom;

    if (geom1 != player_geom && geom2 != player_geom)  return;
    
    dGeomID wall_geom = (geom1 == player_geom ? geom2 : geom1);

    if (wall_geom == self.interact_ray->geom)  return;

    vec3 player_pos;
    px_kinematic_get_position(self.obj, player_pos);

    dContactGeom contact;
    int n = dCollide(player_geom, wall_geom, 1, &contact, sizeof(dContactGeom));
    if (n > 0) {
        f32 normal_y = contact.normal[2];
        f32 pos_y = contact.pos[2];
        f32 player_pos_y = player_pos[1];

        bool contact_near_floor = (pos_y - (player_pos_y - 1.7 / 2)) > 0.1;

        if (normal_y > -0.1 && normal_y < 0.1 && contact_near_floor) {
            self.near_wall = true;
        }
    }
}

static inline
bool _check_wall_collision(vec3 orig_pos, vec3 test_pos) {
    // Move body to test position
    dBodySetPosition(self.obj->body, test_pos[0], -test_pos[2], test_pos[1]);

    // Check for collisions
    self.near_wall = false;
    dSpaceCollide(self.space, 0, _wall_collision_callback);
    
    // Restore original positions
    dBodySetPosition(self.obj->body, orig_pos[0], -orig_pos[2], orig_pos[1]);
    return self.near_wall;
}

void px_player_translate(vec3 dest_pos) {
    vec3 orig_pos, x_test_pos, z_test_pos;

    px_kinematic_get_position(self.obj, orig_pos);
    glm_vec3_copy(orig_pos, x_test_pos);
    glm_vec3_copy(orig_pos, z_test_pos);
    x_test_pos[0] += dest_pos[0];
    z_test_pos[2] += dest_pos[2];

    if (_check_wall_collision(orig_pos, x_test_pos))
        dest_pos[0] = 0.0f;
    
    if (_check_wall_collision(orig_pos, z_test_pos))
        dest_pos[2] = 0.0f;
}

/* ------------------------------------------------------------------------- */

static inline
void on_px_player_interact_ray_collision(dGeomID geom) {
    PxObject* obj = px_get_object_by_geom(geom);
    if (!obj) return;
    
    vec3 obj_pos;
    vec3 ray_pos;
    vec3 v_dist;

    if (obj->body)  px_rigid_get_position(obj, obj_pos);
    else            px_static_get_position(obj, obj_pos);
    
    px_ray_get(self.interact_ray, ray_pos, NULL);

    glm_vec3_sub(ray_pos, obj_pos, v_dist);
    f64 dist = cbrt(fabs(v_dist[0] * v_dist[1] * v_dist[2]));

    PxRayTarget ray_target;
    ray_target.obj = obj;
    ray_target.dist = dist;
    px_ray_add_target(self.interact_ray, ray_target);
}


// TODO unify with collision logic above

static
void on_px_player_collision(void* _, dGeomID geom1, dGeomID geom2) {
    dGeomID player_geom = self.obj->geom;
    dGeomID interact_geom = self.interact_ray->geom;
    
    if (geom1 == interact_geom || geom2 == interact_geom) {
        dGeomID other_geom = (geom1 == interact_geom ? geom2 : geom1);
        if (other_geom == player_geom)  return;

        on_px_player_interact_ray_collision(other_geom);
    }

    if (geom1 != player_geom && geom2 != player_geom)  return;
    
    dGeomID other_geom = (geom1 == player_geom ? geom2 : geom1);

    dContactGeom contact;
    int n = dCollide(player_geom, other_geom, 1, &contact, sizeof(dContactGeom));
    if (n > 0) {
        f32 normal_y = contact.normal[2];
        f32 pos_y = contact.pos[2];
        
        if (contact.normal[2] > 0.7) {
            self.is_grounded = true;
        }
        else if (contact.normal[2] < -0.7) {
            self.is_ceiled = true;
        }
    }
}

void px_player_update() {
    self.is_grounded = false;
    self.is_ceiled = false;
    px_ray_clear_targets(self.interact_ray);

    dSpaceCollide(self.space, 0, on_px_player_collision);
}
