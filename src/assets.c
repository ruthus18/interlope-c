#include "uthash.h"

#include "assets.h"
#include "types.h"
#include "gfx.h"


typedef struct Mesh {
    UT_hash_handle hh;
    char id[32];

    GfxMesh* slots;
    u16 slots_size;
} AssetMesh;


typedef struct Material {
    UT_hash_handle hh;
    char id[32];

    GfxTexture* diffuse_slots;
    u16 slots_size;
} Material;


typedef struct Texture {
    UT_hash_handle hh;
    char id[32];

    GfxTexture* gfx;
} Texture;


typedef struct AssetStorage {
    Mesh* meshes;
    Texture* textures;
    Material* materials;
} AssetStorage;


AssetStorage assets;


void assets_init() {
    assets.meshes = NULL;
    assets.materials = NULL;
}


void assets_exit() {

}
