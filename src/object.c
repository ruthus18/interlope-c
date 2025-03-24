#include <cglm/cglm.h>
#include <string.h>

#include "object.h"

#include "cgm.h"
#include "log.h"
#include "model.h"
#include "texture.h"


/* ------------------------------------------------------------------------- */
/*      Object Base                                                          */
/* ------------------------------------------------------------------------- */


ObjectBase objbase_create(const char* id) {
    ObjectBase objbase = {
        .meshes = NULL,
        .textures = NULL,
        .meshes_count = 0,
        .textures_count = 0,
    };
    strcpy(objbase.id, id);
    return objbase;
}


void objbase_load_meshes(ObjectBase* obj, const char* meshes_path) {
    Model* model = model_read(meshes_path);
    obj->meshes = model->meshes;
    obj->meshes_count = model->slots_count;
    obj->local_positions = model->local_positions;
    obj->local_rotations = model->local_rotations;
    obj->names = model->names;

    obj->__model = model;
}


constexpr u64 _MAX_OBJECT_TEXTURES = 8;

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

void objbase_destroy(ObjectBase* obj) {
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


/* ------------------------------------------------------------------------- */
/*      Object                                                               */
/* ------------------------------------------------------------------------- */


const char* object_get_base_id(Object* obj) {
    return (const char*) obj->base_id;
}


const char* object_get_type_string(Object* obj) {
    if (obj->type == ObjectType_UNKNOWN)
        return "UNKNOWN";

    if (obj->type == ObjectType_RIGID_BODY)
        return "RIGID_BODY";

    log_exit("Unmatched object type: %i", obj->type);
    return "???";
}


void object_get_position(Object* obj, vec3 dest) {
    glm_vec3_copy(obj->pos, dest);
}


void object_get_rotation(Object* obj, vec3 dest) {
    glm_vec3_copy(obj->rot, dest);
}


void object_update_model_mat(Object* obj) {
    vec3 result_pos;
    vec3 result_rot;

    for (int i = 0; i < obj->slots_count; i++) {
        glm_vec3_sub(obj->pos, obj->local_positions[i], result_pos);
        glm_vec3_add(obj->rot, obj->local_rotations[i], result_rot);
        cgm_model_mat(result_pos, result_rot, obj->sc, obj->m_models[i]);
    }
}


void object_set_position(Object* obj, vec3 new_pos) {
    glm_vec3_copy(new_pos, obj->pos);
    object_update_model_mat(obj);
}


void object_set_rotation(Object* obj, vec3 new_rot) {
    glm_vec3_copy(new_rot, obj->rot);
    object_update_model_mat(obj);
}


// TODO: refactoring
void object_set_subm_rotation(Object* obj, vec3 new_rot, u32 slot_idx) {
    u32 i = slot_idx;
    vec3 result_pos;
    vec3 result_rot;

    glm_vec3_copy(new_rot, obj->local_rotations[i]);
    
    glm_vec3_sub(obj->pos, obj->local_positions[i], result_pos);    
    glm_vec3_add(obj->rot, obj->local_rotations[i], result_rot);

    cgm_model_mat(result_pos, result_rot, obj->sc, obj->m_models[i]);
}
