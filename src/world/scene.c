#include <string.h>

#include "scene.h"
#include "core/memory.h"
#include "core/utils.h"
#include "render/gfx.h"


static inline
Scene* _scene_alloc() {
    Scene* scene = malloc(sizeof(Scene));
    memset(scene, 0, sizeof(Scene));

    int scene_size = sizeof(ObjectRef*) * (MEM_SCENE_OBJECTS + 1);
    scene->object_refs = malloc(scene_size);
    memset(scene->object_refs, 0, scene_size);

    return scene;
}

static inline
void _scene_free(Scene* scene) {
    free(scene->object_refs);
    free(scene);
}


Scene* scene_new(SceneInfo* info) {
    Scene* scene = _scene_alloc();

    int i = 0;
    ObjectRefInfo* obj_ref_info;

    for_each(obj_ref_info, info->object_refs) {
        ObjectRef* obj_ref = object_ref_new(obj_ref_info);
        scene->object_refs[i++] = obj_ref;
    }

    glm_vec3_copy(info->player_init_pos, scene->player_init_pos);
    glm_vec2_copy(info->player_init_rot, scene->player_init_rot);

    // TODO tmp, need to rm player collision offsets
    scene->player_init_pos[1] += 0.5;

    return scene;
}

void scene_free(Scene* scene) {
    ObjectRef* obj_ref;
    for_each(obj_ref, scene->object_refs) {
        object_ref_free(obj_ref);
    }

    _scene_free(scene);
}

void scene_update(Scene* scene) {
    ObjectRef* obj_ref;
    for_each(obj_ref, scene->object_refs) {
        object_ref_update(obj_ref);
    }
}

void scene_draw(Scene* scene) {
    gfx_begin_draw_objects();

    ObjectRef* ref;
    for_each(ref, scene->object_refs) {
        object_ref_draw(ref);
    }

    gfx_end_draw_objects();
}
