#pragma once

#include <cgltf.h>

#include "assets/model.h"


typedef cgltf_data GLTF_Asset;


GLTF_Asset* gltf_open(char* path);
void gltf_close(GLTF_Asset* data);

int gltf_get_nodes_count(GLTF_Asset* data);
void gltf_load_model_nodes(GLTF_Asset* data, ModelNode** dest);
