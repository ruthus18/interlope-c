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


static void _draw_rendering_menu() {

}


static void _draw_scene_menu() {

}


static bool is_rendering_menu_active;
static bool is_scene_menu_active;


void editor_update() {
    nk_glfw3_new_frame();

    is_rendering_menu_active = nk_begin(
        ctx, "Rendering",
        nk_rect(10, 10, 300, 200),
        NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE
    );
    if (is_rendering_menu_active)  _draw_rendering_menu();
    nk_end(ctx);

    is_scene_menu_active = nk_begin(
        ctx, "Scene",
        is_rendering_menu_active ? nk_rect(10, 220, 300, 200) : nk_rect(10, 50, 300, 200),
        NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE
    );
    if (is_scene_menu_active)  _draw_scene_menu();
    nk_end(ctx);

    nk_glfw3_render(NK_ANTI_ALIASING_OFF);
}


void editor_destroy() {
    nk_glfw3_shutdown();
}