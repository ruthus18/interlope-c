#pragma once

typedef struct Mesh Mesh;
typedef struct Material Material;
typedef struct Texture Texture;
typedef struct AssetStorage AssetStorage;


void assets_init();
void assets_exit();
void assets_read_meshes();
void assets_read_textures();
void assets_read_materials();

