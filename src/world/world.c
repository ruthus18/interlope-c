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
    Scene* current_scene;
} self;


void world_init() {
    Database* db = db_get();

    

    self.objdb = objdb_read_toml(WORLD_OBJDB_PATH);
    self.current_scene = scene_read_toml(WORLD_INIT_SCENE_PATH, self.objdb);



    player_init((vec3){0.0, 0.0, 0.0}, 0.0, 0.0);
}


void world_destroy() {
    player_destroy();

    scene_destroy(self.current_scene);
    objdb_destroy(self.objdb);
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
