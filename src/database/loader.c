#include <string.h>
#include <cJSON.h>

#include "./loader.h"
#include "../platform/file.h"

static
ObjectType _parse_object_type(const char* type_str) {
    if (strcmp(type_str, "STATIC") == 0) {
        return OBJECT_STATIC;
    }
    return OBJECT_NULL;
}

static
PhysicsShape _parse_physics_shape(const char* shape_str) {
    if (strcmp(shape_str, "BOX") == 0) {
        return PHSHAPE_BOX;
    }
    if (strcmp(shape_str, "AABB") == 0) {
        return PHSHAPE_AABB;
    }
    return PHSHAPE_NULL;
}

static
ModelInfo* _parse_model_info(cJSON* model_json) {
    if (!model_json) return NULL;
    
    ModelInfo* model = malloc(sizeof(ModelInfo));
    memset(model, 0, sizeof(ModelInfo));
    
    cJSON* mesh = cJSON_GetObjectItem(model_json, "mesh");
    if (mesh && cJSON_IsString(mesh)) {
        strncpy(model->mesh, mesh->valuestring, MAX_MESH_PATH_LENGTH - 1);
        model->mesh[MAX_MESH_PATH_LENGTH - 1] = '\0';
    }
    
    cJSON* textures = cJSON_GetObjectItem(model_json, "textures");
    if (textures && cJSON_IsArray(textures)) {
        int tex_count = cJSON_GetArraySize(textures);
        if (tex_count > MAX_TEXTURES) tex_count = MAX_TEXTURES;
        
        model->textures = malloc(sizeof(char[MAX_TEXTURE_PATH_LENGTH]) * tex_count);
        model->texture_count = tex_count;
        
        for (int i = 0; i < tex_count; i++) {
            cJSON* texture = cJSON_GetArrayItem(textures, i);
            if (cJSON_IsString(texture)) {
                strncpy(model->textures[i], texture->valuestring, MAX_TEXTURE_PATH_LENGTH - 1);
                model->textures[i][MAX_TEXTURE_PATH_LENGTH - 1] = '\0';
            }
        }
    }
    
    return model;
}

static
PhysicsInfo* _parse_physics_info(cJSON* physics_json) {
    if (!physics_json) return NULL;
    
    PhysicsInfo* physics = malloc(sizeof(PhysicsInfo));
    memset(physics, 0, sizeof(PhysicsInfo));
    
    cJSON* shape = cJSON_GetObjectItem(physics_json, "shape");
    if (shape && cJSON_IsString(shape)) {
        physics->shape = _parse_physics_shape(shape->valuestring);
    }
    
    cJSON* size = cJSON_GetObjectItem(physics_json, "size");
    if (size && cJSON_IsArray(size) && cJSON_GetArraySize(size) >= 3) {
        physics->size[0] = cJSON_GetArrayItem(size, 0)->valuedouble;
        physics->size[1] = cJSON_GetArrayItem(size, 1)->valuedouble;
        physics->size[2] = cJSON_GetArrayItem(size, 2)->valuedouble;
    }
    
    cJSON* pos = cJSON_GetObjectItem(physics_json, "pos");
    if (pos && cJSON_IsArray(pos) && cJSON_GetArraySize(pos) >= 3) {
        physics->pos[0] = cJSON_GetArrayItem(pos, 0)->valuedouble;
        physics->pos[1] = cJSON_GetArrayItem(pos, 1)->valuedouble;
        physics->pos[2] = cJSON_GetArrayItem(pos, 2)->valuedouble;
    }
    
    cJSON* mass = cJSON_GetObjectItem(physics_json, "mass");
    if (mass && cJSON_IsNumber(mass)) {
        physics->mass = mass->valuedouble;
    }
    
    return physics;
}

ObjectInfo** db_load_objects_data(char* path) {
    const char* json_content = NULL;
    ObjectInfo** objects = NULL;
    
    with_file_read(path, json_content, {
        cJSON* root = cJSON_Parse(json_content);
        if (!root) {
            printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
            return NULL;
        }
        
        int object_count = cJSON_GetArraySize(root);
        objects = malloc(sizeof(ObjectInfo*) * (object_count + 1));

        int i = 0;
        cJSON* object_json = NULL;
        cJSON_ArrayForEach(object_json, root) {
            ObjectInfo* obj = malloc(sizeof(ObjectInfo));
            memset(obj, 0, sizeof(ObjectInfo));
            
            // Set object ID from JSON key
            strncpy(obj->id, object_json->string, MAX_ID_LENGTH - 1);
            obj->id[MAX_ID_LENGTH - 1] = '\0';
            
            // Parse object type
            cJSON* type = cJSON_GetObjectItem(object_json, "type");
            if (type && cJSON_IsString(type)) {
                obj->type = _parse_object_type(type->valuestring);
            }
            
            // Parse model info
            cJSON* model = cJSON_GetObjectItem(object_json, "model");
            if (!model) {
                // Handle case where mesh/textures are at root level
                cJSON* mesh = cJSON_GetObjectItem(object_json, "mesh");
                cJSON* textures = cJSON_GetObjectItem(object_json, "textures");
                if (mesh || textures) {
                    obj->model = _parse_model_info(object_json);
                }
            } else {
                obj->model = _parse_model_info(model);
            }
            
            // Parse physics info
            cJSON* physics = cJSON_GetObjectItem(object_json, "physics");
            if (physics) {
                if (cJSON_IsArray(physics)) {
                    // Handle array case (take first element)
                    cJSON* first_physics = cJSON_GetArrayItem(physics, 0);
                    obj->physics = _parse_physics_info(first_physics);
                } else {
                    obj->physics = _parse_physics_info(physics);
                }
            }
            
            objects[i++] = obj;
        }
        
        objects[object_count] = NULL;
        cJSON_Delete(root);
    });
    
    return objects;
}


