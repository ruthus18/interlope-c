#pragma once
#include "../core/config.h"


const char* _file_read_text(const char* path);
const char* _path_construct(const char* base, const char* child);


#define with_file_read(path, content, inner) \
    content = _file_read_text(path); \
    inner \
    free((void*) content);

#define with_path_to_shader(path, rel_path, inner) \
    path = _path_construct(DIR_SHADERS, rel_path); \
    inner \
    free((void*) path);

#define with_path_to_mesh(path, rel_path, inner) \
    path = _path_construct(DIR_MESHES, rel_path); \
    inner \
    free((void*) path);

#define with_path_to_texture(path, rel_path, inner) \
    path = _path_construct(DIR_TEXTURES, rel_path); \
    inner \
    free((void*) path);
