#include <stdio.h>
#include <math.h>
#include <ode/ode.h>
#include <cglm/cglm.h>

#include "physics.h"

#include "config.h"
#include "types.h"
#include "log.h"


// Documentation: https://ode.org/wiki/index.php/Manual


static const dReal GRAVITY = -9.81;
static const dReal TIMESTEP = (1.0 / WINDOW_MAX_FRAMERATE);

#define MAX_PHYSICS_OBJECTS 128
#define INVALID_PHYSICS_ID 0


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


static dGeomID ground_geom;

// Legacy references for backwards compatibility
static dBodyID cube_body;
static dGeomID cube_geom;


static
void custom_message_handler(int num, const char *msg, va_list args) {
    // skip linear complementarity problem
    if (num == 3)  return;

    char fmt_msg[256];
    vsprintf(fmt_msg, msg, args);
    log_info("ODE Message %i: %s", num, fmt_msg);
}


// Helper function to find an object by ID
static PhysicsObject* find_physics_object(PhysicsObjectID id) {
    if (id == INVALID_PHYSICS_ID) return NULL;
    
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (self.objects[i].in_use && self.objects[i].id == id) {
            return &self.objects[i];
        }
    }
    return NULL;
}

// Helper function to find an empty slot
static PhysicsObject* find_empty_slot() {
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (!self.objects[i].in_use) {
            return &self.objects[i];
        }
    }
    return NULL;
}

void physics_init() {
    dSetMessageHandler(custom_message_handler);
    dInitODE();

    self.world = dWorldCreate();
    self.space = dHashSpaceCreate(0);
    self.contact_group = dJointGroupCreate(0);
    self.next_id = 1; // Start IDs at 1, 0 is invalid

    // Initialize the objects array
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        self.objects[i].in_use = false;
        self.objects[i].id = INVALID_PHYSICS_ID;
        self.objects[i].body = NULL;
        self.objects[i].geom = NULL;
    }

    dWorldSetGravity(self.world, 0.0, 0.0, GRAVITY);
}


void physics_destroy() {
    // Clean up all physics objects
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (self.objects[i].in_use) {
            if (self.objects[i].geom) {
                dGeomDestroy(self.objects[i].geom);
            }
            // Note: we're keeping the FIXME comment about body destruction
            // that was in the original code
            /*
            // FIXME cause segfault, need to investigate
            if (self.objects[i].body) {
                dBodyDestroy(self.objects[i].body);
            }
            */
        }
    }
    
    // Legacy cube cleanup if it exists
    if (cube_geom) {
        dGeomDestroy(cube_geom);
    }
    
    dJointGroupDestroy(self.contact_group);
    dSpaceDestroy(self.space);
    dWorldDestroy(self.world);
    
    dCloseODE();
}

void physics_create_ground() {
    ground_geom = dCreatePlane(self.space, 0, 0, 1, 0);
}


PhysicsObjectID physics_create_object(
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
    
    // Enable auto-disabling for performance
    dBodySetAutoDisableFlag(obj->body, 1);
    dBodySetAutoDisableLinearThreshold(obj->body, 0.05);
    
    // Setup object metadata
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
    
    /*
    // FIXME: Same issue as mentioned in physics_destroy
    if (obj->body) {
        dBodyDestroy(obj->body);
    }
    */
    
    // Mark as unused
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
    
    // Apply force at the center of mass (position 0,0,0 relative to body)
    dBodyAddForce(obj->body, force[0], -force[2], force[1]);
    return true;
}


// Legacy function implementation using the new system
void physics_create_cube(vec3 pos, vec3 rot, vec3 size, f32 mass) {
    PhysicsObjectID id = physics_create_object(PHYSICS_BODY_BOX, pos, rot, size, mass);
    
    // Store references for backwards compatibility
    if (id != INVALID_PHYSICS_ID) {
        PhysicsObject* obj = find_physics_object(id);
        cube_body = obj->body;
        cube_geom = obj->geom;
    }
}


// Legacy getter function implementations using the new system
void physics_get_cube_position(vec3 dest) {
    if (cube_body) {
        const dReal* pos = dBodyGetPosition(cube_body);
        glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
    }
}


void physics_get_cube_rotation(vec3 dest) {
    if (cube_body) {
        const dReal* R = dBodyGetRotation(cube_body);
        
        mat3 rot_ = {R[0], R[1], R[2], R[4], R[5], R[6], R[8], R[9], R[10]};
        
        float rot_x = atan2(rot_[2][1], rot_[2][2]) * (180.0 / M_PI);
        float rot_y = atan2(-rot_[2][0], sqrt(pow(rot_[2][1], 2) + pow(rot_[2][2], 2))) * (180.0 / M_PI);
        float rot_z = atan2(rot_[1][0], rot_[0][0]) * (180.0 / M_PI);
        
        dest[0] = rot_x;
        dest[1] = rot_z;
        dest[2] = -rot_y;
    }
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

        dJointID contact = dJointCreateContact(
            self.world, self.contact_group, &contacts[i]
        );
        dJointAttach(contact, dGeomGetBody(geom1), dGeomGetBody(geom2));
    }
}


void physics_update() {
    dSpaceCollide(self.space, 0, collision_callback);
    dWorldStep(self.world, TIMESTEP);
    dJointGroupEmpty(self.contact_group);
}
