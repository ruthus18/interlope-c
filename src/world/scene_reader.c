#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>
#include <toml.h>

#include "scene_reader.h"
#include "../core/log.h"
#include "../core/types.h"
#include "../world/objdb.h"
#include "../world/scene.h"


static toml_table_t* load_toml_file(const char* toml_path) {
    FILE* fp;
    char errbuf[200];
    toml_table_t* toml_scene;

    fp = fopen(toml_path, "r");
    if (!fp) log_exit("Unable to read file: %s", toml_path);

    toml_scene = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!toml_scene) log_exit("Cannot parse TOML struct in: %s", toml_path);
    
    return toml_scene;
}

static toml_array_t* get_objects_array(toml_table_t* toml_scene, u64* scene_size) {
    toml_array_t* objects = toml_array_in(toml_scene, "obj");
    
    if (!objects) log_exit("Invalid scene file: no objects found");
    
    *scene_size = toml_array_nelem(objects);
    
    return objects;
}

static void parse_position(toml_array_t* pos_array, vec3 position, const char* obj_id_str) {
    if (pos_array) {
        if (toml_array_nelem(pos_array) >= 3) {
            position[0] = toml_double_at(pos_array, 0).u.d;
            position[1] = toml_double_at(pos_array, 1).u.d;
            position[2] = toml_double_at(pos_array, 2).u.d;
        } else {
            log_info(
                "Scene object '%s' has 'pos' array with less than 3 elements. Using default.",
                obj_id_str
            );
            glm_vec3_copy(VEC3__0, position);
        }
    } else {
        glm_vec3_copy(VEC3__0, position);
    }
}

static void parse_rotation(toml_array_t* rot_array, vec3 rotation, const char* obj_id_str) {
    if (rot_array) {
        if (toml_array_nelem(rot_array) >= 3) {
            rotation[0] = toml_double_at(rot_array, 0).u.d;
            rotation[1] = toml_double_at(rot_array, 1).u.d;
            rotation[2] = toml_double_at(rot_array, 2).u.d;
        } else {
            log_info(
                "Scene object '%s' has 'rot' array with less than 3 elements. Using default.",
                obj_id_str
            );
            glm_vec3_copy(VEC3__0, rotation);
        }
    } else {
        glm_vec3_copy(VEC3__0, rotation);
    }
}

static void process_object_record(
    Scene* scene, 
    toml_table_t* obj_record, 
    ObjectsDB* objdb
) {
    toml_datum_t _obj_id = toml_string_in(obj_record, "id");
    
    if (!_obj_id.ok) {
        log_exit("Error while parsing scene object record: `id` kwarg not set");
    }
    
    char* obj_id_str = _obj_id.u.s;
    
    vec3 position, rotation;
    
    toml_array_t* pos_array = toml_array_in(obj_record, "pos");
    parse_position(pos_array, position, obj_id_str);
    
    toml_array_t* rot_array = toml_array_in(obj_record, "rot");
    parse_rotation(rot_array, rotation, obj_id_str);
    
    ObjectRecord* objrec = objdb_find(objdb, obj_id_str);
    if (objrec == NULL) {
        free(obj_id_str);
        log_exit("Unknown object reference: %s", obj_id_str);
    }
    
    scene_add_object(scene, objrec, position, rotation, NULL);
    
    free(obj_id_str);
}

Scene* scene_read_toml(const char* toml_path, ObjectsDB* objdb) {
    u64 scene_size;
    
    // Load the TOML file
    toml_table_t* toml_scene = load_toml_file(toml_path);
    
    // Get the objects array and size
    toml_array_t* objects = get_objects_array(toml_scene, &scene_size);
    
    // Make sure the scene isn't too large
    extern const u64 _MAX_SCENE_OBJECTS;
    if (scene_size > _MAX_SCENE_OBJECTS) {
        log_exit("Scene is too large to open! (size=%llu)", scene_size);
    }
    
    // Create the scene
    Scene* scene = scene_create();
    
    // Process each object in the scene
    for (int i = 0; i < scene_size; i++) {
        toml_table_t* obj_record = toml_table_at(objects, i);
        process_object_record(scene, obj_record, objdb);
    }
    
    log_info("Scene loaded, size: %llu", scene_size);
    
    toml_free(toml_scene);
    return scene;
}