#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "model.h"
#include "gfx.h"
#include "log.h"
#include "platform/file.h"


GfxMesh** model_load_file(const char* model_relpath) {
    cgltf_result parse_res;
    cgltf_options options = {0};
    cgltf_data* data = NULL;

    const char* path = path_to_model(model_relpath);
    parse_res = cgltf_parse_file(&options, path, &data);

    if (parse_res != cgltf_result_success) {
        if (parse_res == cgltf_result_file_not_found) {
            log_error("Mesh not found: %s", model_relpath);
        }
        else {
            log_error("Unknown error while loading GLTF mesh: %i", parse_res);
        }

        free((void*)path);
        return NULL;
    }

    cgltf_load_buffers(&options, data, path);
    free((void*)path);

    /* -- Mesh Handling -- */
    // assert(data->nodes_count == 1);
    // const char* mesh_name = data->nodes[0].name;

    u32 _meshes_memsize = sizeof(GfxMesh*) * (data->meshes_count + 1);
    GfxMesh** meshes = malloc(_meshes_memsize);

    for (int m = 0; m < data->meshes_count; m++) {
        cgltf_mesh* mesh = &data->meshes[m];

        /* -- Primitive Handling -- */
        assert(mesh->primitives_count == 1);
        cgltf_primitive* pr = &mesh->primitives[0];
        
        cgltf_accessor* pos_attr;
        cgltf_accessor* normal_attr;
        cgltf_accessor* texcoord_attr = NULL;
        cgltf_accessor* ind_attr = pr->indices;
        
        for (int i = 0; i < pr->attributes_count; i++) {
            cgltf_attribute* attr = &pr->attributes[i];
            
            if (attr->type == cgltf_attribute_type_position) {
                pos_attr = attr->data;
            }
            
            else if (attr->type == cgltf_attribute_type_normal) {        
                normal_attr = attr->data;
            }
            
            else if (attr->type == cgltf_attribute_type_texcoord) {
                
                if (texcoord_attr != NULL) {
                    log_error("Multiple texcoord attrs!");
                    return NULL;
                }
                texcoord_attr = attr->data;
            }
            else if (attr->type == cgltf_attribute_type_color) {
                continue;
            }
            else {
                log_error("Unprocessed mesh attribute: %i", attr->type);
                return NULL;
            }
        }
        
        /* -- Buffers Handling --*/
        cgltf_buffer_view* pos_bufv = pos_attr->buffer_view;
        cgltf_buffer_view* normal_bufv = normal_attr->buffer_view;
        cgltf_buffer_view* texcoord_bufv = texcoord_attr->buffer_view;
        cgltf_buffer_view* ind_bufv = ind_attr->buffer_view;
        
        u64 vtx_count = pos_attr->count;
        f32* vtx_buf = malloc(pos_bufv->size + normal_bufv->size + texcoord_bufv->size);
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
        u32* ind_buf = malloc(2 * sizeof(u32) * ind_count);
        u16* ind_buf_tmp = ind_bufv->buffer->data + ind_bufv->offset;
        
        for (int i = 0; i < ind_count; i++) {
            ind_buf[i] = ind_buf_tmp[i];
        }
        
        GfxMesh* gfx_mesh = gfx_mesh_load(mesh->name, vtx_buf, ind_buf, vtx_count, ind_count, false);
        free(vtx_buf);
        free(ind_buf);
        
        meshes[m] = gfx_mesh;
    }

    meshes[data->meshes_count] = NULL;
    cgltf_free(data);
    return meshes;
}

