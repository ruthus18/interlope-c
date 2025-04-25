#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <toml.h>

#include "objdb_loader.h"
#include "objdb.h"
#include "log.h"
#include "types.h"

static toml_table_t* load_toml_file(const char* toml_path) {
    FILE* fp;
    char errbuf[200];
    toml_table_t* db;

    fp = fopen(toml_path, "r");
    if (!fp) log_exit("Unable to read file: %s", toml_path);

    db = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!db) log_exit("Cannot parse TOML struct in: %s", toml_path);
    
    return db;
}

static u64 validate_db_size(toml_table_t* db) {
    extern const u64 _MAX_OBJECTS_DB_SIZE;
    u64 db_size = toml_table_ntab(db);
    
    if (db_size > _MAX_OBJECTS_DB_SIZE)
        log_exit("Objects db is too large to open! (size=%llu)", db_size);
        
    return db_size;
}

static const char* parse_model_path(toml_table_t* obj_record, const char* obj_id) {
    toml_datum_t _model_path = toml_string_in(obj_record, "model");
    
    if (!_model_path.ok) {
        log_exit("Error while parsing object [%s]: `model` kwarg not set", obj_id);
    }
    
    return _model_path.u.s;
}

static void parse_texture_paths(
    toml_table_t* obj_record, 
    const char* obj_id,
    const char* texture_paths[],
    char* allocated_texture_paths[],
    int* allocated_texture_count
) {
    toml_datum_t _texture_path = toml_string_in(obj_record, "texture");
    toml_array_t* _texture_paths = toml_array_in(obj_record, "textures");

    if (_texture_path.ok) {
        texture_paths[0] = _texture_path.u.s;
    }
    else if (_texture_paths) {
        for (int cnt = 0; ; cnt++) {
            _texture_path = toml_string_at(_texture_paths, cnt);
            if (!_texture_path.ok)
                break;

            // Store the allocated string pointer to free later
            texture_paths[cnt] = _texture_path.u.s;
            allocated_texture_paths[*allocated_texture_count] = _texture_path.u.s;
            (*allocated_texture_count)++;
        }
    }
    else
        log_exit("Error while parsing object [%s]: `texture` or `textures` kwargs not set", obj_id);
}

static ObjectType parse_object_type(toml_table_t* obj_record, char** obj_type_str) {
    ObjectType obj_type;
    
    toml_datum_t _obj_type = toml_string_in(obj_record, "type");
    if (!_obj_type.ok)
        obj_type = ObjectType_UNKNOWN;
    else {
        *obj_type_str = _obj_type.u.s; // Store to free later
        if (strcmp(*obj_type_str, "RIGID_BODY") == 0)
            obj_type = ObjectType_RIGID_BODY;
        // TODO: Add other types? Currently only RIGID_BODY is handled besides UNKNOWN.
    }
    
    return obj_type;
}

static void free_allocated_resources(
    const char* model_path,
    const char* texture_paths[],
    char* allocated_texture_paths[],
    int allocated_texture_count,
    char* obj_type_str
) {
    // Free the strings allocated by the TOML library
    free((void*)model_path); // Free the model path string
    
    if (texture_paths[0] && allocated_texture_count == 0) {
        // If single "texture" key was used, free that string
        free((void*)texture_paths[0]);
    } else {
        // If "textures" array was used, free all allocated strings from it
        for (int k = 0; k < allocated_texture_count; k++) {
            free(allocated_texture_paths[k]);
        }
    }
    
    if (obj_type_str) {
        free(obj_type_str); // Free the object type string if it was allocated
    }
}

static void process_object_record(
    ObjectsDB* out_db,
    toml_table_t* db,
    int index
) {
    const char* obj_id = toml_key_in(db, index);
    toml_table_t* obj_record = toml_table_in(db, obj_id);
    
    // Parse model path
    const char* model_path = parse_model_path(obj_record, obj_id);
    
    // Parse texture paths
    extern const u64 _MAX_OBJECT_TEXTURES;
    const char* texture_paths[32];
    char* allocated_texture_paths[32];
    
    // Initialize arrays to NULL
    for (int i = 0; i < 32; i++) {
        texture_paths[i] = NULL;
        allocated_texture_paths[i] = NULL;
    }
    int allocated_texture_count = 0;
    
    parse_texture_paths(obj_record, obj_id, texture_paths, allocated_texture_paths, &allocated_texture_count);
    
    // Parse object type
    char* obj_type_str = NULL; // Store pointer to free later
    ObjectType obj_type = parse_object_type(obj_record, &obj_type_str);
    
    // Create and add the object to the database
    out_db->objects[index] = objrec_create(obj_id, obj_type, model_path, texture_paths);
    out_db->objects_count++;
    
    // Free allocated resources
    free_allocated_resources(model_path, texture_paths, allocated_texture_paths, allocated_texture_count, obj_type_str);
}

ObjectsDB* objdb_load_toml(const char* toml_path) {
    // Load the TOML file
    toml_table_t* db = load_toml_file(toml_path);
    
    // Get and validate database size
    u64 db_size = validate_db_size(db);
    
    // Create the objects database
    ObjectsDB* out_db = malloc(sizeof(ObjectsDB));
    out_db->objects_count = 0;
    
    // Process each object record
    for (int i = 0; i < db_size; i++) {
        process_object_record(out_db, db, i);
    }
    
    log_info("Objects DB loaded, size: %llu", out_db->objects_count);
    toml_free(db);
    return out_db;
}