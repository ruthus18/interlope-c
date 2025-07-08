#include "graphics/gfx.h"


GfxTexture* texture_load(const char* texture_path);
GfxSkybox* texture_load_skybox(
	char* path_x, char* path_nx,
	char* path_y, char* path_ny,
	char* path_z, char* path_nz 
);
