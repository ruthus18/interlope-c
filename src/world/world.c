#include <string.h>

#include "world.h"
#include "world/object.h"
#include "world/scene.h"

#include "core/memory.h"
#include "core/log.h"
#include "core/utils.h"
#include "database/db.h"
#include "gameplay/player.h"


static struct World {
    map(Object) objects;
    Scene* current_scene;
} self = {};


void world_init() {
    Database* db = db_get();

    /* --- Objects Loading --- */
    self.objects = NULL;

    ObjectInfo* obj_info;
    Object* obj;

    tuple_for_each(obj_info, db->objects) {
        obj = object_new(obj_info);
        map_set(self.objects, base_id, obj);
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

    Object* obj;
    map_for_each(obj, self.objects) {
        map_delete(self.objects, obj);
        object_free(obj);
    }
    scene_free(self.current_scene);
}


void world_print() {
    log_debug("total Object: %i", map_size(self.objects));
    log_debug("total ObjectRef: %i", tuple_size(self.current_scene->object_refs));
}


Object* world_get_object(char* id) {
    Object* obj = NULL;
    map_get(self.objects, id, obj);
    return obj;
}

Scene* world_get_current_scene() {
    return self.current_scene;
}


void world_update() {
    scene_update(self.current_scene);
    player_update();
}

void world_draw() {
    scene_draw(self.current_scene);
}
