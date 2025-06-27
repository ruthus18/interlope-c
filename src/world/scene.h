#pragma once

#include "./object_ref.h"

#include "../database/schemas.h"


typedef struct Scene {
    ObjectRef** object_refs;

    vec3 player_init_pos;
    vec2 player_init_rot;
} Scene;


Scene* scene_new(SceneInfo*);
void scene_free(Scene*);

void scene_update(Scene*);
void scene_draw(Scene*);
