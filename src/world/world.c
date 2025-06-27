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
    Object** objects;
    Scene* current_scene;
} self;


static inline
void _world_new() {
    int objects_size = sizeof(Object*) * (MEM_WORLD_OBJECTS + 1);

    self.objects = malloc(objects_size);
    memset(self.objects, 0, objects_size);
}

static inline
void _world_free() {
    free(self.objects);
}


void world_init() {
    _world_new();
    Database* db = db_get();

    /* --- Objects Loading --- */
    int i = 0;
    ObjectInfo* obj_info;
    for_each(obj_info, db->objects) {
        self.objects[i++] = object_new(obj_info);
    }

    /* --- Scene Loading --- */
    Scene* scene = scene_new(db->scene);
    self.current_scene = scene;

    /* --- */
    player_init(scene->player_init_pos, scene->player_init_rot);
    world_print();
}


void world_destroy() {
    player_destroy();

    Object* obj;
    for_each(obj, self.objects) {
        object_free(obj);
    }
    scene_free(self.current_scene);

    _world_free();
}


void world_print() {
    log_debug("total Object: %i", count_(self.objects));
    log_debug("total ObjectRef: %i", count_(self.current_scene->object_refs));
}


Object** world_get_objects() {
    return self.objects;
}


Object* world_find_object(char* id) {
    Object* obj = NULL;

    for_each(obj, self.objects) {
        if (strcmp(obj->base_id, id) == 0)
            return obj;
    }
    return NULL;
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
