#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>
#include <toml.h>

#include "cglm/vec3.h"
#include "cgm.h"
#include "gfx.h"
#include "log.h"
#include "model.h"
#include "texture.h"
#include "scene.h"


/* ------------------------------------------------------------------------- */
/*      Object Base                                                          */
/* ------------------------------------------------------------------------- */

typedef struct ObjectBase {
    const char* id;
    GfxMesh** meshes;
    u64 meshes_count;
    GfxTexture** textures;
    u64 textures_count;
} ObjectBase;


static inline
ObjectBase objbase_create(const char* id) {
    char* id_ = malloc(sizeof(id));
    strcpy(id_, id);

    return (ObjectBase){
        .id = id_,
        .meshes = NULL,
        .textures = NULL,
        .meshes_count = 0,
        .textures_count = 0,
    };
}

static inline
void objbase_destroy(ObjectBase* obj) {
    if (obj->meshes != NULL) {
        for (int i = 0; i < obj->meshes_count; i++) {
            gfx_mesh_unload(obj->meshes[i]);
        }
        free(obj->meshes);
    }

    if (obj->textures != NULL) {
        for (int i = 0; i < obj->textures_count; i++) {
            gfx_texture_unload(obj->textures[i]);
        }
        free(obj->textures);
    }

    free((void*)obj->id);
}

static inline
void objbase_load_meshes(ObjectBase* obj, const char* meshes_path) {
    obj->meshes = model_load_file(meshes_path);

    int i = 0;
    while (1) {
        if (obj->meshes[i] == NULL)  break;
        i++;
    }
    obj->meshes_count = i;
}


constexpr u64 _MAX_OBJECT_TEXTURES = 8;

static inline
void objbase_load_texture(ObjectBase* obj, const char* texture_path) {
    if (obj->textures == NULL) {
        obj->textures = malloc(sizeof(GfxTexture*) * _MAX_OBJECT_TEXTURES);
    }
    if (obj->textures_count > _MAX_OBJECT_TEXTURES) {
        log_exit("Max textures per model reached");
    }

    obj->textures[obj->textures_count] = texture_load_file(texture_path);
    obj->textures_count++;
}


/* ------------------------------------------------------------------------- */
/*      Objects DB                                                           */
/* ------------------------------------------------------------------------- */

constexpr u64 _MAX_OBJECTS_DB_SIZE = 1024;

typedef struct ObjectsDB {
    ObjectBase objects[_MAX_OBJECTS_DB_SIZE];
    u64 objects_count;
} ObjectsDB;


ObjectsDB* objdb_create_from(const char* toml_path) {
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
        const char* texture_paths[8] = {NULL};

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

        /* --- Create and load object in `ObjectsDB` */

        out_db->objects[i] = objbase_create(obj_id);
        out_db->objects_count++;
        
        objbase_load_meshes(&out_db->objects[i], model_path);

        for (int j = 0; ; j++) {
            if (texture_paths[j] == NULL)  break;

            objbase_load_texture(&out_db->objects[i], texture_paths[j]);
        }

        if (out_db->objects[i].meshes_count != out_db->objects[i].textures_count) {
            log_error("Unable to load object: meshes_count != textures_count");
            log_error(
                "ID: %s, M: %i, T: %i",
                out_db->objects[i].id,
                out_db->objects[i].meshes_count,
                out_db->objects[i].textures_count
            );
        }
    }

    log_info("Objects DB size: %i", out_db->objects_count);
    toml_free(db);
    return out_db;
}


void objdb_destroy(ObjectsDB* objdb) {
    for (int i = 0; i < objdb->objects_count ; i++) {
        objbase_destroy(&objdb->objects[i]);
    }
    free(objdb);
}


static inline
ObjectBase* objdb_find(ObjectsDB* objdb, const char* base_id) {

    for (int j = 0; j < objdb->objects_count ; j++) {
        // TODO: hashmap
        if (strcmp(objdb->objects[j].id, base_id) == 0) {
            return &objdb->objects[j];
        }
    }
    return NULL;
}


/* ------------------------------------------------------------------------- */
/*      Object                                                               */
/* ------------------------------------------------------------------------- */

typedef struct Object {
    char* base_id;
    vec3 pos;
    vec3 rot;
    vec3 sc;
    bool is_active;

    GfxMesh** meshes;
    GfxTexture** textures;
    u16 slots_count;
    mat4 m_model;
} Object;


const char* object_get_base_id(Object* obj) {
    return (const char*) obj->base_id;
}

void object_get_position(Object* obj, vec3 dest) {
    glm_vec3_copy(obj->pos, dest);
}

void object_get_rotation(Object* obj, vec3 dest) {
    glm_vec3_copy(obj->rot, dest);
}

