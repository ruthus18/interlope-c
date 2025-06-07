#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT

#include <nuklear.h>
#include <nuklear_glfw_gl4.h>

#include "../core/types.h"
#include "../platform/window.h"
#include "../world/scene.h"
#include "../world/world.h"


#define EDITOR_MAX_VERTEX_BUFFER  512 * 1024
#define EDITOR_MAX_ELEMENT_BUFFER 128 * 1024


typedef struct ObjectPanel {
    char pos_val[3][32];
    int pos_len[3];
    char rot_val[3][32];
    int rot_len[3];
} ObjectPanel;


static struct _Editor {
    struct nk_context* ctx;
    struct nk_font_atlas *font_atlas;

    Scene* scene;
    i32 selected_obj_id;
    ObjectPanel object_panel;

    char* show_physics_label;
    bool show_physics;
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

    self.ctx->style.window.spacing = nk_vec2(5,5);
    self.ctx->style.window.padding = nk_vec2(5,5);

    self.show_physics = false;
    self.show_physics_label = "[ ] Physics";

    // --- Set Scene ---
    self.scene = world_get_current_scene();
    self.selected_obj_id = -1;
}


void editor_destroy() {
    nk_glfw3_shutdown();
}


void editor_switch_view_physics() {
    self.show_physics = !self.show_physics;

    if (self.show_physics) {
        self.show_physics_label = "[X] Physics";
    }
    else {
        self.show_physics_label = "[ ] Physics";
    }
}


/* ------ UI Logic ------ */


static inline
void _update_objpan_data() {
    Object* selected_obj = scene_get_object(self.scene, self.selected_obj_id);
    vec3 pos, rot;

    object_get_position(selected_obj, pos);
    object_get_rotation(selected_obj, rot);

    ObjectPanel* pan = &self.object_panel;

    sprintf(pan->pos_val[0], "%.2f", pos[0]);
    sprintf(pan->pos_val[1], "%.2f", pos[1]);
    sprintf(pan->pos_val[2], "%.2f", pos[2]);

    pan->pos_len[0] = snprintf(NULL, 0, "%.2f", pos[0]);
    pan->pos_len[1] = snprintf(NULL, 0, "%.2f", pos[1]);
    pan->pos_len[2] = snprintf(NULL, 0, "%.2f", pos[2]);
    
    sprintf(pan->rot_val[0], "%.1f", rot[0]);
    sprintf(pan->rot_val[1], "%.1f", rot[1]);
    sprintf(pan->rot_val[2], "%.1f", rot[2]);

    pan->rot_len[0] = snprintf(NULL, 0, "%.1f", rot[0]);
    pan->rot_len[1] = snprintf(NULL, 0, "%.1f", rot[1]);
    pan->rot_len[2] = snprintf(NULL, 0, "%.1f", rot[2]);    
}


static inline
void _draw_scene_panel() {
    if (self.scene == NULL) {
        nk_label(self.ctx, "ERR: NO SCENE\0", NK_TEXT_LEFT);
        return;
    }
    
    /* Scene Tree */
    for (int i = 0; i < scene_get_objects_count(self.scene); i++) {
        nk_layout_row_static(self.ctx, 12, 275, 1);
        
        bool select_cond = (self.selected_obj_id == i);
        Object* obj = scene_get_object(self.scene, i);
        
        if (nk_select_label(self.ctx, object_get_base_id(obj), NK_TEXT_LEFT, select_cond)) {
            if (self.selected_obj_id != i) {
                self.selected_obj_id = i;
                scene_set_selected_object(self.scene, obj);
                _update_objpan_data();
            }
        }
    }
}


