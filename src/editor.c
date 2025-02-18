#include "log.h"
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

#include "scene.h"
#include "platform/window.h"


#define EDITOR_MAX_VERTEX_BUFFER  512 * 1024
#define EDITOR_MAX_ELEMENT_BUFFER 128 * 1024


static struct _Editor {
    struct nk_context* ctx;
    struct nk_font_atlas *font_atlas;

    Scene* scene;
} self;


/* https://github.com/Immediate-Mode-UI/Nuklear/blob/master/demo/glfw_opengl4/main.c */


void editor_init() {
    self.ctx = nk_glfw3_init(
        window_get(),
        NK_GLFW3_INSTALL_CALLBACKS,
        EDITOR_MAX_VERTEX_BUFFER,
        EDITOR_MAX_ELEMENT_BUFFER
    );

    nk_glfw3_font_stash_begin(&self.font_atlas);
    struct nk_font* default_font = nk_font_atlas_add_from_file(
        self.font_atlas, "assets/fonts/BerkeleyMono-Regular.ttf", 16, 0
    );
    nk_glfw3_font_stash_end();

    nk_style_set_font(self.ctx, &default_font->handle);

    self.ctx->style.window.spacing = nk_vec2(25,5);
    self.ctx->style.window.padding = nk_vec2(5,5);
}


void editor_set_scene(Scene* scene) {
    self.scene = scene;
}


int selected_obj_id = -1;

static
void _draw_scene_panel() {
    if (self.scene == NULL) {
        nk_label(self.ctx, "ERR: NO SCENE\0", NK_TEXT_LEFT);
        return;
    }

    nk_layout_row_static(self.ctx, 12, 275, 1);

    for (int item_id = 0; item_id < self.scene->objects_cnt; item_id++) {
        const char* obj_id = self.scene->objects[item_id].obj->id;

        if (nk_select_label(self.ctx, obj_id, NK_TEXT_LEFT, (selected_obj_id == item_id))) {
            selected_obj_id = item_id;
        }
    }
}


static inline
void _draw_object_panel() {
    if (selected_obj_id != -1) {
        nk_layout_row_static(self.ctx, 12, 275, 1);
        nk_label(self.ctx, "Name: Some Object", NK_TEXT_LEFT);
    }
}


void editor_update() {
    nk_glfw3_new_frame();

    nk_begin(
        self.ctx, "Scene",
        nk_rect(10, 10, 300, 400),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE
    );
    _draw_scene_panel();
    nk_end(self.ctx);
    
    nk_begin(
        self.ctx, "Object",
        nk_rect(10, 420, 300, 620),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE
    );
    _draw_object_panel();
    nk_end(self.ctx);

    nk_glfw3_render(NK_ANTI_ALIASING_OFF);
}


void editor_destroy() {
    nk_glfw3_shutdown();
}




// static void _draw_rendering_menu() {
//     /* menubar */
//         enum menu_states {MENU_DEFAULT, MENU_WINDOWS};
//         static nk_size mprog = 60;
//         static int mslider = 10;
//         static nk_bool mcheck = nk_true;
//         nk_menubar_begin(ctx);

//         /* menu #1 */
//         nk_layout_row_begin(ctx, NK_STATIC, 25, 5);
//         nk_layout_row_push(ctx, 45);
//         if (nk_menu_begin_label(ctx, "MENU", NK_TEXT_LEFT, nk_vec2(120, 200)))
//         {
//             static size_t prog = 40;
//             static int slider = 10;
//             static nk_bool check = nk_true;
//             nk_layout_row_dynamic(ctx, 25, 1);
//             if (nk_menu_item_label(ctx, "Hide", NK_TEXT_LEFT)) {}
//                 // show_menu = nk_false;
//             if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT)) {}
//                 // show_app_about = nk_true;
//             nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
//             nk_slider_int(ctx, 0, &slider, 16, 1);
//             nk_checkbox_label(ctx, "check", &check);
//             nk_menu_end(ctx);
//         }
//         /* menu #2 */
//         nk_layout_row_push(ctx, 60);
//         if (nk_menu_begin_label(ctx, "ADVANCED", NK_TEXT_LEFT, nk_vec2(200, 600)))
//         {
//             enum menu_state {MENU_NONE,MENU_FILE, MENU_EDIT,MENU_VIEW,MENU_CHART};
//             static enum menu_state menu_state = MENU_NONE;
//             enum nk_collapse_states state;

//             state = (menu_state == MENU_FILE) ? NK_MAXIMIZED: NK_MINIMIZED;
//             if (nk_tree_state_push(ctx, NK_TREE_TAB, "FILE", &state)) {
//                 menu_state = MENU_FILE;
//                 nk_menu_item_label(ctx, "New", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Close", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT);
//                 nk_tree_pop(ctx);
//             } else menu_state = (menu_state == MENU_FILE) ? MENU_NONE: menu_state;

//             state = (menu_state == MENU_EDIT) ? NK_MAXIMIZED: NK_MINIMIZED;
//             if (nk_tree_state_push(ctx, NK_TREE_TAB, "EDIT", &state)) {
//                 menu_state = MENU_EDIT;
//                 nk_menu_item_label(ctx, "Copy", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Delete", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Cut", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Paste", NK_TEXT_LEFT);
//                 nk_tree_pop(ctx);
//             } else menu_state = (menu_state == MENU_EDIT) ? MENU_NONE: menu_state;

//             state = (menu_state == MENU_VIEW) ? NK_MAXIMIZED: NK_MINIMIZED;
//             if (nk_tree_state_push(ctx, NK_TREE_TAB, "VIEW", &state)) {
//                 menu_state = MENU_VIEW;
//                 nk_menu_item_label(ctx, "About", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Options", NK_TEXT_LEFT);
//                 nk_menu_item_label(ctx, "Customize", NK_TEXT_LEFT);
//                 nk_tree_pop(ctx);
//             } else menu_state = (menu_state == MENU_VIEW) ? MENU_NONE: menu_state;

//             state = (menu_state == MENU_CHART) ? NK_MAXIMIZED: NK_MINIMIZED;
//             if (nk_tree_state_push(ctx, NK_TREE_TAB, "CHART", &state)) {
//                 size_t i = 0;
//                 const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f,12.0f,8.0f,22.0f,28.0f};
//                 menu_state = MENU_CHART;
//                 nk_layout_row_dynamic(ctx, 150, 1);
//                 nk_chart_begin(ctx, NK_CHART_COLUMN, NK_LEN(values), 0, 50);
//                 for (i = 0; i < NK_LEN(values); ++i)
//                     nk_chart_push(ctx, values[i]);
//                 nk_chart_end(ctx);
//                 nk_tree_pop(ctx);
//             } else menu_state = (menu_state == MENU_CHART) ? MENU_NONE: menu_state;
//             nk_menu_end(ctx);
//         }
//         /* menu widgets */
//         nk_layout_row_push(ctx, 70);
//         nk_progress(ctx, &mprog, 100, NK_MODIFIABLE);
//         nk_slider_int(ctx, 0, &mslider, 16, 1);
//         nk_checkbox_label(ctx, "check", &mcheck);
//         nk_menubar_end(ctx);
// }
