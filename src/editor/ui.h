#pragma once

#include "world/scene.h"


void editor_init();
void editor_destroy();
void editor_set_scene(Scene*);
void editor_update(bool visible);
