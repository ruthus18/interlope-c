#include <GL/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "texture.h"

#include "core/log.h"
#include "platform/file.h"


GfxTexture* texture_load_dds(const char* texture_relpath) {
	const char* path;
	FILE* f;
	
	with_path_to_texture(path, texture_relpath, {
		if((f = fopen(path, "rb")) == 0)
		return 0;
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
	
	unsigned int height = (header[12]) | (header[13] << 8) | (header[14] << 16) | (header[15] << 24);
	unsigned int width = (header[16]) | (header[17] << 8) | (header[18] << 16) | (header[19] << 24);
	unsigned int mipmap_cnt = (header[28]) | (header[29] << 8) | (header[30] << 16) | (header[31] << 24);
	
	unsigned int format;
    unsigned int block_size;
	
	if(header[84] == 'D') {
		switch(header[87]) {
			case '1': // DXT1
				format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				block_size = 8;
				break;
			case '3': // DXT3
				format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				block_size = 16;
				break;
			case '5': // DXT5
				format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				block_size = 16;
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
	
    /* -- Read and Load Texture -- */
    u8* buffer = malloc(file_size - 128);
    fread(buffer, 1, file_size - 128, f);
	
    GfxTexture* tex = gfx_texture_load_dds(buffer, width, height, format, mipmap_cnt, block_size);
	
    free(buffer);
	fclose(f);
	return tex;
}


GfxTexture* texture_load_png(const char* texture_relpath) {
	u32 width = 0;
	u32 height = 0;
	int nr_channels = 0;
	const char* path;

	GfxTexture* texture;

	with_path_to_texture(path, texture_relpath, {
		u8* data = stbi_load(path, (i32*)&width, (i32*)&height, &nr_channels, 0);
		if (data == NULL) {	
			log_error(stbi_failure_reason());	
			log_exit("Cannot open texture file: %s", texture_relpath);
			return texture;
		}
	
		int gl_format = GL_RGBA;
		texture = gfx_texture_load(data, width, height, gl_format);
	
		stbi_image_free(data);
	});

	return texture;
}
