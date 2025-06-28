#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <ode/ode.h>
#include <ode/threading_impl.h>
#include <cglm/cglm.h>

#include "physics.h"

#include "core/config.h"
#include "core/types.h"
#include "core/log.h"


// Documentation: https://ode.org/wiki/index.php/Manual


// TODO: Dynamic timestep
static const dReal TIMESTEP = (1.0 / PHYSICS_MAX_RATE);

#define MAX_PHYSICS_OBJECTS 128
#define INVALID_PHYSICS_ID  0


static struct PhysicsStorage {
    dWorldID world;
    dSpaceID space;
    dJointGroupID contact_group;
    PhysicsObject objects[MAX_PHYSICS_OBJECTS];
    PhysicsObjectID next_id;
} self;


static dGeomID ground_geom = NULL;


static
void custom_message_handler(int num, const char *msg, va_list args) {
    // skip linear complementarity problem
    if (num == 3)  return;

    char fmt_msg[256];
    vsprintf(fmt_msg, msg, args);
    log_info("ODE Message %i: %s", num, fmt_msg);
}


void px_init() {
    dSetMessageHandler(custom_message_handler);
    dInitODE();

    self.world = dWorldCreate();
    self.space = dHashSpaceCreate(0);
    self.contact_group = dJointGroupCreate(0);
    self.next_id = 1;

    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        self.objects[i].in_use = false;
        self.objects[i].id = INVALID_PHYSICS_ID;
        self.objects[i].body = NULL;
        self.objects[i].geom = NULL;
    }

    dWorldSetGravity(self.world, 0.0, 0.0, (dReal)PHYSICS_GRAVITY);
    
    // Increase solver iterations for better stability with high mass objects
    dWorldSetQuickStepNumIterations(self.world, 100);
    
    // Set global physics parameters for stability
    dWorldSetERP(self.world, 0.2);  // Error reduction parameter
    dWorldSetCFM(self.world, 1e-5); // Constraint force mixing
}

PhysicsObject* px_get_object(PhysicsObjectID id) {
    if (id == INVALID_PHYSICS_ID) return NULL;
    
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (self.objects[i].in_use && self.objects[i].id == id) {
            return &self.objects[i];
        }
    }
    return NULL;
}

static
PhysicsObject* find_empty_slot() {
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (!self.objects[i].in_use) {
            return &self.objects[i];
        }
    }
    return NULL;
}


void px_destroy() {
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (self.objects[i].in_use) {
            if (self.objects[i].geom) {
                dGeomDestroy(self.objects[i].geom);
                self.objects[i].geom = NULL;
            }
            
            if (self.objects[i].body) {
                dBodyDisable(self.objects[i].body);
                dBodySetData(self.objects[i].body, NULL);
                dBodyDestroy(self.objects[i].body);
                self.objects[i].body = NULL;
            }
        }
    }
    
    if (ground_geom) {
        dGeomDestroy(ground_geom);
        ground_geom = NULL;
    }
    
    dJointGroupDestroy(self.contact_group);
    dSpaceDestroy(self.space);
    dWorldDestroy(self.world);
    
    dCloseODE();
}


PhysicsObject* px_static_create(
    PhysicsBodyType type,
    vec3 pos,
    vec3 rot,
    vec3 size
) {
    PhysicsObject* obj = find_empty_slot();
    if (!obj) {
        log_info("Physics error: Maximum number of physics objects reached (%d)", MAX_PHYSICS_OBJECTS);
        return INVALID_PHYSICS_ID;
    }

    if (type == PHYSICS_BODY_BOX) {
        obj->geom = dCreateBox(self.space, size[0], size[2], size[1]);
    }
    else if (type == PHYSICS_BODY_CAPSULE) {
        obj->geom = dCreateCapsule(self.space, size[0], size[1]);
    }

    dGeomSetPosition(obj->geom, pos[0], -pos[2], pos[1]);
    dMatrix3 R;
    dRFromEulerAngles(
        R,
        rot[0] / (180 / M_PI),
        rot[2] / (180 / M_PI),
        rot[1] / (180 / M_PI)
    );
    dGeomSetRotation(obj->geom, R);

    obj->id = self.next_id++;
    obj->type = type;
    obj->in_use = true;

    return obj;
}


