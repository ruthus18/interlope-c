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


/* ------ Object ------ */

Object object_create(const char* id) {
    char* id_ = malloc(sizeof(id));
    strcpy(id_, id);

    return (Object){
        .id = id_,
        .meshes = NULL,
        .textures = NULL,
        .meshes_count = 0,
        .textures_count = 0,
    };
}


void object_destroy(Object* obj) {
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

void object_load_meshes(Object* obj, const char* meshes_path) {
    obj->meshes = model_load_file(meshes_path);

    int i = 0;
    while (1) {
        if (obj->meshes[i] == NULL)  break;
        i++;
    }
    obj->meshes_count = i;
}

void object_load_texture(Object* obj, const char* texture_path) {
    if (obj->textures == NULL) {
        obj->textures = malloc(sizeof(GfxTexture*) * _MAX_OBJECT_TEXTURES);
    }
    if (obj->textures_count > _MAX_OBJECT_TEXTURES) {
        log_exit("Max textures per model reached");
    }

    obj->textures[obj->textures_count] = texture_load_file(texture_path);
    obj->textures_count++;
}


/* ------ Objects DB ------ */

ObjectsDB objdb_create_from(const char* toml_path) {
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

    ObjectsDB out_db = { .objects_count = 0 };

    for (int i = 0; i < db_size; i++) {
        obj_id = toml_key_in(db, i);
        obj_record = toml_table_in(db, obj_id);

        /* --- "model" kwarg --- */
        const char* model_path;

        _model_path = toml_string_in(obj_record, "model");
        if (!_model_path.ok) {
            log_error("Error while parsing object [%s]: `model` kwarg not set", obj_id);
            exit(EXIT_FAILURE);
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
        out_db.objects[i] = object_create(obj_id);
        out_db.objects_count++;
        
        object_load_meshes(&out_db.objects[i], model_path);

        for (int j = 0; ; j++) {
            if (texture_paths[j] == NULL)  break;

            object_load_texture(&out_db.objects[i], texture_paths[j]);
        }
    }
    
    /* ------ Cleanup ------ */
    log_info("Objects DB size: %i", out_db.objects_count);

    toml_free(db);
    return out_db;
}


void objdb_destroy(ObjectsDB* objdb) {
    for (int i = 0; i < objdb->objects_count ; i++) {
        object_destroy(&objdb->objects[i]);
    }
}


/* ------ Scene ------ */

Scene* scene_create() {
    Scene* scene = malloc(sizeof(Scene));
    scene->objects_count = 0;
    scene->gfxd.objects_count = 0;
    return scene;
}


void scene_destroy(Scene* scene) {
    free(scene);
}


#define _vec3__0 (vec3){0.0, 0.0, 0.0}
#define _vec3__1 (vec3){1.0, 1.0, 1.0}


void scene_add_object(Scene* scene, Object* obj, vec3 pos, vec3 rot, vec3 sc) {
    assert(scene->objects_count < _MAX_SCENE_OBJECTS);

    if (obj->meshes_count != obj->textures_count) {
        log_error("Unable to add object to scene");
        log_error("ID: %s, M: %i, T: %i", obj->id, obj->meshes_count, obj->textures_count);
    }

    ObjectInst inst = {
        .obj=obj,
        .is_active=true
    };
    if (pos != NULL)  glm_vec3_copy(     pos, inst.pos);
    else              glm_vec3_copy(_vec3__0, inst.pos);

    if (rot != NULL)  glm_vec3_copy(     rot, inst.rot);
    else              glm_vec3_copy(_vec3__0, inst.rot);

    if (sc != NULL)   glm_vec3_copy(      sc, inst.sc);
    else              glm_vec3_copy(_vec3__1, inst.sc);
    
    mat4 m_model;
    cgm_model_mat(inst.pos, inst.rot, inst.sc, m_model);

    for (int i = 0; i < obj->meshes_count; i++) {
        GfxObject objg = {
            .mesh=obj->meshes[i],
            .texture=obj->textures[i]
        };
        glm_mat4_copy(m_model, objg.m_model);

        scene->gfxd.objects[scene->gfxd.objects_count] = objg;
        scene->gfxd.objects_count++;
    }

    scene->objects[scene->objects_count] = inst;
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

    toml_array_t* instances = toml_array_in(scene, "obj");

    if (!instances) log_exit("Invalid scene file: no instances");

    scene_size = toml_array_nelem(instances);

    if (scene_size > _MAX_SCENE_OBJECTS)
        log_exit("Scene is too large to open! (size=%i)", scene_size);

    /* ------ Parse object instances ------ */
    toml_datum_t _obj_id;
    toml_array_t* _pos;
    toml_array_t* _rot;

    const char* obj_id;
    vec3 pos;
    vec3 rot;

    Scene* out_scene = scene_create();

    for (int i = 0; i < scene_size; i++) {
        toml_table_t* inst = toml_table_at(instances, i);

        _obj_id = toml_string_in(inst, "id");
        if (_obj_id.ok) {
            obj_id = _obj_id.u.s;
        }
        else
            log_exit("Error while parsing object instance: `id` kwarg not set", obj_id);

        _pos = toml_array_in(inst, "pos");
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

        _rot = toml_array_in(inst, "rot");
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

        Object* obj = NULL;
        for (int j = 0; j < objdb->objects_count ; j++) {
            if (strcmp(objdb->objects[j].id, obj_id) == 0) {
                obj = &objdb->objects[j];
                break;
            }
        }

        if (obj == NULL)  log_exit("Unknown object reference: %s", obj_id);

        scene_add_object(out_scene, obj, pos, rot, NULL);
    }
    
    /* ------ Cleanup ------ */
    log_info("Scene size: %i", scene_size);

    toml_free(scene);
    return out_scene;
}

void scene_set_camera(Scene* scene, Camera* camera) {
    scene->gfxd.camera = &camera->gfxd;
}
