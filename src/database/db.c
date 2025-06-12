#include "./db.h"

#include <string.h>
#include "../core/config.h"
#include "../core/utils.h"
#include "./loader.h"


static Database self;


void db_init() {
    self.objects = db_load_objects_data(PATH_OBJECTS_DATA);
    count_to_null(self.objects, self.objects_count);

    self.scene = db_load_scene_data(PATH_INIT_SCENE_DATA);
}


void db_destroy() {
    for (u32 i = 0; i < self.objects_count; i++) {
        if (self.objects[i]->model) {
            free(self.objects[i]->model->textures);
            free(self.objects[i]->model);
        }
        if (self.objects[i]->physics) {
            free(self.objects[i]->physics);
        }
        free(self.objects[i]);
    }
    free(self.objects);
    
    if (self.scene->object_refs) {
        free(self.scene->object_refs);
    }
    free(self.scene);
}


Database* db_get() {
    return &self;
}


ObjectInfo* db_get_object_info(char* obj_id) {
    for (u32 i = 0; i < self.objects_count; i++) {
        if (strcmp(self.objects[i]->id, obj_id) == 0) {
            return self.objects[i];
        }
    }
    return NULL;
}