void object_set_position(Object* obj, vec3 new_pos) {
    glm_vec3_copy(new_pos, obj->pos);
    cgm_model_mat(obj->pos, obj->rot, obj->sc, obj->m_model);
}

void object_set_rotation(Object* obj, vec3 new_rot) {
    glm_vec3_copy(new_rot, obj->rot);
    cgm_model_mat(obj->pos, obj->rot, obj->sc, obj->m_model);
}


/* ------------------------------------------------------------------------- */
/*      Scene                                                                */
/* ------------------------------------------------------------------------- */

constexpr u64 _MAX_SCENE_OBJECTS = 1024;


typedef struct Scene {
    Object objects[_MAX_SCENE_OBJECTS];
    u64 objects_count;
} Scene;


Scene* scene_create() {
    Scene* scene = malloc(sizeof(Scene));
    scene->objects_count = 0;
    return scene;
}


void scene_destroy(Scene* scene) {
    free(scene);
}


#define VEC3__0 (vec3){0.0, 0.0, 0.0}
#define VEC3__1 (vec3){1.0, 1.0, 1.0}


void scene_add_object(Scene* scene, ObjectBase* objbase, vec3 pos, vec3 rot, vec3 sc) {
    assert(scene->objects_count < _MAX_SCENE_OBJECTS);

    Object obj = {
        .base_id = (char*) objbase->id,
        .is_active = true,
        .meshes = objbase->meshes,
        .textures = objbase->textures,
        .slots_count = objbase->meshes_count,
    };
    if (pos != NULL)  glm_vec3_copy(    pos, obj.pos);
    else              glm_vec3_copy(VEC3__0, obj.pos);

    if (rot != NULL)  glm_vec3_copy(    rot, obj.rot);
    else              glm_vec3_copy(VEC3__0, obj.rot);

    if (sc != NULL)   glm_vec3_copy(     sc, obj.sc);
    else              glm_vec3_copy(VEC3__1, obj.sc);
    
    cgm_model_mat(obj.pos, obj.rot, obj.sc, obj.m_model);

    scene->objects[scene->objects_count] = obj;
    scene->objects_count++;
}


Scene* scene_create_from(const char* toml_path, ObjectsDB* objdb) {
    /* ------ Read scene file ------ */
    FILE* fp;
    char errbuf[200];

    toml_table_t* scene;
    u64 scene_size;

    fp = fopen(toml_path, "r");
    if (!fp)  log_exit("Unable to read file: %s", toml_path);

    scene = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!scene)  log_exit("Cannot parse TOML struct in: %s", toml_path);

    toml_array_t* objects = toml_array_in(scene, "obj");

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

    Scene* out_scene = scene_create();

    for (int i = 0; i < scene_size; i++) {
        toml_table_t* obj_record = toml_table_at(objects, i);

        _obj_id = toml_string_in(obj_record, "id");
        if (_obj_id.ok) {
            obj_id = _obj_id.u.s;
        }
        else
            log_exit("Error while parsing scene object record: `id` kwarg not set", obj_id);

        _pos = toml_array_in(obj_record, "pos");
        if (_pos) {
            glm_vec3_copy((vec3){
                toml_double_at(_pos, 0).u.d,
                toml_double_at(_pos, 1).u.d,
                toml_double_at(_pos, 2).u.d
            }, pos);
        }
        else {
            glm_vec3_copy((vec3){0.0, 0.0, 0.0}, pos);
        }

        _rot = toml_array_in(obj_record, "rot");
        if (_rot) {
            glm_vec3_copy((vec3){
                toml_double_at(_rot, 0).u.d,
                toml_double_at(_rot, 1).u.d,
                toml_double_at(_rot, 2).u.d
            }, rot);
        }
        else {
            glm_vec3_copy((vec3){0.0, 0.0, 0.0}, rot);
        }

        /* ------ */

        ObjectBase* objbase = objdb_find(objdb, obj_id);
        if (objbase == NULL)
            log_exit("Unknown object reference: %s", obj_id);

        scene_add_object(out_scene, objbase, pos, rot, NULL);
    }
    
    /* ------ Cleanup ------ */
    log_info("Scene size: %i", scene_size);

    toml_free(scene);
    return out_scene;
}


void scene_draw(Scene* scene) {
    gfx_begin_draw_objects();

    for (int i = 0; i < scene->objects_count; i++) {
        Object* obj = &scene->objects[i];

        for (int j = 0; j < obj->slots_count; j++) {
            gfx_draw_object(obj->meshes[j], obj->textures[j], obj->m_model);
        }
    }
    gfx_end_draw_objects();
}


u64 scene_get_objects_count(Scene* scene) {
    return scene->objects_count;
}

Object* scene_get_object(Scene* scene, u64 idx) {
    return &scene->objects[idx];
}