PhysicsObject* px_rigid_create(
    PhysicsBodyType type,
    vec3 pos,
    vec3 rot,
    vec3 size,
    f32 mass
) {
    // Find an empty slot in the objects array
    PhysicsObject* obj = find_empty_slot();
    if (!obj) {
        log_info("Physics error: Maximum number of physics objects reached (%d)", MAX_PHYSICS_OBJECTS);
        return INVALID_PHYSICS_ID;
    }
    
    // Create the body
    obj->body = dBodyCreate(self.world);
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
    
    // Set rotation
    dMatrix3 R;
    dRFromEulerAngles(
        R,
        rot[0] / (180 / M_PI),
        rot[2] / (180 / M_PI),
        rot[1] / (180 / M_PI)
    );
    dBodySetRotation(obj->body, R);
    
    // Set mass and type-specific properties
    dMass mass_;
    
    switch (type) {
        case PHYSICS_BODY_BOX:
            dMassSetBoxTotal(&mass_, mass, size[0], size[2], size[1]);
            obj->geom = dCreateBox(self.space, size[0], size[2], size[1]);
            break;

        case PHYSICS_BODY_CAPSULE:
            dMassSetCapsuleTotal(&mass_, mass, 3, size[0], size[1]);
            obj->geom = dCreateCapsule(self.space, size[0], size[1]);
            break;

        default:
            log_info("Physics error: Unsupported body type %d", type);
            dBodyDestroy(obj->body);
            return INVALID_PHYSICS_ID;
    }
    
    dBodySetMass(obj->body, &mass_);
    dGeomSetBody(obj->geom, obj->body);

    obj->id = self.next_id++;
    obj->type = type;
    obj->in_use = true;
    
    return obj;
}


PhysicsObject* px_kinematic_create(
    PhysicsBodyType type,
    vec3 pos,
    vec3 rot,
    vec3 size
) {
    PhysicsObject* obj = find_empty_slot();
    if (!obj)
        log_exit(
            "Physics error: Maximum number of physics objects reached (%d)",
            MAX_PHYSICS_OBJECTS
        );
    
    // Create the body
    obj->body = dBodyCreate(self.world);
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
    
    // Set rotation
    dMatrix3 R;
    dRFromEulerAngles(
        R,
        rot[0] / (180 / M_PI),
        rot[2] / (180 / M_PI),
        rot[1] / (180 / M_PI)
    );
    dBodySetRotation(obj->body, R);
    
    // Make it kinematic (no gravity, infinite mass)
    dBodySetKinematic(obj->body);
    
    // Create geometry based on type
    if (type == PHYSICS_BODY_BOX) {
        obj->geom = dCreateBox(self.space, size[0], size[2], size[1]);
    }
    else if (type == PHYSICS_BODY_CAPSULE) {
        obj->geom = dCreateCapsule(self.space, size[0], size[1] - size[0]);
    }
    else {
        log_error("Physics error: Unsupported body type %d", type);
        dBodyDestroy(obj->body);
        return NULL;
    }
    dGeomSetBody(obj->geom, obj->body);

    obj->id = self.next_id++;
    obj->type = type;
    obj->in_use = true;
    
    return obj;
}


bool px_remove_object(PhysicsObjectID id) {
    PhysicsObject* obj = px_get_object(id);
    if (!obj) {
        return false;
    }
    
    if (obj->geom) {
        dGeomDestroy(obj->geom);
    }
    
    if (obj->body) {
        dBodyDisable(obj->body);
        dBodySetData(obj->body, NULL);
        dBodyDestroy(obj->body);
    }
    
    obj->in_use = false;
    obj->id = INVALID_PHYSICS_ID;
    obj->body = NULL;
    obj->geom = NULL;
    
    return true;
}

/* ------------------------------------------------------------------------- */

dSpaceID px_get_space() {
    return self.space;
}

/* ------ px_static ------ */
/* ------------------------------------------------------------------------- */

void px_static_get_position(PhysicsObject* obj, vec3 dest) {
    const dReal* pos = dGeomGetPosition(obj->geom);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}

void px_static_set_position(PhysicsObject* obj, vec3 pos) {
    dGeomSetPosition(obj->geom, pos[0], -pos[2], pos[1]);
}

/* ------ px_rigid ------ */
/* ------------------------------------------------------------------------- */

void px_rigid_get_position(PhysicsObject* obj, vec3 dest) {
    const dReal* pos = dBodyGetPosition(obj->body);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}

void px_rigid_set_position(PhysicsObject* obj, vec3 pos) {
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
}

