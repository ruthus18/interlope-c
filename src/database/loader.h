#pragma once
#include "./schemas.h"


// WARNING: these functions requiring explicit `free` call after usage
ObjectInfo** db_load_objects_data(char* path);
SceneInfo* db_load_scene_data(char* path);
