#pragma once

#include "../core/types.h"
#include "./schemas.h"

typedef struct Database {
    ObjectInfo** objects;
    SceneInfo* scene;
} Database;


void db_init();
void db_destroy();

Database* db_get();
ObjectInfo* db_find_object(char* obj_id);
