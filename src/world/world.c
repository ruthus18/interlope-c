#include <string.h>

#include "world.h"
#include "./limits.h"
#include "./object.h"
#include "./scene.h"

#include "../core/config.h"
#include "../core/log.h"
#include "../core/utils.h"
#include "../database/db.h"
#include "../gameplay/player.h"


static struct World {
    Object** objects;
    Scene* current_scene;
} self;


static inline
void _world_alloc() {
    int objects_size = sizeof(Object*) * (WORLD_MAX_OBJECTS + 1);

    self.objects = malloc(objects_size);
    memset(self.objects, 0, objects_size);
}

static inline
void _world_free() {
    free(self.objects);
}


void world_init() {
    _world_alloc();
    Database* db = db_get();

    /* --- Objects Loading --- */
    int i = 0;
    ObjectInfo* obj_info;
    for_each(obj_info, db->objects) {
        self.objects[i++] = object_create_from_info(obj_info);
    }

    /* --- Scene Loading --- */
    Scene* scene = scene_create_from_info(db->scene);
    self.current_scene = scene;

    /* --- */
    player_init(scene->player_init_pos, scene->player_init_rot);
}


void world_destroy() {
    player_destroy();
    
    Object* obj;
    for_each(obj, self.objects) {
        object_destroy(obj);
    }
    scene_destroy(self.current_scene);

    _world_free();
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
