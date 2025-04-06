#include <string.h>
#include <toml.h>

#include "objects_db.h"

#include "log.h"
#include "model.h"
#include "texture.h"
#include "types.h"


ObjectRecord objrec_create(
    const char* id,
    ObjectType type,
    const char* model_path,
    const char** texture_paths
) {
    ObjectRecord objrec = {
        .meshes = NULL,
        .textures = NULL,
        .meshes_count = 0,
        .textures_count = 0,
        .type = type,
    };
    strcpy(objrec.id, id);

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


constexpr u64 _MAX_OBJECT_TEXTURES = 8;

void objrec_load_texture(ObjectRecord* obj, const char* texture_path) {
    if (obj->textures == NULL) {
        obj->textures = malloc(sizeof(GfxTexture*) * _MAX_OBJECT_TEXTURES);
    }
    if (obj->textures_count > _MAX_OBJECT_TEXTURES) {
        log_exit("Max textures per model reached");
    }

    obj->textures[obj->textures_count] = texture_load_file(texture_path);
    obj->textures_count++;
}


/* ========================================================================= */


constexpr u64 _MAX_OBJECTS_DB_SIZE = 1024;

typedef struct ObjectsDB {
    ObjectRecord objects[_MAX_OBJECTS_DB_SIZE];
    u64 objects_count;
} ObjectsDB;


ObjectsDB* objdb_read_toml(const char* toml_path) {
    /* ------ Read database file ------ */
    FILE* fp;
    char errbuf[200];

    toml_table_t* db;
    u64 db_size;

    fp = fopen(toml_path, "r");
    if (!fp)  log_exit("Unable to read file: %s", toml_path);

    db = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!db)  log_exit("Cannot parse TOML struct in: %s", toml_path);

    db_size = toml_table_ntab(db);

    if (db_size > _MAX_OBJECTS_DB_SIZE)
        log_exit("Objects db is too large to open! (size=%i)", db_size);

    /* ------ Parse database records ------ */
    const char* obj_id;
    toml_table_t* obj_record;

    toml_datum_t _model_path;
    toml_datum_t _texture_path;
    toml_array_t* _texture_paths;
    toml_datum_t _obj_type;

    ObjectsDB* out_db = malloc(sizeof(ObjectsDB));
    out_db->objects_count = 0;

    for (int i = 0; i < db_size; i++) {
        obj_id = toml_key_in(db, i);
        obj_record = toml_table_in(db, obj_id);

        /* --- "model" kwarg --- */
        const char* model_path;

        _model_path = toml_string_in(obj_record, "model");
        if (!_model_path.ok) {
            log_exit("Error while parsing object [%s]: `model` kwarg not set", obj_id);
        }
        model_path = _model_path.u.s;
        
        /* --- "texture" and "textures" kwargs --- */

        // TODO: validate max texture paths
        const char* texture_paths[_MAX_OBJECT_TEXTURES + 1] = {NULL};
        
        _texture_path = toml_string_in(obj_record, "texture");
        _texture_paths = toml_array_in(obj_record, "textures");

        if (_texture_path.ok) {
            texture_paths[0] = _texture_path.u.s;
        }
        else if (_texture_paths) {
            for (int cnt = 0; ; cnt++) {
                _texture_path = toml_string_at(_texture_paths, cnt);
                if (!_texture_path.ok)
                    break;

                texture_paths[cnt] = _texture_path.u.s;
            }
        }
        else
            log_exit("Error while parsing object [%s]: `texture` or `textures` kwargs not set", obj_id);

        /* --- "type" kwarg --- */
        ObjectType obj_type;

        _obj_type = toml_string_in(obj_record, "type");
        if (!_obj_type.ok) 
            obj_type = ObjectType_UNKNOWN;

        else if (strcmp(_obj_type.u.s, "RIGID_BODY") == 0)
            obj_type = ObjectType_RIGID_BODY;

        /* --- Create and load object in `ObjectsDB` */

        out_db->objects[i] = objrec_create(obj_id, obj_type, model_path, texture_paths);
        out_db->objects_count++;
    }

    log_info("Objects DB size: %i", out_db->objects_count);
    toml_free(db);
    return out_db;
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