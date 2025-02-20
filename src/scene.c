#include <assert.h>
#include <stdlib.h>

#include <cglm/cglm.h>

#include "camera.h"
#include "cglm/vec3.h"
#include "cgm.h"
#include "gfx.h"
#include "scene.h"


Scene* scene_create() {
    Scene* scene = malloc(sizeof(Scene));
    scene->objects_cnt = 0;
    return scene;
}


void scene_destroy(Scene* scene) {
    free(scene);
}


#define _vec3__0 (vec3){0.0, 0.0, 0.0}
#define _vec3__1 (vec3){1.0, 1.0, 1.0}


void scene_add_object(Scene* scene, Object* obj, vec3 pos, vec3 rot, vec3 sc) {
    assert(scene->objects_cnt < __MAX_SCENE_OBJECTS);

    ObjectPtr objp = {
        .obj=obj,
        .is_active=true
    };
    if (pos != NULL)  glm_vec3_copy(     pos, objp.pos);
    else              glm_vec3_copy(_vec3__0, objp.pos);

    if (rot != NULL)  glm_vec3_copy(     rot, objp.rot);
    else              glm_vec3_copy(_vec3__0, objp.rot);

    if (sc != NULL)  glm_vec3_copy(      sc, objp.sc);
    else              glm_vec3_copy(_vec3__1, objp.sc);

    GfxObject objg = {
        .mesh=obj->mesh,
        .texture=obj->texture
    };

    cgm_model_mat(objp.pos, objp.rot, objp.sc, objg.m_model);

    scene->objects[scene->objects_cnt] = objp;
    scene->gfx_objects[scene->objects_cnt] = objg;
    scene->objects_cnt++;
}


void scene_draw(Scene* scene, Camera* camera) {
    gfx_draw(&camera->gfxd, scene->gfx_objects, scene->objects_cnt);
}
