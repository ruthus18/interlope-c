#pragma once

#include "object_ref.h"

#include "core/containers/map.h"
#include "database/schemas.h"


typedef struct Scene {
    map(ObjectRef) object_refs;

    vec3 player_init_pos;
    vec2 player_init_rot;
} Scene;


Scene* scene_new(SceneInfo*);
void scene_free(Scene*);

ObjectRef* scene_get_oref_by_id(Scene* self, u32 ref_id);
ObjectRef* scene_get_oref_by_physics(Scene* self, PxObject* px_obj);
void scene_remove_oref(Scene* self, ObjectRef* oref);

void scene_update(Scene*);
void scene_draw(Scene*);
