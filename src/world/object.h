#pragma once
#include <uthash.h>

#include "../assets/model.h"
#include "../core/map.h"
#include "../database/schemas.h"


typedef struct Object {
    char base_id[MAX_ID_LENGTH];
    
    Model* model;
    ObjectInfo* info;
    ObjectType type;

    map_item_meta;
} Object;


Object* object_new(ObjectInfo*);
void object_free(Object*);
void object_update(Object* obj);
