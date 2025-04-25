#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // For strcasecmp

#include "object.h"

#include "cgm.h"
#include "log.h"
#include "physics.h"
#include "types.h"


Object* object_create(ObjectRecord* objrec, vec3 pos, vec3 rot, vec3 sc) {
    Object* obj = malloc(sizeof(Object));
    if (obj == NULL) {
        log_exit("Failed to allocate memory for Object (ID: %s)", objrec->id);
    }
    
    // Initialize the object with values from the ObjectRecord
    obj->base_id = objrec->id;
    obj->type = objrec->type;
    obj->meshes = objrec->meshes;
    obj->textures = objrec->textures;
    obj->slots_count = objrec->meshes_count;
    obj->is_active = true;
    obj->local_positions = NULL;
    obj->local_rotations = NULL;
    obj->m_models = NULL;
    
    // Set position, rotation, and scale with defaults if NULL
    if (pos != NULL)  glm_vec3_copy(    pos, obj->pos);
    else              glm_vec3_copy(VEC3__0, obj->pos);
    
    if (rot != NULL)  glm_vec3_copy(    rot, obj->rot);
    else              glm_vec3_copy(VEC3__0, obj->rot);
    
    if (sc != NULL)   glm_vec3_copy(     sc, obj->sc);
    else              glm_vec3_copy(VEC3__1, obj->sc);
    
    obj->physics_id = 0; // Default to no physics
    
    if (objrec->physics.has_physics) {
        PhysicsBodyType body_type = PHYSICS_BODY_BOX;
        
        if (strcasecmp(objrec->physics.collision_type, "BOX") == 0) {
            body_type = PHYSICS_BODY_BOX;
        }
        
        obj->physics_id = physics_create_object(
            body_type,
            obj->pos,
            obj->rot,
            objrec->physics.collision_size,
            objrec->physics.mass
        );
    }

    // Allocate memory for local positions, rotations, and model matrices
    obj->local_positions = malloc(sizeof(vec3) * obj->slots_count);
    if (obj->local_positions == NULL) {
        free(obj);
        log_exit("Failed to allocate memory for object local_positions (ID: %s)", obj->base_id);
    }
    
    obj->local_rotations = malloc(sizeof(vec3) * obj->slots_count);
    if (obj->local_rotations == NULL) {
        free(obj->local_positions);
        free(obj);
        log_exit("Failed to allocate memory for object local_rotations (ID: %s)", obj->base_id);
    }
    
    obj->m_models = malloc(sizeof(mat4) * obj->slots_count);
    if (obj->m_models == NULL) {
        free(obj->local_positions);
        free(obj->local_rotations);
        free(obj);
        log_exit("Failed to allocate memory for object m_models (ID: %s)", obj->base_id);
    }
    
    // Initialize the local positions and rotations
    for (int i = 0; i < obj->slots_count; i++) {
        glm_vec3_copy(objrec->local_positions[i], obj->local_positions[i]);
        glm_vec3_copy(objrec->local_rotations[i], obj->local_rotations[i]);

        vec3 result_pos;
        glm_vec3_copy(obj->pos, result_pos);
        glm_vec3_sub(obj->pos, obj->local_positions[i], result_pos);
        
        vec3 result_rot;
        glm_vec3_copy(obj->rot, result_rot);
        glm_vec3_add(obj->rot, obj->local_rotations[i], result_rot);

        cgm_model_mat(result_pos, result_rot, obj->sc, obj->m_models[i]);
    }
    
    return obj;
}


void object_destroy(Object* obj) {
    if (obj == NULL)  return;
    
    // Clean up physics resources if assigned
    if (obj->physics_id != 0) {
        physics_remove_object(obj->physics_id);
    }
    
    free(obj->local_positions);
    free(obj->local_rotations);
    free(obj->m_models);
    free(obj);
}


const char* object_get_base_id(Object* obj) {
    return (const char*) obj->base_id;
}


const char* object_get_type_string(Object* obj) {
    if (obj->type == ObjectType_UNKNOWN)
        return "UNKNOWN";

    if (obj->type == ObjectType_RIGID_BODY)
        return "RIGID_BODY";

    log_exit("Unmatched object type: %i", obj->type);
    return "???";
}


void object_get_position(Object* obj, vec3 dest) {
    glm_vec3_copy(obj->pos, dest);
}


void object_get_rotation(Object* obj, vec3 dest) {
    glm_vec3_copy(obj->rot, dest);
}


void object_update_model_mat(Object* obj) {
    vec3 result_pos;
    vec3 result_rot;

    for (int i = 0; i < obj->slots_count; i++) {
        glm_vec3_sub(obj->pos, obj->local_positions[i], result_pos);
        glm_vec3_add(obj->rot, obj->local_rotations[i], result_rot);
        cgm_model_mat(result_pos, result_rot, obj->sc, obj->m_models[i]);
    }
}


void object_set_position(Object* obj, vec3 new_pos) {
    glm_vec3_copy(new_pos, obj->pos);
    object_update_model_mat(obj);
}


void object_set_rotation(Object* obj, vec3 new_rot) {
    glm_vec3_copy(new_rot, obj->rot);
    object_update_model_mat(obj);
}


// TODO: refactoring
void object_set_subm_rotation(Object* obj, vec3 new_rot, u32 slot_idx) {
    u32 i = slot_idx;
    vec3 result_pos;
    vec3 result_rot;

    glm_vec3_copy(new_rot, obj->local_rotations[i]);
    
    glm_vec3_sub(obj->pos, obj->local_positions[i], result_pos);    
    glm_vec3_add(obj->rot, obj->local_rotations[i], result_rot);

    cgm_model_mat(result_pos, result_rot, obj->sc, obj->m_models[i]);
}
