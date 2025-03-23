#include <stdio.h>
#include <math.h>
#include <ode/ode.h>
#include <cglm/cglm.h>

#include "physics.h"
#include "cglm/io.h"
#include "ode/contact.h"
#include "ode/objects.h"
#include "ode/rotation.h"
#include "types.h"
#include "log.h"


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


static
void custom_message_handler(int num, const char *msg, va_list args) {
    // skip linear complementarity problem
    if (num == 3)  return;

    char fmt_msg[256];
    vsprintf(fmt_msg, msg, args);
    log_info("ODE Message %i: %s", num, fmt_msg);
}


void physics_init() {
    dSetMessageHandler(custom_message_handler);
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


void physics_create_cube(vec3 pos, vec3 rot, vec3 size, f32 mass) {
    cube_body = dBodyCreate(world);
    dBodySetPosition(cube_body, pos[0], -pos[2], pos[1]);

    dMatrix3 R;
    dRFromEulerAngles(
        R,
        rot[0] / (180 / M_PI),
        rot[2] / (180 / M_PI),
        rot[1] / (180 / M_PI)
    );
    dBodySetRotation(cube_body, R);

    dMass mass_;
    dMassSetBoxTotal(&mass_, mass, size[0], size[2], size[1]);
    dBodySetMass(cube_body, &mass_);

    cube_geom = dCreateBox(space, size[0], size[2], size[1]);
    dGeomSetBody(cube_geom, cube_body);

    dBodySetAutoDisableFlag(cube_body, 1);
    dBodySetAutoDisableLinearThreshold(cube_body, 0.05);
}


void physics_get_cube_position(vec3 dest) {
    const dReal* pos = dBodyGetPosition(cube_body);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}


void physics_get_cube_rotation(vec3 dest) {
    const dReal* R = dBodyGetRotation(cube_body);
    
    mat3 rot_ = {R[0], R[1], R[2], R[4], R[5], R[6], R[8], R[9], R[10]};
    // glm_mat3_print(rot_, stdout);

    float rot_x = atan2(rot_[2][1], rot_[2][2]) * (180.0 / M_PI);
    float rot_y = atan2(-rot_[2][0], sqrt(pow(rot_[2][1], 2) + pow(rot_[2][2], 2))) * (180.0 / M_PI);
    float rot_z = atan2(rot_[1][0], rot_[0][0]) * (180.0 / M_PI);

    dest[0] = rot_x;
    dest[1] = rot_z;
    dest[2] = -rot_y;
}


static constexpr u16 MAX_CONTACTS = 4;

static
void collision_callback(void* data, dGeomID geom1, dGeomID geom2) {
    dContact contacts[MAX_CONTACTS];
    int n = dCollide(geom1, geom2, MAX_CONTACTS, &contacts[0].geom, sizeof(dContact));

    for (int i = 0; i < n; i++) {
        contacts[i].surface.mode = dContactBounce | dContactSoftERP | dContactSoftCFM;
        contacts[i].surface.bounce = 0.05;
        contacts[i].surface.bounce_vel = 0.1;
        contacts[i].surface.soft_erp = 0.8;
        contacts[i].surface.soft_cfm = 0.005;

        dJointID contact = dJointCreateContact(world, contact_group, &contacts[i]);
        dJointAttach(contact, dGeomGetBody(geom1), dGeomGetBody(geom2));
    }
}


void physics_update() {
    dSpaceCollide(space, 0, collision_callback);
    dWorldStep(world, TIMESTEP);
    dJointGroupEmpty(contact_group);
}
