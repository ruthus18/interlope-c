#pragma once
#include "../assets/model.h"
#include "../database/schemas.h"


typedef struct Object {
    char base_id[MAX_ID_LENGTH];

    Model* model;
    ObjectInfo* info;
} Object;


Object* object_create_from_info(ObjectInfo*);
void object_destroy(Object*);
void object_update(Object* obj);
