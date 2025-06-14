#include <stdlib.h>
#include <assert.h>

#define CGLTF_IMPLEMENTATION

#include "mesh_gltf.h"
#include "./limits.h"
#include "./model.h"

#include "../core/log.h"
#include "../platform/file.h"


GLTF_Asset* gltf_open(char* path) {
    cgltf_result parse_res;
    cgltf_options options = {0};
    cgltf_data* data = NULL;

    const char* full_path;
    with_path_to_mesh(full_path, path, {
        parse_res = cgltf_parse_file(&options, full_path, &data);
    
        if (parse_res != cgltf_result_success) {
            if (parse_res == cgltf_result_file_not_found) {
                log_error("Mesh not found: %s", path);
            }
            else {
                log_error("Unknown error while loading GLTF mesh: %i", parse_res);
            }
    
            return NULL;
        }
        cgltf_load_buffers(&options, data, full_path);
    });

    return data;
}


void gltf_close(GLTF_Asset* data) {
    cgltf_free(data);
}


int gltf_get_nodes_count(GLTF_Asset* data) {
    assert(data->nodes_count == data->meshes_count);
    return data->nodes_count;
}


void gltf_load_model_nodes(GLTF_Asset* data, ModelNode** dest) {
    for (int i = 0; i < gltf_get_nodes_count(data); i++) {
        cgltf_node node = data->nodes[i];

        if (node.name && strlen(node.name) >= MAX_MESH_NAME_LEN) {
            log_error(
                "GLTF node name '%s' is too long (>= %d chars). Truncating.",
                node.name,
                MAX_MESH_NAME_LEN
            );
            // Intentionally allow truncation instead of exiting
        }
        // Use strncpy for safety, ensure null termination
        if (node.name) {
             strncpy(dest[i]->name, node.name, MAX_MESH_NAME_LEN - 1);
             dest[i]->name[MAX_MESH_NAME_LEN - 1] = '\0';
        } else {
             // Handle case where node has no name
             dest[i]->name[0] = '\0';
        }

        if (node.has_translation) {
            glm_vec3_copy(node.translation, dest[i]->position);
        }
        else {
            glm_vec3_copy((vec3){0}, dest[i]->position);
        }

        if (node.has_rotation) {
            // Convert quaternion (node.rotation) to mat4, then to Euler angles (vec3)
            mat4 rot_matrix;
            glm_quat_mat4(node.rotation, rot_matrix);
            glm_euler_angles(rot_matrix, dest[i]->rotation);
        }
        else {
            glm_vec3_copy((vec3){0}, dest[i]->rotation);
        }

        cgltf_mesh* mesh = node.mesh;
        // Skip nodes without meshes
        if (!mesh) {
            log_info("Skipping node '%s' as it has no mesh.", node.name ? node.name : "[unnamed]");
            continue;
        }

        // Check primitive count - currently only support one primitive per mesh node
        if (mesh->primitives_count == 0) {
             log_info("Skipping mesh for node '%s' as it has no primitives.", node.name ? node.name : "[unnamed]");
             continue;
        }
        if (mesh->primitives_count > 1) {
            log_info("Mesh for node '%s' has %d primitives. Only processing the first one.", node.name ? node.name : "[unnamed]", mesh->primitives_count);
        }
        cgltf_primitive* pr = &mesh->primitives[0];
        
        cgltf_accessor* pos_attr;
        cgltf_accessor* normal_attr;
        cgltf_accessor* texcoord_attr = NULL;
        cgltf_accessor* ind_attr = pr->indices;
        
        for (int j = 0; j < pr->attributes_count; j++) {
            cgltf_attribute* attr = &pr->attributes[j];
            
            if (attr->type == cgltf_attribute_type_position) {
                pos_attr = attr->data;
            }
            
            else if (attr->type == cgltf_attribute_type_normal) {        
                normal_attr = attr->data;
            }
            
            else if (attr->type == cgltf_attribute_type_texcoord) {
                
                if (texcoord_attr != NULL) {
                    log_exit("Multiple texcoord attrs!");
                }
                texcoord_attr = attr->data;
            }
            else if (attr->type == cgltf_attribute_type_color) {
                continue;
            }
            else {
                log_exit("Unprocessed mesh attribute: %i", attr->type);
            }
        }

        // Check if essential attributes were found
        if (!pos_attr) {
             log_error("Mesh primitive for node '%s' is missing POSITION attribute. Skipping.", node.name ? node.name : "[unnamed]");
             continue;
        }
         if (!normal_attr) {
             log_error("Mesh primitive for node '%s' is missing NORMAL attribute. Skipping.", node.name ? node.name : "[unnamed]");
             continue;
        }
        // Check if indices exist (required by current loading logic)
        if (!ind_attr) {
             log_error("Mesh primitive for node '%s' is missing indices. Skipping.", node.name ? node.name : "[unnamed]");
             continue;
        }

        /* -- Buffers Handling --*/
        cgltf_buffer_view* pos_bufv = pos_attr->buffer_view;
        cgltf_buffer_view* normal_bufv = normal_attr->buffer_view;
        cgltf_buffer_view* texcoord_bufv = texcoord_attr->buffer_view;
        cgltf_buffer_view* ind_bufv = ind_attr->buffer_view;
        
        u64 vtx_count = pos_attr->count;
        f32* vtx_buf = malloc(pos_bufv->size + normal_bufv->size + texcoord_bufv->size);
        if (!vtx_buf) {
            log_error("Failed to allocate memory for vertex buffer (Node: %s)", node.name ? node.name : "[unnamed]");
            // TODO: Consider freeing model data allocated so far before returning NULL
            cgltf_free(data);
            exit(EXIT_FAILURE);
        }
        memcpy(
            vtx_buf,
            pos_bufv->buffer->data + pos_bufv->offset,
            pos_bufv->size
        );
        memcpy(
            (void*)vtx_buf + pos_bufv->size,
            normal_bufv->buffer->data + normal_bufv->offset,
            normal_bufv->size
        );
        memcpy(
            (void*)vtx_buf + pos_bufv->size + normal_bufv->size,
            texcoord_bufv->buffer->data + texcoord_bufv->offset,
            texcoord_bufv->size
        );
        
        u64 ind_count = ind_attr->count;
        // Allocate space for u32 indices
        u32* ind_buf = malloc(sizeof(u32) * ind_count);
        if (!ind_buf) {
            log_error("Failed to allocate memory for index buffer (Node: %s)", node.name ? node.name : "[unnamed]");
            free(vtx_buf); // Free previously allocated vertex buffer
            // TODO: Consider freeing model data allocated so far before returning NULL
            cgltf_free(data);
            exit(EXIT_FAILURE);
        }

        // Read indices based on their actual component type
        void* ind_data_ptr = (u8*)ind_bufv->buffer->data + ind_bufv->offset;
        
        for (u64 j = 0; j < ind_count; j++) {
            switch (ind_attr->component_type) {
                case cgltf_component_type_r_8u:
                    ind_buf[j] = ((u8*)ind_data_ptr)[j];
                    break;
                case cgltf_component_type_r_16u:
                    ind_buf[j] = ((u16*)ind_data_ptr)[j];
                    break;
                case cgltf_component_type_r_32u:
                    ind_buf[j] = ((u32*)ind_data_ptr)[j];
                    break;
                default:
                    log_error("Unsupported index component type: %d (Node: %s)", ind_attr->component_type, node.name ? node.name : "[unnamed]");
                    free(ind_buf);
                    free(vtx_buf);
                     // TODO: Consider freeing model data allocated so far before returning NULL
                    cgltf_free(data);
                    exit(EXIT_FAILURE);
            }
        }
        
        GfxMesh* gfx_mesh = gfx_mesh_load(mesh->name, vtx_buf, ind_buf, vtx_count, ind_count, false);
        free(vtx_buf);
        free(ind_buf);
        
        dest[i]->mesh = gfx_mesh;
    }
}
