#pragma once
#include <toml.h>

#include "objdb.h"
#include "scene.h"

Scene* scene_read_toml(const char* toml_path, ObjectsDB* objdb);