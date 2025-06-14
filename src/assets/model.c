#include <string.h>

#include "model.h"
#include "./mesh_gltf.h"
#include "./texture.h"

#include "../core/log.h"
#include "../core/utils.h"


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
    }

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
