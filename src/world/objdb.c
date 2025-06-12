#include <string.h>
#include <toml.h>

#include "objdb.h"

#include "../assets/model.h"
#include "../assets/texture.h"
#include "../core/log.h"
#include "../core/types.h"


ObjectRecord objrec_create(
    const char* id,
    ObjectTypeD type,
    const char* model_path,
    const char** texture_paths,
    const PhysicsProperties* physics
) {
    ObjectRecord objrec = {
        .meshes = NULL,
        .textures = NULL,
        .meshes_count = 0,
        .textures_count = 0,
        .type = type,
        .physics = {
            .has_physics = false,
            .mass = 0.0f,
            .collision_size = {1.0f, 1.0f, 1.0f},
            .collision_type = "box"
        },
    };

    // Copy physics properties if provided
    if (physics != NULL) {
        objrec.physics = *physics;
    }

    strncpy(objrec.id, id, sizeof(objrec.id) - 1);
    objrec.id[sizeof(objrec.id) - 1] = '\0';

    objrec_load_model(&objrec, model_path);
    for (int j = 0; ; j++) {
        if (texture_paths[j] == NULL)  break;

        objrec_load_texture(&objrec, texture_paths[j]);
    }

    if (objrec.meshes_count != objrec.textures_count) {
        log_error("Unable to load object: meshes_count != textures_count");
        log_error(
            "ID: %s, M: %i, T: %i", objrec.id,
            objrec.meshes_count,
            objrec.textures_count
        );
    }

    return objrec;
}

void objrec_destroy(ObjectRecord* obj) {
    if (obj->meshes != NULL) {
        model_destroy(obj->__model); // FIXME
    }

    if (obj->textures != NULL) {
        for (int i = 0; i < obj->textures_count; i++) {
            gfx_texture_unload(obj->textures[i]);
        }
        free(obj->textures);
    }
}


void objrec_load_model(ObjectRecord* obj, const char* model_path) {
    Model* model = model_read(model_path);
    obj->meshes = model->meshes;
    obj->meshes_count = model->slots_count;
    obj->local_positions = model->local_positions;
    obj->local_rotations = model->local_rotations;
    obj->names = model->names;

    obj->__model = model;
}


static constexpr u64 MAX_OBJECT_TEXTURES = 8;

void objrec_load_texture(ObjectRecord* obj, const char* texture_path) {
    if (obj->textures == NULL) {
        obj->textures = malloc(sizeof(GfxTexture*) * MAX_OBJECT_TEXTURES);

        if (obj->textures == NULL) {
            log_exit("Failed to allocate memory for object textures (ID: %s)", obj->id);
        }
    }

    if (obj->textures_count > MAX_OBJECT_TEXTURES) {
        log_exit("Max textures per model reached");
    }

    obj->textures[obj->textures_count] = texture_load_file(texture_path);
    obj->textures_count++;
}


/* ========================================================================= */


ObjectsDB* objdb_create(void) {
    ObjectsDB* db = malloc(sizeof(ObjectsDB));
    if (db == NULL) {
        log_exit("Failed to allocate memory for ObjectsDB");
    }
    
    db->objects_count = 0;
    return db;
}


void objdb_destroy(ObjectsDB* objdb) {
    for (int i = 0; i < objdb->objects_count ; i++) {
        objrec_destroy(&objdb->objects[i]);
    }
    free(objdb);
}


ObjectRecord* objdb_find(ObjectsDB* objdb, const char* base_id) {
    for (int j = 0; j < objdb->objects_count ; j++) {
        // TODO: hashmap
        if (strcmp(objdb->objects[j].id, base_id) == 0) {
            return &objdb->objects[j];
        }
    }
    return NULL;
}
