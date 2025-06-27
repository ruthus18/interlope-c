#pragma once
#include "../assets/model.h"
#include "../database/schemas.h"


typedef struct Object {
    char base_id[MAX_ID_LENGTH];

    Model* model;
    ObjectInfo* info;
    ObjectType type;
} Object;


Object* object_new(ObjectInfo*);
void object_free(Object*);
void object_update(Object* obj);
