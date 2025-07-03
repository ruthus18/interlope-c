#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>

#include "object_ref.h"
#include "world/world.h"

#include "core/containers/tuple.h"
#include "core/containers/map.h"
#include "core/cgm.h"
#include "core/log.h"

static u32 next_ref_id = 0x00000001;

/* ------------------------------------------------------------------------- */

static inline void _create_physics(ObjectRef* self, PhysicsInfo** infos);

/* ------------------------------------------------------------------------- */

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
    _create_physics(self, obj->info->physics);

    return self;
}

void object_ref_free(ObjectRef* self) {
    if (self->node_positions)   free(self->node_positions);
    if (self->node_rotations)   free(self->node_rotations);

    if (self->physics)          free(self->physics);
    // FIXME: also need to remove related px_objects from px storage
    free(self);
}

static inline
void _create_physics(ObjectRef* self, PhysicsInfo** infos) {
    self->physics = NULL;
    if (!infos)  return;
    
    int physics_count = tuple_size(infos);
    if (physics_count == 0) return;
    
    int physics_size = sizeof(PxObject) * (physics_count + 1);
    self->physics = malloc(physics_size);
    memset(self->physics, 0, physics_size);
    
    for (int i = 0; i < physics_count; i++) {
        PhysicsInfo* info = infos[i];
        if (!info)  continue;
        
        vec3 pos, rot, size;
        PxBodyType body_type;

        if (info->shape == PHSHAPE_BOX) {
            body_type = PXBODY_BOX;
            glm_vec3_add(self->position, info->pos, pos);
            // glm_vec3_add(self->rotation, info->rot, relative_rot);  // TODO: Not supported at now
            glm_vec3_copy(self->rotation, rot);
            glm_vec3_negate(rot);
            glm_vec3_copy(info->size, size);
        }
        else if (info->shape == PHSHAPE_AABB) {
            body_type = PXBODY_BOX;
            glm_vec3_add(self->position, self->obj->model->aabb.offset, pos);
            // glm_vec3_add(self->rotation, info->rot, relative_rot);  // TODO: Not supported at now
            glm_vec3_copy(self->rotation, rot);
            glm_vec3_negate(rot);
            glm_vec3_copy(self->obj->model->aabb.size, size);
        }
        else if (info->shape == PHSHAPE_NULL) {
            log_error("Unknown physics shape in object: %s", self->obj->base_id);
            return;
        }
        else {
            log_exit("Unhandled physics shape");
        }

        if (self->obj->type == OBJECT_STATIC)
            self->physics[i] = px_static_create(body_type, pos, rot, size);
            
        else if (self->obj->type == OBJECT_ITEM)
            self->physics[i] = px_static_create(body_type, pos, rot, size);
    }
}

/* ------------------------------------------------------------------------- */

void object_ref_update(ObjectRef* self) {
    object_update(self->obj);
    
    if (self->obj->type == OBJECT_ITEM) {
        // TODO: check on `object_ref_new` that there is only 1 physics body 
        px_static_get_position(self->physics[0], self->position);
        px_static_get_rotation(self->physics[0], self->rotation);
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
