#include <stdio.h>
#include <math.h>
#include <ode/ode.h>
#include <cglm/cglm.h>

#include "ode/collision.h"
#include "ode/contact.h"
#include "ode/mass.h"
#include "ode/objects.h"

#include "physics.h"
#include "./core/config.h"
#include "./core/types.h"
#include "./core/log.h"


// Documentation: https://ode.org/wiki/index.php/Manual


// TODO: Dynamic timestep
static const dReal TIMESTEP = (1.0 / WINDOW_MAX_FRAMERATE);

static constexpr i32 MAX_PHYSICS_OBJECTS = 1024;
static constexpr i32 INVALID_PHYSICS_ID = 0;


typedef struct PhysicsObject {
    PhysicsObjectID id;
    PhysicsBodyType type;
    dBodyID body;
    dGeomID geom;
    bool in_use;
} PhysicsObject;


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


void physics_init() {
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

static
PhysicsObject* find_physics_object(PhysicsObjectID id) {
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


void physics_destroy() {
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


PhysicsObjectID physics_create_static_object(
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

    return obj->id;
}


PhysicsObjectID physics_create_rigid_object(
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
        
        // Future types can be added here
        // case PHYSICS_BODY_SPHERE:
        //     dMassSetSphereTotal(&mass_, mass, size[0]);
        //     obj->geom = dCreateSphere(self.space, size[0]);
        //     break;
        
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
    
    return obj->id;
}


PhysicsObjectID physics_create_kinematic_object(
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
    switch (type) {
        case PHYSICS_BODY_BOX:
            obj->geom = dCreateBox(self.space, size[0], size[2], size[1]);
            break;

        case PHYSICS_BODY_CAPSULE:
            obj->geom = dCreateCapsule(self.space, size[0], size[1]);
            break;
        
        default:
            log_info("Physics error: Unsupported body type %d", type);
            dBodyDestroy(obj->body);
            return INVALID_PHYSICS_ID;
    }
    
    dGeomSetBody(obj->geom, obj->body);

    obj->id = self.next_id++;
    obj->type = type;
    obj->in_use = true;
    
    return obj->id;
}


bool physics_remove_object(PhysicsObjectID id) {
    PhysicsObject* obj = find_physics_object(id);
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
bool physics_get_object_position(PhysicsObjectID id, vec3 dest) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj || !obj->body) {
        return false;
    }

    const dReal* pos = dBodyGetPosition(obj->body);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
    return true;
}


void physics_set_object_position(PhysicsObjectID id, vec3 pos) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj) {
        log_error("Physics object not found: %i", id);
        return;
    }
    dGeomSetPosition(obj->geom, pos[0], -pos[2], pos[1]);
}


void physics_set_kinematic_position(PhysicsObjectID id, vec3 pos) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj || !obj->body) {
        log_error("Physics object not found or has no body: %i", id);
        return;
    }
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
}


bool physics_get_object_rotation(PhysicsObjectID id, vec3 dest) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj || !obj->body) {
        return false;
    }
    
    const dReal* R = dBodyGetRotation(obj->body);
    
    mat3 rot_ = {R[0], R[1], R[2], R[4], R[5], R[6], R[8], R[9], R[10]};
    
    float rot_x = atan2(rot_[2][1], rot_[2][2]) * (180.0 / M_PI);
    float rot_y = atan2(-rot_[2][0], sqrt(pow(rot_[2][1], 2) + pow(rot_[2][2], 2))) * (180.0 / M_PI);
    float rot_z = atan2(rot_[1][0], rot_[0][0]) * (180.0 / M_PI);
    
    dest[0] = rot_x;
    dest[1] = rot_z;
    dest[2] = -rot_y;
    return true;
}


bool physics_apply_force(PhysicsObjectID id, vec3 force) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj || !obj->body) {
        return false;
    }
    
    dBodySetLinearVel(obj->body, force[0], -force[2], force[1]);
    // dBodyAddForce(obj->body, force[0], -force[2], force[1]);
    return true;
}


