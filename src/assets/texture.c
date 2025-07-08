/*
	texture.c -- Textures Management

	* Only .dds format is supported
*/
#include <GL/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "texture.h"

#include "core/log.h"
#include "platform/file.h"


typedef struct {
	u8* data;
	u32 width;
	u32 height;

	u32 gl_format;
	u32 mipmap_cnt;
	u32 block_size;
} DDS_File;


static
void _open_dds_file(DDS_File* dds, const char* path) {
	const char* full_path;
	FILE* f;
	
	with_path_to_texture(full_path, path, {
		if((f = fopen(full_path, "rb")) == 0)
			log_exit("Unable to open texture file: %s", path);
	});

	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
    /* -- .dds Header Processing -- */
	u8* header = malloc(128);
	fread(header, 1, 128, f);
	
	if(memcmp(header, "DDS ", 4) != 0) {
		free(header);
	    fclose(f);
		log_exit("Error while loading .dds: invalid signature");
    }
	
	dds->height = (header[12]) | (header[13] << 8) | (header[14] << 16) | (header[15] << 24);
	dds->width = (header[16]) | (header[17] << 8) | (header[18] << 16) | (header[19] << 24);
	dds->mipmap_cnt = (header[28]) | (header[29] << 8) | (header[30] << 16) | (header[31] << 24);

	if(header[84] == 'D') {
		switch(header[87]) {
			case '1': // DXT1
				dds->gl_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				dds->block_size = 8;
				break;
			case '3': // DXT3
				dds->gl_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				dds->block_size = 16;
				break;
			case '5': // DXT5
				dds->gl_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				dds->block_size = 16;
				break;
			default:
				free(header);
				fclose(f);
				log_exit("Unsupported .dds type: only DXT1-5 are supported!");
		}
	} else {
		free(header);
	    fclose(f);
		log_exit("Unsupported .dds type: only DXT1-5 are supported!");
    }
    free(header);
	
    dds->data = malloc(file_size - 128);
    fread(dds->data, 1, file_size - 128, f);
	fclose(f);
}

static
void _close_dds_file(DDS_File* dds) {
	free(dds->data);
}

GfxTexture* texture_load(const char* texture_path) {
	DDS_File dds;
	_open_dds_file(&dds, texture_path);

    GfxTexture* tex = gfx_load_texture(
		dds.data, dds.width, dds.height, dds.gl_format, dds.mipmap_cnt, dds.block_size
	);

    _close_dds_file(&dds);
	return tex;
}

GfxSkybox* texture_load_skybox(
	char* path_x, char* path_nx,
	char* path_y, char* path_ny,
	char* path_z, char* path_nz 
) {
	DDS_File dds_x, dds_nx, dds_y, dds_ny, dds_z, dds_nz;
	_open_dds_file(&dds_x, path_x);
	_open_dds_file(&dds_nx, path_nx);
	_open_dds_file(&dds_y, path_y);
	_open_dds_file(&dds_ny, path_ny);
	_open_dds_file(&dds_z, path_z);
	_open_dds_file(&dds_nz, path_nz);
	
	// TODO: check that w, h, format are same

	GfxSkybox* skybox = gfx_load_skybox(
		dds_x.data, dds_nx.data, dds_y.data, dds_ny.data, dds_z.data, dds_nz.data,
		dds_x.width, dds_x.height, dds_x.gl_format, dds_x.block_size
	);

	_close_dds_file(&dds_x);
	_close_dds_file(&dds_nx);
	_close_dds_file(&dds_y);
	_close_dds_file(&dds_ny);
	_close_dds_file(&dds_z);
	_close_dds_file(&dds_nz);

	return skybox;
}