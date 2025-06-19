#pragma once

#include "database/schemas.h"

#include "core/types.h"


typedef struct Database {
    ObjectInfo** objects;
    SceneInfo* scene;
} Database;


void db_init();
void db_destroy();

Database* db_get();
ObjectInfo* db_find_object(char* obj_id);
