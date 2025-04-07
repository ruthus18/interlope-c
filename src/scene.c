#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>
#include <toml.h>

#include "gfx.h"
#include "log.h"
#include "object.h"
#include "objects_db.h"
#include "texture.h"
#include "scene.h"
#include "types.h"


constexpr u64 _MAX_SCENE_OBJECTS = 1024;


typedef struct Scene {
    Object** objects;
    u64 objects_count;
    // Maximum number of objects that can be stored without resizing
    u64 objects_capacity;

    Object* selected_object;
} Scene;


Scene* scene_create() {
    Scene* scene = malloc(sizeof(Scene));
    if (scene == NULL) {
        log_exit("Failed to allocate memory for Scene");
    }
    
    scene->objects_capacity = _MAX_SCENE_OBJECTS;
    scene->objects = malloc(sizeof(Object*) * scene->objects_capacity);
    if (scene->objects == NULL) {
        free(scene);
        log_exit("Failed to allocate memory for Scene objects array");
    }
    
    scene->objects_count = 0;
    scene->selected_object = NULL;
    return scene;
}


void scene_destroy(Scene* scene) {
    for (u64 i = 0; i < scene->objects_count; i++) {
        object_destroy(scene->objects[i]);
    }
    
    free(scene->objects);
    free(scene);
}


void scene_add_object(Scene* scene, ObjectRecord* objrec, vec3 pos, vec3 rot, vec3 sc) {
    if (scene->objects_count >= scene->objects_capacity) {
        log_error(
            "Cannot add object '%s': Maximum scene object limit (%llu) reached.",
            objrec->id, scene->objects_capacity
        );
        return;
    }
    
    Object* obj = object_create(objrec, pos, rot, sc);
    
    scene->objects[scene->objects_count] = obj;
    scene->objects_count++;
}


Scene* scene_read_toml(const char* toml_path, ObjectsDB* objdb) {
    /* ------ Read scene file ------ */
    FILE* fp;
    char errbuf[200];

    toml_table_t* toml_scene;
    u64 scene_size;

    fp = fopen(toml_path, "r");
    if (!fp)  log_exit("Unable to read file: %s", toml_path);

    toml_scene = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!toml_scene)  log_exit("Cannot parse TOML struct in: %s", toml_path);

    toml_array_t* objects = toml_array_in(toml_scene, "obj");

    if (!objects) log_exit("Invalid scene file: no objects found");

    scene_size = toml_array_nelem(objects);

    if (scene_size > _MAX_SCENE_OBJECTS)
        log_exit("Scene is too large to open! (size=%i)", scene_size);

    /* ------ Parse object records ------ */
    toml_datum_t _obj_id;
    toml_array_t* _pos;
    toml_array_t* _rot;

    const char* obj_id;
    vec3 pos;
    vec3 rot;

    Scene* scene = scene_create();

    for (int i = 0; i < scene_size; i++) {
        toml_table_t* obj_record = toml_table_at(objects, i);

        _obj_id = toml_string_in(obj_record, "id");
        if (_obj_id.ok) {
            obj_id = _obj_id.u.s;
        }
        else
            log_exit(
                "Error while parsing scene object record: `id` kwarg not set",
                _obj_id.u.s
            );

        char* obj_id_str = _obj_id.u.s;

        _pos = toml_array_in(obj_record, "pos");
        if (_pos) {
            if (toml_array_nelem(_pos) >= 3) {
                // Assuming elements are doubles
                pos[0] = toml_double_at(_pos, 0).u.d;
                pos[1] = toml_double_at(_pos, 1).u.d;
                pos[2] = toml_double_at(_pos, 2).u.d;
            } else {
                log_info(
                    "Scene object '%s' has 'pos' array with less than 3 elements. Using default.",
                    obj_id_str
                );
                glm_vec3_copy(VEC3__0, pos);
            }
        }
        else {
            glm_vec3_copy(VEC3__0, pos);
        }

        _rot = toml_array_in(obj_record, "rot");
        if (_rot) {
            if (toml_array_nelem(_rot) >= 3) {
                // Assuming elements are doubles
                rot[0] = toml_double_at(_rot, 0).u.d;
                rot[1] = toml_double_at(_rot, 1).u.d;
                rot[2] = toml_double_at(_rot, 2).u.d;
            } else {
                log_info(
                    "Scene object '%s' has 'rot' array with less than 3 elements. Using default.",
                    obj_id_str
                );
                glm_vec3_copy(VEC3__0, rot); // Use default
            }
        }
        else {
            glm_vec3_copy(VEC3__0, rot);
        }

        /* ------ */

        ObjectRecord* objrec = objdb_find(objdb, obj_id_str);
        if (objrec == NULL) {
            free(obj_id_str);
            log_exit("Unknown object reference: %s", obj_id_str);
        }

        scene_add_object(scene, objrec, pos, rot, NULL);

        free(obj_id_str);
    }

    /* ------ Cleanup ------ */
    log_info("Scene size: %i", scene_size);

    toml_free(toml_scene);
    return scene;
}


u64 scene_get_objects_count(Scene* scene) {
    return scene->objects_count;
}

Object* scene_get_object(Scene* scene, u64 idx) {
    if (idx >= scene->objects_count) {
        return NULL;
    }
    return scene->objects[idx];
}


Object* scene_find_object(Scene* scene, const char* base_id) {
    for (int i = 0; i < scene->objects_count ; i++) {
        if (strcmp(scene->objects[i]->base_id, base_id) == 0) {
            return scene->objects[i];
        }
    }
    return NULL;
}


void scene_set_selected_object(Scene* scene, Object* obj) {
    scene->selected_object = obj;
}


void scene_draw(Scene* scene) {
    gfx_begin_draw_objects();

    for (int i = 0; i < scene->objects_count; i++) {
        Object* obj = scene->objects[i];

        for (int j = 0; j < obj->slots_count; j++) {
            if (obj != scene->selected_object) {
                gfx_draw_object(obj->meshes[j], obj->textures[j], obj->m_models[j]);
            }
            else {
                gfx_draw_object_outlined(obj->meshes[j], obj->textures[j], obj->m_models[j]);
            }
        }
    }
    gfx_end_draw_objects();
}
