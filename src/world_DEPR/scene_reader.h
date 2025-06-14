#pragma once
#include <toml.h>

#include "../world/objdb.h"
#include "../world/scene.h"

Scene* scene_read_toml(const char* toml_path, ObjectsDB* objdb);