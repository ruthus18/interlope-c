#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>

#include "object_ref.h"
#include "./world.h"

#include "../core/cgm.h"
#include "../core/log.h"
#include "../core/utils.h"


ObjectRef* object_ref_create_from_info(ObjectRefInfo* info) {
    ObjectRef* self = malloc(sizeof(ObjectRef));
    memset(self, 0, sizeof(ObjectRef));

    /* --- Object --- */
    Object* obj = world_find_object(info->id);
    if (!obj)
        log_exit("[world] Object not found: %s", info->id);
    
    self->obj = obj;
    glm_vec3_copy(info->pos, self->position);
    glm_vec3_copy(info->rot, self->rotation);

    /* --- Object Model --- */
    int nodes_vec3_size = sizeof(vec3) * count_(obj->model->nodes);
    if (obj->model) {
        self->node_positions = malloc(nodes_vec3_size);
        self->node_rotations = malloc(nodes_vec3_size);
    }

    /* --- Object Physics --- */
    object_ref_create_physics(self, obj->info->physics);

    return self;
}


void object_ref_create_physics(ObjectRef* self, PhysicsInfo* info) {
    self->physics_id = 0;
    if (!info)  return;
    
    vec3 relative_pos;
    glm_vec3_add(self->position, info->pos, relative_pos);

    switch (info->shape) {
        case PHSHAPE_BOX:
            self->physics_id = physics_create_static_object(
                PHYSICS_BODY_BOX,
                relative_pos,
                self->rotation,
                info->size
            );
            break;
            
        case PHSHAPE_AABB:
            log_error("AABB phyics shape unsupported for now...");
            break;

        case PHSHAPE_NULL:
            log_error("Unknown physics shape in object: %s", self->obj->base_id);
            break;
    }
}


void object_ref_destroy(ObjectRef* self) {
    if (self->node_positions)  free(self->node_positions);
    if (self->node_rotations)  free(self->node_rotations);

    free(self);
}


void object_ref_update(ObjectRef* self) {

}


void object_ref_draw(ObjectRef* self) {
    ModelNode* node;
    mat4 model_mat;

    // TODO: concat ModelNode pos and rot
    cgm_model_mat(self->position, self->rotation, NULL, model_mat);

    for_each(node, self->obj->model->nodes) {
        gfx_draw_object(node->mesh, node->texture, model_mat, 0);
    }
}