static inline
void _draw_object_panel() {
    if (self.selected_obj_id == -1)  return;
    
    ObjectPanel* pan = &self.object_panel;
    Object* selected_obj = scene_get_object(self.scene, self.selected_obj_id);
    nk_flags res;
    
    /* ------ ID ------ */
    nk_layout_row(self.ctx, NK_STATIC, 10, 2, (float[]){70, 190});
    nk_label(self.ctx, "Base ID: ", NK_TEXT_LEFT);
    nk_label(self.ctx, object_get_base_id(selected_obj), NK_TEXT_LEFT);

    /* ------ Type ------ */
    nk_layout_row(self.ctx, NK_STATIC, 10, 2, (float[]){70, 190});
    nk_label(self.ctx, "Type: ", NK_TEXT_LEFT);
    nk_label(self.ctx, object_get_type_string(selected_obj), NK_TEXT_LEFT);
    
    // Empty row
    nk_layout_row_static(self.ctx, 10, 275, 1);

    /* ------ Position Input ------ */
    
    nk_layout_row(self.ctx, NK_STATIC, 20, 4, (float[]){50, 70, 70, 70});
    nk_label(self.ctx, "Pos", NK_TEXT_LEFT);
    
    res = nk_edit_string(
        self.ctx, NK_EDIT_SIMPLE | NK_EDIT_SIG_ENTER, pan->pos_val[0], &pan->pos_len[0], 32, nk_filter_float
    );
    if (res & NK_EDIT_COMMITED) {
        object_set_position(selected_obj, (vec3) {
            atof(pan->pos_val[0]),
            atof(pan->pos_val[1]),
            atof(pan->pos_val[2]),
        });
    }

    res = nk_edit_string(
        self.ctx, NK_EDIT_SIMPLE | NK_EDIT_SIG_ENTER, pan->pos_val[1], &pan->pos_len[1], 32, nk_filter_float
    );
    if (res & NK_EDIT_COMMITED) {
        object_set_position(selected_obj, (vec3) {
            atof(pan->pos_val[0]),
            atof(pan->pos_val[1]),
            atof(pan->pos_val[2]),
        });
    }

    res = nk_edit_string(
        self.ctx, NK_EDIT_SIMPLE | NK_EDIT_SIG_ENTER, pan->pos_val[2], &pan->pos_len[2], 32, nk_filter_float
    );
    if (res & NK_EDIT_COMMITED) {
        object_set_position(selected_obj, (vec3) {
            atof(pan->pos_val[0]),
            atof(pan->pos_val[1]),
            atof(pan->pos_val[2]),
        });
    }

    /* ------ Rotation Input ------ */

    nk_layout_row(self.ctx, NK_STATIC, 20, 4, (float[]){50, 70, 70, 70});
    nk_label(self.ctx, "Rot", NK_TEXT_LEFT);

    res = nk_edit_string(
        self.ctx, NK_EDIT_SIMPLE | NK_EDIT_SIG_ENTER, pan->rot_val[0], &pan->rot_len[0], 32, nk_filter_float
    );
    if (res & NK_EDIT_COMMITED) {
        object_set_rotation(selected_obj, (vec3) {
            atof(pan->rot_val[0]),
            atof(pan->rot_val[1]),
            atof(pan->rot_val[2]),
        });
    }

    res = nk_edit_string(
        self.ctx, NK_EDIT_SIMPLE | NK_EDIT_SIG_ENTER, pan->rot_val[1], &pan->rot_len[1], 32, nk_filter_float
    );
    if (res & NK_EDIT_COMMITED) {
        object_set_rotation(selected_obj, (vec3) {
            atof(pan->rot_val[0]),
            atof(pan->rot_val[1]),
            atof(pan->rot_val[2]),
        });
    }

    res = nk_edit_string(
        self.ctx, NK_EDIT_SIMPLE | NK_EDIT_SIG_ENTER, pan->rot_val[2], &pan->rot_len[2], 32, nk_filter_float
    );
    if (res & NK_EDIT_COMMITED) {
        object_set_rotation(selected_obj, (vec3) {
            atof(pan->rot_val[0]),
            atof(pan->rot_val[1]),
            atof(pan->rot_val[2]),
        });
    }
}


void _draw_menu_bar() {
    struct nk_context* ctx = self.ctx;

    nk_menubar_begin(ctx);
    nk_layout_row_begin(ctx, NK_STATIC, 15, 3);

    nk_layout_row_push(ctx, 60);
    if (nk_menu_begin_label(ctx, "Scene", NK_TEXT_LEFT, nk_vec2(200, 300))) {
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_menu_item_label(ctx, "Open...", NK_TEXT_LEFT)) {}
        if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT)) {}
        if (nk_menu_item_label(ctx, " ", NK_TEXT_LEFT)) {}

        nk_menu_end(ctx);
    }

    nk_layout_row_push(ctx, 60);
    if (nk_menu_begin_label(ctx, "View", NK_TEXT_LEFT, nk_vec2(200, 300))) {
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_menu_item_label(ctx, self.show_physics_label, NK_TEXT_LEFT)) {
            editor_switch_view_physics();
        }
        if (nk_menu_item_label(ctx, " ", NK_TEXT_LEFT)) {}
        if (nk_menu_item_label(ctx, " ", NK_TEXT_LEFT)) {}

        nk_menu_end(ctx);
    }
    
    nk_layout_row_push(ctx, 60);
    if (nk_menu_begin_label(ctx, "Assets", NK_TEXT_LEFT, nk_vec2(200, 300))) {
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_menu_item_label(ctx, "Open Viewer", NK_TEXT_LEFT)) {
            // TODO: swith gfx state to assets viewer
        }
        if (nk_menu_item_label(ctx, " ", NK_TEXT_LEFT)) {}
        if (nk_menu_item_label(ctx, " ", NK_TEXT_LEFT)) {}

        nk_menu_end(ctx);
    }

    nk_menubar_end(ctx);
}


// static void _draw_rendering_menu() {
//     struct nk_context* ctx = self.ctx;

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


/* ------ Editor Main Loop ------ */

static inline
void _editor_draw_ui() {
    nk_begin(
        self.ctx, "Scene",
        nk_rect(10, 40, 300, 390),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE
    );
    _draw_scene_panel();
    nk_end(self.ctx);

    nk_begin(
        self.ctx, "Object",
        nk_rect(10, 440, 300, 600),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE
    );
    _draw_object_panel();
    nk_end(self.ctx);

    nk_begin(
        self.ctx, "Menu",
        nk_rect(0, 0, 1680, 30),
        NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR
    );
    _draw_menu_bar();
    nk_end(self.ctx);
}

void editor_update(bool visible) {
    /* --- Nuklear draw --- */
    nk_glfw3_new_frame();

    if (visible) {
        _editor_draw_ui();
    }  
    nk_glfw3_render(NK_ANTI_ALIASING_OFF);

    /* --- Editor Geometry --- */
    // TODO check selected obj id and call editor geometry interface
}