bool physics_check_collision_at_position(PhysicsObjectID id, vec3 pos) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj || !obj->geom) {
        return false;
    }
    
    // Store original positions
    const dReal* original_geom_pos = dGeomGetPosition(obj->geom);
    dReal orig_geom_x = original_geom_pos[0];
    dReal orig_geom_y = original_geom_pos[1];
    dReal orig_geom_z = original_geom_pos[2];
    
    const dReal* original_body_pos = NULL;
    dReal orig_body_x, orig_body_y, orig_body_z;
    if (obj->body) {
        original_body_pos = dBodyGetPosition(obj->body);
        orig_body_x = original_body_pos[0];
        orig_body_y = original_body_pos[1];
        orig_body_z = original_body_pos[2];
        
        // Move body to test position
        dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
    } else {
        // Move geometry to test position
        dGeomSetPosition(obj->geom, pos[0], -pos[2], pos[1]);
    }
    
    // Check for collisions with all other geometries in space
    bool has_collision = false;
    
    // Iterate through all objects in space
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (!self.objects[i].in_use || self.objects[i].id == id) {
            continue;
        }
        
        if (self.objects[i].geom) {
            dContactGeom contact;
            int n = dCollide(obj->geom, self.objects[i].geom, 1, &contact, sizeof(dContactGeom));
            if (n > 0) {
                // Check if collision is significant (not just touching ground)
                // Allow small vertical overlaps (for ground contact)
                if (contact.depth > 0.01) {
                    has_collision = true;
                    break;
                }
            }
        }
    }
    
    // Restore original positions
    if (obj->body) {
        dBodySetPosition(obj->body, orig_body_x, orig_body_y, orig_body_z);
    } else {
        dGeomSetPosition(obj->geom, orig_geom_x, orig_geom_y, orig_geom_z);
    }
    
    return has_collision;
}


bool physics_check_ground_collision(PhysicsObjectID id, vec3 pos) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj || !obj->geom) {
        return false;
    }
    
    // Store original positions
    const dReal* original_geom_pos = dGeomGetPosition(obj->geom);
    dReal orig_geom_x = original_geom_pos[0];
    dReal orig_geom_y = original_geom_pos[1];
    dReal orig_geom_z = original_geom_pos[2];
    
    const dReal* original_body_pos = NULL;
    dReal orig_body_x, orig_body_y, orig_body_z;
    if (obj->body) {
        original_body_pos = dBodyGetPosition(obj->body);
        orig_body_x = original_body_pos[0];
        orig_body_y = original_body_pos[1];
        orig_body_z = original_body_pos[2];
        
        // Move body slightly down to test for ground
        dBodySetPosition(obj->body, pos[0], -(pos[2] - 0.05), pos[1]);
    } else {
        // Move geometry slightly down to test for ground
        dGeomSetPosition(obj->geom, pos[0], -(pos[2] - 0.05), pos[1]);
    }
    
    // Check for any collision (simplified ground detection)
    bool has_ground_collision = false;
    
    // Iterate through all objects in space
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (!self.objects[i].in_use || self.objects[i].id == id) {
            continue;
        }
        
        if (self.objects[i].geom) {
            dContactGeom contact;
            int n = dCollide(obj->geom, self.objects[i].geom, 1, &contact, sizeof(dContactGeom));
            if (n > 0) {
                has_ground_collision = true;
                break;
            }
        }
    }
    
    // Restore original positions
    if (obj->body) {
        dBodySetPosition(obj->body, orig_body_x, orig_body_y, orig_body_z);
    } else {
        dGeomSetPosition(obj->geom, orig_geom_x, orig_geom_y, orig_geom_z);
    }
    
    return has_ground_collision;
}


