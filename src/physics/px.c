#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <ode/ode.h>
#include <ode/threading_impl.h>
#include <cglm/cglm.h>

#include "px.h"
#include "physics/px_object.h"

#include "core/containers/map.h"
#include "core/config.h"
#include "core/types.h"
#include "core/log.h"


// Documentation: https://ode.org/wiki/index.php/Manual


static const dReal TIMESTEP = (1.0 / PHYSICS_MAX_RATE);


static struct PxStorage {
    dWorldID world;
    dSpaceID space;
    dJointGroupID contact_group;

    map(PxObject) objects;
    u32 next_id;
} self;


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
    
    self.objects = map_new(MHASH_INT);
    self.next_id = 1;

    dWorldSetGravity(self.world, 0.0, 0.0, (dReal)PHYSICS_GRAVITY);

    // Set global physics parameters for stability
    dWorldSetERP(self.world, 0.2);  // Error reduction parameter
    dWorldSetCFM(self.world, 1e-5); // Constraint force mixing
}

void px_destroy() {
    PxObject* obj;

    map_for_each(obj, self.objects) {
        px_object_free(obj);
    }
    map_free(self.objects);

    dJointGroupDestroy(self.contact_group);
    dSpaceDestroy(self.space);
    dWorldDestroy(self.world);
    
    dCloseODE();
}

void px_add_object(PxObject* obj) {
    map_set(self.objects, obj, (void*)(intptr_t)obj->id);
}

void px_delete_object(PxObject* obj) {
    if (!obj)  return;

    px_object_free(obj);
    map_remove(self.objects, (void*)(intptr_t)obj->id);
}

u32 px_next_id() { return self.next_id++; }
dWorldID px_get_world() { return self.world; }
dSpaceID px_get_space() { return self.space; }


/* ------------------------------------------------------------------------- */

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

static const int MAX_CONTACTS = 8;

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
