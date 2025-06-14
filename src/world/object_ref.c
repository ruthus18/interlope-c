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

    Object* obj = world_find_object(info->id);
    if (!obj)
        log_exit("[world] Object not found: %s", info->id);
    
    self->obj = obj;
    glm_vec3_copy(info->pos, self->position);
    glm_vec3_copy(info->rot, self->rotation);

    int nodes_vec3_size = sizeof(vec3) * count_(obj->model->nodes);
    if (obj->model) {
        self->node_positions = malloc(nodes_vec3_size);
        self->node_rotations = malloc(nodes_vec3_size);
    }

    self->physics_id = 0; // Default to no physics
    return self;
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

    // TODO: concat model node pos and rot
    cgm_model_mat(self->position, self->rotation, NULL, model_mat);

    for_each(node, self->obj->model->nodes) {
        gfx_draw_object(node->mesh, node->texture, model_mat, 0);
    }
}