bool physics_check_wall_collision(PhysicsObjectID id, vec3 pos) {
    PhysicsObject* obj = find_physics_object(id);
    if (!obj || !obj->geom) {
        return false;
    }
    
    // Store original positions
    const dReal* original_geom_pos = dGeomGetPosition(obj->geom);
    dReal orig_geom_x = original_geom_pos[0];
    dReal orig_geom_y = original_geom_pos[1];
    dReal orig_geom_z = original_geom_pos[2];
    
    const dReal* original_body_pos = NULL;
    dReal orig_body_x, orig_body_y, orig_body_z;
    if (obj->body) {
        original_body_pos = dBodyGetPosition(obj->body);
        orig_body_x = original_body_pos[0];
        orig_body_y = original_body_pos[1];
        orig_body_z = original_body_pos[2];
        
        // Move body to test position
        dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
    } else {
        // Move geometry to test position
        dGeomSetPosition(obj->geom, pos[0], -pos[2], pos[1]);
    }
    
    // Check for wall/cube collisions (exclude ground)
    bool has_wall_collision = false;
    
    // Iterate through all objects in space
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (!self.objects[i].in_use || self.objects[i].id == id) {
            continue;
        }
        
        if (self.objects[i].geom) {
            dContactGeom contact;
            int n = dCollide(obj->geom, self.objects[i].geom, 1, &contact, sizeof(dContactGeom));
            if (n > 0) {
                // Only count as wall collision if:
                // 1. Normal is NOT pointing upward (not ground)
                // 2. Collision depth is significant (not just floor tile edges)
                if (contact.normal[2] <= 0.5 && contact.depth > 0.02) {  // More strict ground check + depth threshold
                    has_wall_collision = true;
                    break;
                }
            }
        }
    }
    
    // Restore original positions
    if (obj->body) {
        dBodySetPosition(obj->body, orig_body_x, orig_body_y, orig_body_z);
    } else {
        dGeomSetPosition(obj->geom, orig_geom_x, orig_geom_y, orig_geom_z);
    }
    
    return has_wall_collision;
}


static const int MAX_CONTACTS = 8;

static
void collision_callback(void* data, dGeomID geom1, dGeomID geom2) {
    dBodyID body1 = dGeomGetBody(geom1);
    dBodyID body2 = dGeomGetBody(geom2);
    
    // Skip self-collisions for the same body
    if (body1 && body2 && body1 == body2) return;
    
    // Skip collisions between two static objects (both without bodies)
    if (!body1 && !body2) return;

    dContact contact[MAX_CONTACTS];
    int n = dCollide(geom1, geom2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact));

    for (int i = 0; i < n; i++) {
        // Check if either body is kinematic for zero friction
        bool is_kinematic = false;
        if (body1 && dBodyIsKinematic(body1)) is_kinematic = true;
        if (body2 && dBodyIsKinematic(body2)) is_kinematic = true;
        
        if (is_kinematic) {
            // For kinematic bodies, create contact only for the kinematic body (player gets blocked, cube doesn't move)
            contact[i].surface.mode = dContactBounce | dContactSoftERP | dContactSoftCFM;
            contact[i].surface.mu = 0.0;  // No friction
            contact[i].surface.mu2 = 0;
            contact[i].surface.bounce = 0.001;  // Almost no bounce
            contact[i].surface.bounce_vel = 0.001;
            contact[i].surface.soft_erp = 0.15;  // Strong enough to block player
            contact[i].surface.soft_cfm = 0.02;
            
            // Only attach contact to kinematic body (player), not the rigid body (cube)
            dJointID c = dJointCreateContact(self.world, self.contact_group, &contact[i]);
            if (body1 && dBodyIsKinematic(body1)) {
                dJointAttach(c, body1, NULL);  // Only kinematic body feels the force
            } else if (body2 && dBodyIsKinematic(body2)) {
                dJointAttach(c, body2, NULL);  // Only kinematic body feels the force
            }
        } else {
            // Normal contact for rigid-rigid collisions
            contact[i].surface.mode = dContactBounce | dContactSoftERP | dContactSoftCFM;
            contact[i].surface.mu = dInfinity;
            contact[i].surface.mu2 = 0;
            contact[i].surface.bounce = 0.1;
            contact[i].surface.bounce_vel = 0.1;
            contact[i].surface.soft_erp = 0.2;
            contact[i].surface.soft_cfm = 0.001;
            
            dJointID c = dJointCreateContact(self.world, self.contact_group, &contact[i]);
            dJointAttach(c, dGeomGetBody(geom1), dGeomGetBody(geom2));
        }
    }
}


void physics_update() {
    dSpaceCollide(self.space, 0, collision_callback);
    dWorldQuickStep(self.world, TIMESTEP);
    dJointGroupEmpty(self.contact_group);
}
