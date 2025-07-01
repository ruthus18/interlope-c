#include "px_object.h"
#include "physics/px.h"

#include "core/log.h"

#define RCOMP  (M_PI / 180.0)


void px_object_free(PxObject* obj) {
    if (obj->geom) {
        dGeomSetBody(obj->geom, 0);
        dGeomDestroy(obj->geom);
    }
    if (obj->body) {
        dBodyDisable(obj->body);
        dBodySetData(obj->body, NULL);
        dBodyDestroy(obj->body);
    }
    free(obj);
}

/* ------ px_static ------ */
/* ------------------------------------------------------------------------- */

PxObject* px_static_create(PxBodyType type, vec3 pos, vec3 rot,vec3 size) {
    PxObject* obj = malloc(sizeof(PxObject));
    memset(obj, 0, sizeof(PxObject));

    if (type == PXBODY_BOX) {
        obj->geom = dCreateBox(px_get_space(), size[0], size[2], size[1]);
    }
    else if (type == PXBODY_CAPSULE) {
        obj->geom = dCreateCapsule(px_get_space(), size[0], size[1]);
    }

    dGeomSetPosition(obj->geom, pos[0], -pos[2], pos[1]);
    dMatrix3 R;
    dRFromEulerAngles(R, rot[0]*RCOMP, rot[2]*RCOMP, rot[1]*RCOMP);
    dGeomSetRotation(obj->geom, R);

    obj->id = px_next_id();
    obj->type = type;
    obj->body = NULL;  // Static objects don't have bodies

    px_add_object(obj);
    return obj;
}

void px_static_get_position(PxObject* obj, vec3 dest) {
    const dReal* pos = dGeomGetPosition(obj->geom);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}

void px_static_set_position(PxObject* obj, vec3 pos) {
    dGeomSetPosition(obj->geom, pos[0], -pos[2], pos[1]);
}

/* ------ px_rigid ------ */
/* ------------------------------------------------------------------------- */

PxObject* px_rigid_create(PxBodyType type, vec3 pos, vec3 rot, vec3 size, f32 mass) {
    PxObject* obj = malloc(sizeof(PxObject));
    memset(obj, 0, sizeof(PxObject));
    
    // Create the body
    obj->body = dBodyCreate(px_get_world());
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
    
    // Set rotation
    dMatrix3 R;
    dRFromEulerAngles(R, rot[0]*RCOMP, rot[2]*RCOMP, rot[1]*RCOMP);
    dBodySetRotation(obj->body, R);
    
    // Set mass and type-specific properties
    dMass mass_;
    
    switch (type) {
        case PXBODY_BOX:
            dMassSetBoxTotal(&mass_, mass, size[0], size[2], size[1]);
            obj->geom = dCreateBox(px_get_space(), size[0], size[2], size[1]);
            break;

        case PXBODY_CAPSULE:
            dMassSetCapsuleTotal(&mass_, mass, 3, size[0], size[1]);
            obj->geom = dCreateCapsule(px_get_space(), size[0], size[1]);
            break;

        default:
            dBodyDestroy(obj->body);
            log_exit("Physics error: Unsupported body type %d", type);
    }
    
    dBodySetMass(obj->body, &mass_);
    dGeomSetBody(obj->geom, obj->body);

    obj->id = px_next_id();
    obj->type = type;
    
    px_add_object(obj);
    return obj;
}

void px_rigid_get_position(PxObject* obj, vec3 dest) {
    const dReal* pos = dBodyGetPosition(obj->body);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}

void px_rigid_set_position(PxObject* obj, vec3 pos) {
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
}

void px_rigid_get_rotation(PxObject* obj, vec3 dest) {
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

PxObject* px_kinematic_create(PxBodyType type, vec3 pos, vec3 rot, vec3 size) {
    PxObject* obj = malloc(sizeof(PxObject));
    memset(obj, 0, sizeof(PxObject));
    
    // Create the body
    obj->body = dBodyCreate(px_get_world());
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
    
    // Set rotation
    dMatrix3 R;
    dRFromEulerAngles(R, rot[0]*RCOMP, rot[2]*RCOMP, rot[1]*RCOMP);
    dBodySetRotation(obj->body, R);
    
    // Make it kinematic (no gravity, infinite mass)
    dBodySetKinematic(obj->body);
    
    // Create geometry based on type
    if (type == PXBODY_BOX) {
        obj->geom = dCreateBox(px_get_space(), size[0], size[2], size[1]);
    }
    else if (type == PXBODY_CAPSULE) {
        obj->geom = dCreateCapsule(px_get_space(), size[0], size[1] - size[0]);
    }
    else {
        log_error("Physics error: Unsupported body type %d", type);
        dBodyDestroy(obj->body);
        free(obj);
        return NULL;
    }
    dGeomSetBody(obj->geom, obj->body);
    
    obj->id = px_next_id();
    obj->type = type;
    
    px_add_object(obj);
    return obj;
}

void px_kinematic_get_position(PxObject* obj, vec3 dest) {
    const dReal* pos = dBodyGetPosition(obj->body);
    glm_vec3_copy((vec3){pos[0], pos[2], -pos[1]}, dest);
}
void px_kinematic_set_position(PxObject* obj, vec3 pos) {
    dBodySetPosition(obj->body, pos[0], -pos[2], pos[1]);
}

void px_kinematic_get_rotation(PxObject* obj, vec3 dest) {
    const dReal* R = dBodyGetRotation(obj->body);
    
    mat3 rot_ = {R[0], R[1], R[2], R[4], R[5], R[6], R[8], R[9], R[10]};
    
    float rot_x = atan2(rot_[2][1], rot_[2][2]) * (180.0 / M_PI);
    float rot_y = atan2(-rot_[2][0], sqrt(pow(rot_[2][1], 2) + pow(rot_[2][2], 2))) * (180.0 / M_PI);
    float rot_z = atan2(rot_[1][0], rot_[0][0]) * (180.0 / M_PI);
    
    dest[0] = rot_x;
    dest[1] = rot_z;
    dest[2] = -rot_y;
}