SceneInfo* db_load_scene_data(char* path) {
    const char* json_content = NULL;
    SceneInfo* scene = NULL;
    
    with_file_read(path, json_content, {
        cJSON* root = cJSON_Parse(json_content);
        if (!root) {
            printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
            return NULL;
        }
        
        scene = malloc(sizeof(SceneInfo));
        memset(scene, 0, sizeof(SceneInfo));
        
        // Extract scene name from path (remove .json extension)
        char* filename = strrchr(path, '/');

        if (filename)   filename++;  // Skip the '/'
        else            filename = path;
        
        char scene_name[MAX_ID_LENGTH];
        strncpy(scene_name, filename, MAX_ID_LENGTH - 1);
        scene_name[MAX_ID_LENGTH - 1] = '\0';
        char* dot = strrchr(scene_name, '.');
        if (dot) *dot = '\0'; // Remove extension
        strncpy(scene->id, scene_name, MAX_ID_LENGTH - 1);
        scene->id[MAX_ID_LENGTH - 1] = '\0';

        /* --- Player Init --- */
        cJSON* player_init = cJSON_GetObjectItem(root, "player_init");
        if (player_init) {
            cJSON* pos = cJSON_GetObjectItem(player_init, "pos");
            if (pos && cJSON_IsArray(pos) && cJSON_GetArraySize(pos) >= 3) {
                scene->player_init_pos[0] = cJSON_GetArrayItem(pos, 0)->valuedouble;
                scene->player_init_pos[1] = cJSON_GetArrayItem(pos, 1)->valuedouble;
                scene->player_init_pos[2] = cJSON_GetArrayItem(pos, 2)->valuedouble;
            }
            
            cJSON* rot = cJSON_GetObjectItem(player_init, "rot");
            if (rot && cJSON_IsArray(rot) && cJSON_GetArraySize(rot) >= 2) {
                scene->player_init_rot[0] = cJSON_GetArrayItem(rot, 0)->valuedouble;
                scene->player_init_rot[1] = cJSON_GetArrayItem(rot, 1)->valuedouble;
            }
        }
        
        /* --- Object Refs --- */
        cJSON* object_refs = cJSON_GetObjectItem(root, "object_refs");
        if (!object_refs) {
            printf("Warning: No object_refs found in scene JSON\n");
            scene->object_refs = malloc(sizeof(ObjectRefInfo*));
            scene->object_refs[0] = NULL;
            cJSON_Delete(root);
            return scene;
        }
        
        int object_count = cJSON_GetArraySize(object_refs);
        scene->object_refs = malloc(sizeof(ObjectRefInfo*) * (object_count + 1));

        int i = 0;
        cJSON* object_json = NULL;
        cJSON_ArrayForEach(object_json, object_refs) {
            ObjectRefInfo* obj_ref = malloc(sizeof(ObjectRefInfo));
            memset(obj_ref, 0, sizeof(ObjectRefInfo));

            // Parse object ID
            cJSON* id = cJSON_GetObjectItem(object_json, "id");
            if (id && cJSON_IsString(id)) {
                strncpy(obj_ref->id, id->valuestring, MAX_ID_LENGTH - 1);
                obj_ref->id[MAX_ID_LENGTH - 1] = '\0';
            }
            
            // Parse position
            cJSON* pos = cJSON_GetObjectItem(object_json, "pos");
            if (pos && cJSON_IsArray(pos) && cJSON_GetArraySize(pos) >= 3) {
                obj_ref->pos[0] = cJSON_GetArrayItem(pos, 0)->valuedouble;
                obj_ref->pos[1] = cJSON_GetArrayItem(pos, 1)->valuedouble;
                obj_ref->pos[2] = cJSON_GetArrayItem(pos, 2)->valuedouble;
            }
            
            // Parse rotation (optional, defaults to 0)
            cJSON* rot = cJSON_GetObjectItem(object_json, "rot");
            if (rot && cJSON_IsArray(rot) && cJSON_GetArraySize(rot) >= 3) {
                obj_ref->rot[0] = cJSON_GetArrayItem(rot, 0)->valuedouble;
                obj_ref->rot[1] = cJSON_GetArrayItem(rot, 1)->valuedouble;
                obj_ref->rot[2] = cJSON_GetArrayItem(rot, 2)->valuedouble;
            } else {
                // Default rotation to 0
                obj_ref->rot[0] = 0.0f;
                obj_ref->rot[1] = 0.0f;
                obj_ref->rot[2] = 0.0f;
            }
            
            scene->object_refs[i++] = obj_ref;
        }

        scene->object_refs[object_count] = NULL;
        cJSON_Delete(root);
    });
    
    return scene;
}