void px_rigid_get_rotation(PhysicsObject* obj, vec3 dest) {
    const dReal* R = dBodyGetRotation(obj->body);
    
    mat3 rot_ = {R[0], R[1], R[2], R[4], R[5], R[6], R[8], R[9], R[10]};
    
    float rot_x = atan2(rot_[2][1], rot_[2][2]) * (180.0 / M_PI);
    float rot_y = atan2(-rot_[2][0], sqrt(pow(rot_[2][1], 2) + pow(rot_[2][2], 2))) * (180.0 / M_PI);
    float rot_z = atan2(rot_[1][0], rot_[0][0]) * (180.0 / M_PI);
    
    dest[0] = rot_x;
    dest[1] = rot_z;
    dest[2] = -rot_y;
}

/* ------ px_kinematic ------ */
/* ------------------------------------------------------------------------- */

void px_kinematic_get_position(PhysicsObject* obj, vec3 dest) {
    const dReal* pos = dBodyGetPosition(obj->body);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}
void px_kinematic_set_position(PhysicsObject* obj, vec3 pos) {
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
}

void px_kinematic_get_rotation(PhysicsObject* obj, vec3 dest) {
    const dReal* R = dBodyGetRotation(obj->body);
    
    mat3 rot_ = {R[0], R[1], R[2], R[4], R[5], R[6], R[8], R[9], R[10]};
    
    float rot_x = atan2(rot_[2][1], rot_[2][2]) * (180.0 / M_PI);
    float rot_y = atan2(-rot_[2][0], sqrt(pow(rot_[2][1], 2) + pow(rot_[2][2], 2))) * (180.0 / M_PI);
    float rot_z = atan2(rot_[1][0], rot_[0][0]) * (180.0 / M_PI);
    
    dest[0] = rot_x;
    dest[1] = rot_z;
    dest[2] = -rot_y;
}

/* ------------------------------------------------------------------------- */


static const int MAX_CONTACTS = 8;


// For kinematic bodies, create contact only for the kinematic body
// (player gets blocked, cube doesn't move)
static inline
void _kinematic_contact_surface_params(dContact contact) {
    contact.surface.mode = dContactBounce | dContactSoftERP | dContactSoftCFM;
    contact.surface.mu = 0.0;  // No friction
    contact.surface.mu2 = 0;
    contact.surface.bounce = 0.001;  // Almost no bounce
    contact.surface.bounce_vel = 0.001;
    contact.surface.soft_erp = 0.15;  // Strong enough to block player
    contact.surface.soft_cfm = 0.02;
}

// Normal contact for rigid-rigid collisions
static inline
void _rigid_contact_surface_params(dContact contact) {
    contact.surface.mode = dContactBounce | dContactSoftERP | dContactSoftCFM;
    contact.surface.mu = dInfinity;
    contact.surface.mu2 = 0;
    contact.surface.bounce = 0.1;
    contact.surface.bounce_vel = 0.1;
    contact.surface.soft_erp = 0.2;
    contact.surface.soft_cfm = 0.001;
}


static
void collision_callback(void* data, dGeomID geom1, dGeomID geom2) {
    dBodyID body1 = dGeomGetBody(geom1);
    dBodyID body2 = dGeomGetBody(geom2);
    
    /* --- Collision Validation --- */
    // Skip self-collisions for the same body
    if (body1 && body2 && body1 == body2) return;
    
    // Skip collisions between two static objects (both without bodies)
    if (!body1 && !body2) return;

    bool is_kinematic = false;
    if (body1 && dBodyIsKinematic(body1))  is_kinematic = true;
    if (body2 && dBodyIsKinematic(body2))  is_kinematic = true;

    /* --- Collision Handling --- */
    dContact contact[MAX_CONTACTS];
    i32 n = dCollide(geom1, geom2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact));

    for (int i = 0; i < n; i++) {        
        if (is_kinematic) {
            _kinematic_contact_surface_params(contact[i]);
        }
        else {
            _rigid_contact_surface_params(contact[i]);
            dJointID c = dJointCreateContact(self.world, self.contact_group, &contact[i]);
            dJointAttach(c, dGeomGetBody(geom1), dGeomGetBody(geom2));
        }
    }
}

void px_update() {
    dSpaceCollide(self.space, 0, collision_callback);
    dWorldQuickStep(self.world, TIMESTEP);
    dJointGroupEmpty(self.contact_group);
}
