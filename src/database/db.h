#pragma once

#include "../core/types.h"
#include "./schemas.h"

typedef struct Database {
    ObjectInfo** objects;
    u32 objects_count;

    SceneInfo* scene;
} Database;


void db_init();
void db_destroy();

Database* db_get();
ObjectInfo* db_get_object_info(char* obj_id);
