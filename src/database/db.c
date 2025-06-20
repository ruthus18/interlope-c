#include "./db.h"

#include <string.h>
#include "../core/config.h"
#include "../core/utils.h"
#include "./loader.h"


static Database self;


void db_init() {
    self.objects = db_load_objects_data(PATH_OBJECTS_DATA);
    self.scene = db_load_scene_data(PATH_INIT_SCENE_DATA);
}


void db_destroy() {
    ObjectInfo* obj;
    for_each(obj, self.objects) {
        if (obj->model) {
            free(obj->model->textures);
            free(obj->model);
        }
        if (obj->physics) {
            free(obj->physics);
        }
        free(obj);
    }

    free(self.objects);  
    free(self.scene->object_refs);
    free(self.scene);
}


Database* db_get() {
    return &self;
}


ObjectInfo* db_find_object(char* id) {
    ObjectInfo* obj = NULL;

    for_each(obj, self.objects) {
        if (strcmp(obj->id, id) == 0)  return obj;
    }
    return NULL;
}
