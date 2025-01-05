#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "assets.h"
#include "gfx.h"
#include "log.h"
#include "platform/file.h"


GfxMesh* mesh_load_obj(const char* mesh_name) {
    // float vtx_buf[] = {};
    // int ind_buf[] = {};
    // int vtx_count = 0;
    // int ind_count = 0;
    // bool cw = false;

    // return gfx_mesh_load("_test_id_", vtx_buf, ind_buf, vtx_count, ind_count, cw);
}


GfxMesh* mesh_load_gltf(const char* mesh_name) {
    float vtx_buf[] = {};
    int ind_buf[] = {};
    int vtx_count = 0;
    int ind_count = 0;
    bool cw = false;

    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result;

    const char* path = path_to_mesh(mesh_name);
    result = cgltf_parse_file(&options, path, &data);

    if (result == cgltf_result_success) {
        log_success("Asset loaded: %s", mesh_name);
    }
    else {
        if (result == cgltf_result_file_not_found) {
            log_error("Mesh not found: %s", mesh_name);
        }
        else {
            log_error("Unknown error while loading GLTF mesh; code=%i", result);
        }

        free((void*)path);
        return NULL;
    }

    cgltf_load_buffers(&options, data, path);
    free((void*)path);

    log_info("BUFFER VIEWS: %i", data->buffer_views_count);
    for (int i = 0; i < data->buffer_views_count; i++) {

        const cgltf_buffer_view* buf_view = &(data->buffer_views[i]);

        log_info("BUF[%i] SIZE: %i", i, buf_view->buffer->size);

        // log_info("BUFFER[%i] NAME: %s", i, buf_view->name);
        // log_info("BUFFER[%i] OFFSET: %i", i, buf_view->offset);
        // log_info("BUFFER[%i] SIZE: %i", i, buf_view->size);
        // log_info("BUFFER[%i] TYPE: %i", i, buf_view->type);
        log_info("---");
        
    }


    // // TODO: process all nodes
    // if (data->meshes_count != 1) {
    //     log_error("Mesh count for model > 1: %s", mesh_path);
    //     return;
    // }

    // data->scene->nodes;

    return NULL;
    // return gfx_mesh_load("_test_id_", vtx_buf, ind_buf, vtx_count, ind_count, cw);
}
