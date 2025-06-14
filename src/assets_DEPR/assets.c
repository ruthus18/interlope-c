#include "assets.h"

#include "./model.h"


Model_* assets_model_init() {
    Model_* model = malloc(sizeof(Model_));

    model->id[0] = '\0';
    model->nodes = NULL;
    model->nodes_count = 0;

    // Model* legacy_model = model_read(info->mesh);
    
    return model;
}


void assets_model_destroy(Model_* model) {
    if (model->nodes) {
        free(model->nodes);
    }

    free(model);
}
