#include <assert.h>
#include <stdlib.h>

#include <cglm/cglm.h>

#include "camera.h"
#include "cglm/vec3.h"
#include "cgm.h"
#include "gfx.h"
#include "log.h"
#include "model.h"
#include "texture.h"
#include "scene.h"


Object object_create(const char* id) {
    return (Object){
        .id = id,
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
        obj->textures = malloc(sizeof(GfxTexture*) * 8);
    }
    if (obj->textures_count > 8) {
        log_error("Max textures per model reached");
        exit(EXIT_FAILURE);
    }

    obj->textures[obj->textures_count] = texture_load_file(texture_path);
    obj->textures_count++;
}


Scene* scene_create() {
    Scene* scene = malloc(sizeof(Scene));
    scene->objects_count = 0;
    scene->_gfx_objects_count = 0;
    return scene;
}


void scene_destroy(Scene* scene) {
    free(scene);
}


#define _vec3__0 (vec3){0.0, 0.0, 0.0}
#define _vec3__1 (vec3){1.0, 1.0, 1.0}


void scene_add_object(Scene* scene, Object* obj, vec3 pos, vec3 rot, vec3 sc) {
    assert(scene->objects_count < __MAX_SCENE_OBJECTS);
    assert(obj->meshes_count == obj->textures_count);

    ObjectInst inst = {
        .obj=obj,
        .is_active=true
    };
    if (pos != NULL)  glm_vec3_copy(     pos, inst.pos);
    else              glm_vec3_copy(_vec3__0, inst.pos);

    if (rot != NULL)  glm_vec3_copy(     rot, inst.rot);
    else              glm_vec3_copy(_vec3__0, inst.rot);

    if (sc != NULL)  glm_vec3_copy(      sc, inst.sc);
    else              glm_vec3_copy(_vec3__1, inst.sc);
    
    mat4 m_model;
    cgm_model_mat(inst.pos, inst.rot, inst.sc, m_model);

    for (int i = 0; i < obj->meshes_count; i++) {
        GfxObject objg = {
            .mesh=obj->meshes[i],
            .texture=obj->textures[i]
        };
        glm_mat4_copy(m_model, objg.m_model);

        scene->_gfx_objects[scene->_gfx_objects_count] = objg;
        scene->_gfx_objects_count++;
    }

    scene->objects[scene->objects_count] = inst;
    scene->objects_count++;
}


void scene_draw(Scene* scene, Camera* camera) {
    gfx_draw(&camera->gfxd, scene->_gfx_objects, scene->_gfx_objects_count);
}
