#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>
#include <toml.h>

#include "cgm.h"
#include "gfx.h"
#include "log.h"
#include "object.h"
#include "objects_db.h"
#include "texture.h"
#include "scene.h"


constexpr u64 _MAX_SCENE_OBJECTS = 1024;


typedef struct Scene {
    Object objects[_MAX_SCENE_OBJECTS];
    u64 objects_count;

    Object* selected_object;
} Scene;


Scene* scene_create() {
    Scene* scene = malloc(sizeof(Scene));
    scene->objects_count = 0;
    scene->selected_object = NULL;
    return scene;
}


void scene_destroy(Scene* scene) {
    free(scene);
}


#define VEC3__0 (vec3){0.0, 0.0, 0.0}
#define VEC3__1 (vec3){1.0, 1.0, 1.0}


void scene_add_object(Scene* scene, ObjectRecord* objrec, vec3 pos, vec3 rot, vec3 sc) {
    assert(scene->objects_count < _MAX_SCENE_OBJECTS);
    
    Object obj = {
        .base_id = objrec->id,
        .type = objrec->type,
        .meshes = objrec->meshes,
        .textures = objrec->textures,
        .slots_count = objrec->meshes_count,
        .is_active = true,
        .local_positions = NULL,
        .local_rotations = NULL,
    };
    if (pos != NULL)  glm_vec3_copy(    pos, obj.pos);
    else              glm_vec3_copy(VEC3__0, obj.pos);
    
    if (rot != NULL)  glm_vec3_copy(    rot, obj.rot);
    else              glm_vec3_copy(VEC3__0, obj.rot);
    
    if (sc != NULL)   glm_vec3_copy(     sc, obj.sc);
    else              glm_vec3_copy(VEC3__1, obj.sc);

    // TODO: free
    obj.local_positions = malloc(sizeof(vec3) * obj.slots_count);
    obj.local_rotations = malloc(sizeof(vec3) * obj.slots_count);
    obj.m_models = malloc(sizeof(mat4) * obj.slots_count);
    
    // TODO: refactoring
    for (int i = 0; i < obj.slots_count; i++) {
        glm_vec3_copy(objrec->local_positions[i], obj.local_positions[i]);
        glm_vec3_copy(objrec->local_rotations[i], obj.local_rotations[i]);

        vec3 result_pos;
        glm_vec3_copy(obj.pos, result_pos);
        glm_vec3_sub(obj.pos, obj.local_positions[i], result_pos);
        
        vec3 result_rot;
        glm_vec3_copy(obj.rot, result_rot);
        glm_vec3_add(obj.rot, obj.local_rotations[i], result_rot);

        cgm_model_mat(result_pos, result_rot, obj.sc, obj.m_models[i]);
    }

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

        ObjectRecord* objrec = objdb_find(objdb, obj_id);
        if (objrec == NULL)
            log_exit("Unknown object reference: %s", obj_id);

        scene_add_object(scene, objrec, pos, rot, NULL);
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
    return &scene->objects[idx];
}


Object* scene_find_object(Scene* scene, const char* base_id) {
    for (int i = 0; i < scene->objects_count ; i++) {
        if (strcmp(scene->objects[i].base_id, base_id) == 0) {
            return &scene->objects[i];
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
        Object* obj = &scene->objects[i];

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
