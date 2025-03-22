#include <ode/ode.h>
#include <cglm/cglm.h>

#include "physics.h"
#include "cglm/vec3.h"
#include "ode/collision.h"
#include "ode/common.h"
#include "ode/objects.h"
#include "types.h"


// Documentation: https://ode.org/wiki/index.php/Manual


static dWorldID world;
static dSpaceID space;
static dJointGroupID contact_group;

static const dReal GRAVITY = -9.81;
static const dReal TIMESTEP = (1.0 / 60.0);

static dGeomID ground_geom;

static dBodyID cube_body;
static dGeomID cube_geom;
static const dReal CUBE_SIZE = 0.5; 


void physics_init() {
    dInitODE();

    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    contact_group = dJointGroupCreate(0);

    dWorldSetGravity(world, 0.0, 0.0, GRAVITY);
}


void physics_destroy() {
    dGeomDestroy(cube_geom);
    // FIXME cause segfult, need to investigate
    // dBodyDestroy(cube_body);
    
    dJointGroupDestroy(contact_group);
    dSpaceDestroy(space);
    dWorldDestroy(world);
    
    dCloseODE();
}


void physics_create_ground() {
    ground_geom = dCreatePlane(space, 0, 0, 1, 0);
}


void physics_create_cube(vec3 pos, vec3 size, f32 mass) {
    cube_body = dBodyCreate(world);
    dBodySetPosition(cube_body, pos[0], -pos[2], pos[1]);

    dMass mass_;
    dMassSetBoxTotal(&mass_, mass, size[0], size[2], size[1]);
    dBodySetMass(cube_body, &mass_);

    cube_geom = dCreateBox(space, size[0], size[2], size[1]);
    dGeomSetBody(cube_geom, cube_body);
}


void physics_get_cube_position(vec3 dest) {
    const dReal* pos = dBodyGetPosition(cube_body);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}


static
void collision_callback(void* data, dGeomID geom1, dGeomID geom2) {
    dContact contacts[4];
    int n = dCollide(geom1, geom2, 4, &contacts[0].geom, sizeof(dContact));

    for (int i = 0; i < n; i++) {
        contacts[i].surface.mode = dContactBounce | dContactSoftCFM;
        contacts[i].surface.bounce = 0.1;
        contacts[i].surface.bounce_vel = 0.1;
        contacts[i].surface.soft_cfm = 0.01;

        dJointID contact = dJointCreateContact(world, contact_group, &contacts[i]);
        dJointAttach(contact, dGeomGetBody(geom1), dGeomGetBody(geom2));
    }
}


void physics_update() {
    dSpaceCollide(space, 0, collision_callback);
    dWorldStep(world, TIMESTEP);
    dJointGroupEmpty(contact_group);
}
