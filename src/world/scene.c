#include <string.h>

#include "scene.h"

#include "core/containers/tuple.h"
#include "physics/px_object.h"


Scene* scene_new(SceneInfo* info) {
    Scene* self = malloc(sizeof(Scene));
    memset(self, 0, sizeof(Scene));
    
    self->object_refs = map_new(MHASH_INT);

    ObjectRefInfo* oref_info;

    tuple_for_each(oref_info, info->object_refs) {
        ObjectRef* oref = object_ref_new(oref_info);
        map_set(self->object_refs, oref, (void*)(intptr_t)oref->ref_id);
    }

    glm_vec3_copy(info->player_init_pos, self->player_init_pos);
    glm_vec2_copy(info->player_init_rot, self->player_init_rot);

    return self;
}

void scene_free(Scene* self) {
    ObjectRef* oref;
    map_for_each(oref, self->object_refs) {
        object_ref_free(oref);
    }

    map_free(self->object_refs);
    free(self);
}

/* ------------------------------------------------------------------------- */

ObjectRef* scene_get_oref_by_id(Scene* self, u32 ref_id) {
    return map_get(self->object_refs, (void*)(intptr_t)ref_id);
}

ObjectRef* scene_get_oref_by_physics(Scene* self, PxObject* px_obj) {
    ObjectRef* oref;
    PxObject* i_obj;

    // FIXME
    map_for_each(oref, self->object_refs) {
        tuple_for_each(i_obj, oref->physics) {
            if (i_obj == px_obj)
                return oref;
        }
    }
    return NULL;
}

void scene_remove_oref(Scene* self, ObjectRef* oref) {
    map_remove(self->object_refs, (void*)(intptr_t)oref->ref_id);
    object_ref_free(oref);
}

/* ------------------------------------------------------------------------- */

void scene_update(Scene* self) {
    ObjectRef* oref;
    map_for_each(oref, self->object_refs) {
        object_ref_update(oref);
    }
}

void scene_draw(Scene* self) {
    ObjectRef* oref;
    map_for_each(oref, self->object_refs) {
        object_ref_draw(oref);
    }
}
