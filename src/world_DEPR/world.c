#include "./world.h"
#include "./objdb.h"
#include "./objdb_reader.h"
#include "./scene.h"
#include "./scene_reader.h"

#include "../core/config.h"
#include "../database/db.h"
#include "../gameplay/player.h"


static struct World {
    ObjectsDB* objdb;

    Object_* objects[1024];
    u32 objects_count;

    Scene_* current_scene;
} self;


void world_init() {
    Database* db = db_get();

    /* --- Objects Loading --- */
    for (int i = 0; i < db->objects_count; i++) {
        self.objects[i] = object_create_from_info_(db->objects[i]);
    }
    self.objects_count = db->objects_count;
    
    /* --- Scene Loading --- */
    self.current_scene = scene_create_();

    for (int i = 0; i < db->scene->object_refs_count; i++) {
        // scene_add_object_();
    }

    // self.objdb = objdb_read_toml(WORLD_OBJDB_PATH);
    // self.current_scene = scene_read_toml(WORLD_INIT_SCENE_PATH, self.objdb);



    player_init((vec3){0.0, 0.0, 0.0}, 0.0, 0.0);
}


void world_destroy() {
    player_destroy();

    for (int i = 0; i < self.objects_count; i++) {
        object_destroy(self.objects[i]);
    }

    scene_destroy(self.current_scene);
    // objdb_destroy(self.objdb);
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
