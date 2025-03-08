#include "uthash.h"

#include "assets.h"
#include "types.h"
#include "gfx.h"


typedef struct Mesh {
    UT_hash_handle hh;
    char id[32];

    GfxMesh** slots;
    u16 slots_size;
} Mesh;


typedef struct Material {
    UT_hash_handle hh;
    char id[32];

    GfxTexture* texture_diffuse;
    // (+) Lighting params
} Material;


typedef struct AssetStorage {
    Mesh* meshes;
    Texture* textures;
    Material* materials;
} AssetStorage;


AssetStorage assets;


void assets_init() {
    assets.meshes = NULL;
    assets.textures = NULL;
    assets.materials = NULL;
}


void assets_destroy() {
    
}


void assets_read_meshes() {

}