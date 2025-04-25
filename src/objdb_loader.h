#pragma once
#include <toml.h>

#include "objdb.h"

ObjectsDB* objdb_load_toml(const char* toml_path);