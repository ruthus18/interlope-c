#include <float.h>
#include <string.h>

#include "model.h"
#include "assets/mesh_gltf.h"
#include "assets/texture.h"

#include "core/log.h"
#include "core/utils.h"


static vec3 AABB_MIN = {FLT_MAX, FLT_MAX, FLT_MAX};
static vec3 AABB_MAX = {-FLT_MAX, -FLT_MAX, -FLT_MAX};


static inline
Model* _model_alloc(int nodes_count) {
    Model* model = malloc(sizeof(Model));
    memset(model, 0, sizeof(Model));

    int nodes_size = sizeof(ModelNode*) * (nodes_count + 1);
    model->nodes = malloc(nodes_size);
    memset(model->nodes, 0, nodes_size);

    for (int i = 0; i < nodes_count; i++) {
        model->nodes[i] = malloc(sizeof(ModelNode));
        memset(model->nodes[i], 0, sizeof(ModelNode));
    }

    return model;
}

static inline
void _model_free(Model* model) {
    ModelNode* node;
    for_each(node, model->nodes) {
        free(node);
    }
    free(model->nodes);
    free(model);
}

static inline
void _model_calc_aabb(Model* m) {
    /* --- Total AABB Calculation --- */
    glm_vec3_copy(AABB_MIN, m->aabb.min);
    glm_vec3_copy(AABB_MAX, m->aabb.max);
    
    ModelNode* n;
    for_each(n, m->nodes) {
        if (n->aabb_min[0] < m->aabb.min[0])  m->aabb.min[0] = n->aabb_min[0];
        if (n->aabb_min[1] < m->aabb.min[1])  m->aabb.min[1] = n->aabb_min[1];
        if (n->aabb_min[2] < m->aabb.min[2])  m->aabb.min[2] = n->aabb_min[2];
        
        if (n->aabb_max[0] > m->aabb.max[0])  m->aabb.max[0] = n->aabb_max[0];
        if (n->aabb_max[1] > m->aabb.max[1])  m->aabb.max[1] = n->aabb_max[1];
        if (n->aabb_max[2] > m->aabb.max[2])  m->aabb.max[2] = n->aabb_max[2];
    }
    
    /* --- Handle Case With Zero Nodes --- */
    if (glm_vec3_eqv(m->aabb.min, AABB_MIN))
        glm_vec3_zero(m->aabb.min);
    
    if (glm_vec3_eqv(m->aabb.max, AABB_MAX))
        glm_vec3_zero(m->aabb.max);
    
    /* --- Size & Offset Calculation --- */
    glm_vec3_sub(m->aabb.max, m->aabb.min, m->aabb.size);

    vec3 half_size;
    glm_vec3_copy(m->aabb.size, half_size);
    glm_vec3_scale(half_size, 0.5, half_size);

    glm_vec3_sub(m->aabb.max, half_size, m->aabb.offset);
}


Model* model_create_from_info(ModelInfo* info) {
    GLTF_Asset* gltf = gltf_open(info->mesh);
    int nodes_count = gltf_get_nodes_count(gltf);
    
    Model* model = _model_alloc(nodes_count);
    gltf_load_model_nodes(gltf, model->nodes);

    assert(info->texture_count == nodes_count);

    int i = 0;
    ModelNode* node;
    for_each(node, model->nodes) {
        node->texture = texture_load_dds(info->textures[i]);
        
        if (!gltf_get_mesh_aabb(gltf, i, node->aabb_min, node->aabb_max)) {
            log_info("Failed to extract AABB for node %d ('%s')", i, node->name);
            glm_vec3_zero(node->aabb_min);
            glm_vec3_zero(node->aabb_max);
        }
        i++;
    }
    _model_calc_aabb(model);

    gltf_close(gltf);
    return model;
}


void model_destroy(Model* model) {
    ModelNode* node;
    for_each(node, model->nodes) {
        gfx_mesh_unload(node->mesh);
        gfx_texture_unload(node->texture);
    };

    _model_free(model);
}
