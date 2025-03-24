#pragma once
#include "object.h"


typedef struct ObjectsDB ObjectsDB;

ObjectsDB* objdb_read_toml(const char* toml_path);
void objdb_destroy(ObjectsDB*);
ObjectBase* objdb_find(ObjectsDB* objdb, const char* base_id);
