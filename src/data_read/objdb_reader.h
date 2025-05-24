#pragma once
#include <toml.h>

#include "../world/objdb.h"

ObjectsDB* objdb_read_toml(const char* toml_path);