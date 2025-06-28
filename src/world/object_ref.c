#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>

#include "object_ref.h"
#include "./world.h"

#include "../core/cgm.h"
#include "../core/log.h"
#include "../core/utils.h"

static u32 next_ref_id = 0x00000001;


ObjectRef* object_ref_new(ObjectRefInfo* info) {
    ObjectRef* self = malloc(sizeof(ObjectRef));
    memset(self, 0, sizeof(ObjectRef));

    /* --- Reference ID --- */
    self->ref_id = next_ref_id++;

    /* --- Object --- */
    Object* obj = world_get_object(info->id);
    if (!obj)
        log_exit("[world] Object not found: %s", info->id);
    
    self->obj = obj;
    glm_vec3_copy(info->pos, self->position);
    glm_vec3_copy(info->rot, self->rotation);

    /* --- Object Model --- */
    int nodes_vec3_size = sizeof(vec3) * tuple_size(obj->model->nodes);
    if (obj->model) {
        self->node_positions = malloc(nodes_vec3_size);
        self->node_rotations = malloc(nodes_vec3_size);
    }

    /* --- Object Physics --- */
    object_ref_create_physics(self, obj->info->physics);

    return self;
}


void object_ref_create_physics(ObjectRef* self, PhysicsInfo** infos) {
    self->physics_ids = NULL;
    if (!infos)  return;
    
    int physics_count = tuple_size(infos);
    if (physics_count == 0) return;
    
    int physics_size = sizeof(PhysicsObjectID) * (physics_count + 1);
    self->physics_ids = malloc(physics_size);
    memset(self->physics_ids, 0, physics_size);
    
    for (int i = 0; i < physics_count; i++) {
        PhysicsInfo* info = infos[i];
        if (!info)  continue;
        
        vec3 pos, rot, size;
        PhysicsBodyType body_type;

        if (info->shape == PHSHAPE_BOX) {
            glm_vec3_add(self->position, info->pos, pos);
            // glm_vec3_add(self->rotation, info->rot, relative_rot);  // TODO: Not supported at now
            glm_vec3_copy(self->rotation, rot);
            glm_vec3_negate(rot);
            glm_vec3_copy(info->size, size);
            body_type = PHYSICS_BODY_BOX;
        }
        else if (info->shape == PHSHAPE_AABB) {
            glm_vec3_add(self->position, self->obj->model->aabb.offset, pos);
            // glm_vec3_add(self->rotation, info->rot, relative_rot);  // TODO: Not supported at now
            glm_vec3_copy(self->rotation, rot);
            glm_vec3_negate(rot);
            glm_vec3_copy(self->obj->model->aabb.size, size);
            body_type = PHYSICS_BODY_BOX;
        }
        else if (info->shape == PHSHAPE_NULL) {
            log_error("Unknown physics shape in object: %s", self->obj->base_id);
            return;
        }
        else {
            log_exit("Unhandled physics shape");
        }

        if (self->obj->type == OBJECT_STATIC)
            self->physics_ids[i] = physics_create_static_object(body_type, pos, rot, size);

        else if (self->obj->type == OBJECT_ITEM)
            self->physics_ids[i] = physics_create_rigid_object(body_type, pos, rot, size, 1.0);
    }   
}


void object_ref_free(ObjectRef* self) {
    if (self->node_positions)  free(self->node_positions);
    if (self->node_rotations)  free(self->node_rotations);
    if (self->physics_ids)     free(self->physics_ids);
    free(self);
}


void object_ref_update(ObjectRef* self) {
    object_update(self->obj);

    if (self->obj->type == OBJECT_ITEM) {
        vec3 pos, rot;
        // TODO: check on `object_ref_new` that there is only 1 physics body 
        physics_get_object_position(self->physics_ids[0], pos);
        physics_get_object_rotation(self->physics_ids[0], rot);
        glm_vec3_copy(pos, self->position);
        glm_vec3_copy(rot, self->rotation);
    }
}


void object_ref_draw(ObjectRef* self) {
    ModelNode* node;
    mat4 model_mat;

    // TODO: concat ModelNode pos and rot
    cgm_model_mat(self->position, self->rotation, NULL, model_mat);

    tuple_for_each(node, self->obj->model->nodes) {
        gfx_enqueue_object(node->mesh, node->texture, model_mat);
    }
}


ObjectRef* object_ref_find_by_id(u32 ref_id) {
    extern Scene* world_get_current_scene(void);  // Forward declaration
    Scene* scene = world_get_current_scene();
    if (!scene) return NULL;

    ObjectRef* obj_ref;
    tuple_for_each(obj_ref, scene->object_refs) {
        if (obj_ref->ref_id == ref_id) {
            return obj_ref;
        }
    }
    return NULL;
}

u32 object_ref_get_next_id(void) {
    return next_ref_id;
}
