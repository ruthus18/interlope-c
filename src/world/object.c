#include <string.h>

#include "object.h"

#include "../assets/model.h"


Object* object_create_from_info(ObjectInfo* info) {
    Object* obj = malloc(sizeof(Object));
    memset(obj, 0, sizeof(Object));

    strcpy(obj->base_id, info->id);
    obj->info = info;

    Model* model = model_create_from_info(info->model);
    obj->model = model;

    return obj;
}


void object_destroy(Object* obj) {
    model_destroy(obj->model);
    free(obj);
}


void object_update(Object* obj) {
    
}
