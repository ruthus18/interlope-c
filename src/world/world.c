#include <string.h>
#include <stdlib.h>

#include "world.h"
#include "world/object.h"
#include "world/scene.h"

#include "core/containers/map.h"
#include "core/containers/tuple.h"
#include "core/log.h"
#include "database/db.h"
#include "gameplay/player.h"


static struct World {
    map(Object) objects;
    Scene* current_scene;
} self = {};


void world_init() {
    self.objects = map_new(MHASH_STR);
    
    /* --- Objects Loading --- */
    Database* db = db_get();
    ObjectInfo* obj_info;
    Object* obj;

    tuple_for_each(obj_info, db->objects) {
        obj = object_new(obj_info);
        map_set(self.objects, obj, obj->base_id);
    }

    /* --- Scene Loading --- */
    Scene* scene = scene_new(db->scene);
    self.current_scene = scene;

    /* --- Player Loading */
    player_init(scene->player_init_pos, scene->player_init_rot);

    world_print();
}


void world_destroy() {
    player_destroy();
    scene_free(self.current_scene);

    Object* obj;
    map_for_each(obj, self.objects) {
        object_free(obj);
    }
    map_free(self.objects);
}


void world_print() {
    log_debug("total Object: %i", map_size(self.objects));
    log_debug("total ObjectRef: %i", tuple_size(self.current_scene->object_refs));
}


Object* world_get_object(char* id) {
    return map_get(self.objects, id);
}

Scene* world_get_current_scene() {
    return self.current_scene;
}

ObjectRef* world_get_oref_by_id(u32 ref_id) {
    return scene_get_oref_by_id(self.current_scene, ref_id);
}

ObjectRef* world_get_oref_by_physics(PxObject* px_obj) {
    return scene_get_oref_by_physics(self.current_scene, px_obj);
}

void world_remove_oref(ObjectRef* oref) {
    scene_remove_oref(self.current_scene, oref);
}


void world_update() {
    player_update();
    scene_update(self.current_scene);
}

void world_draw() {
    scene_draw(self.current_scene);
}
