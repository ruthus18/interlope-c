#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>
#include <nuklear_glfw_gl4.h>

#include "platform/window.h"


#define EDITOR_MAX_VERTEX_BUFFER  512 * 1024
#define EDITOR_MAX_ELEMENT_BUFFER 128 * 1024


static struct nk_context* ctx;
struct nk_font_atlas *atlas;
struct nk_image img;
struct nk_colorf bg;


/* https://github.com/Immediate-Mode-UI/Nuklear/blob/master/demo/glfw_opengl4/main.c */


void editor_init() {
    ctx = nk_glfw3_init(
        window_get(),
        NK_GLFW3_INSTALL_CALLBACKS,
        EDITOR_MAX_VERTEX_BUFFER,
        EDITOR_MAX_ELEMENT_BUFFER
    );

    nk_glfw3_font_stash_begin(&atlas);
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
    nk_glfw3_font_stash_end();
    /*nk_style_set_font(ctx, &droid->handle);*/

    int tex_index = 0;
    int tex_width = 256;
    int tex_height = 256;

    char pixels[tex_width * tex_height * 4];
    memset(pixels, 128, sizeof(pixels));

    tex_index = nk_glfw3_create_texture(pixels, tex_width, tex_height);
    img = nk_image_id(tex_index);

    bg.r = 0.10, bg.g = 0.18, bg.b = 0.24, bg.a = 1.0;
}


void editor_update() {
    nk_glfw3_new_frame();

    // Draw...

    // nk_end(ctx);

    if (
        nk_begin(
            ctx,
            "Texture",
            nk_rect(10, 10, 230, 250),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE
        )
    ) {
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
        struct nk_rect total_space = nk_window_get_content_region(ctx);
        nk_draw_image(canvas, total_space, &img, nk_white);
    }
    nk_end(ctx);

    nk_glfw3_render(NK_ANTI_ALIASING_OFF);
}


void editor_destroy() {
    nk_glfw3_shutdown();
}