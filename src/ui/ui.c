#include "ui.h"

#include "assets/font.h"
#include "core/colors.h"
#include "platform/time.h"
#include "render/gfx.h"


typedef struct FPSComponent {
    bool enabled;
    char value[8];
    vec3 color;
} FPSComponent;

typedef struct InteractionComponent {
    bool enabled;
    vec3 color;
} InteractionComponent;


static struct UI {
    GfxMesh2D* gfx_data;
    Font* fonts;

    struct {
        struct FPSComponent fps;
        struct InteractionComponent interaction;
    } components;
} self;


void ui_init() {
    font_load_default();
    self.gfx_data = gfx_load_mesh_2d();

    self.components.fps.enabled = false;
    glm_vec3_copy(COLOR_YELLOW, self.components.fps.color);

    self.components.interaction.enabled = false;
    glm_vec3_copy(COLOR_AMBER, self.components.interaction.color);
}

void ui_destroy() {
    gfx_unload_mesh_2d(self.gfx_data);
    font_unload_default();
}

void ui_enable_fps(bool value) {
    self.components.fps.enabled = value;
}

void ui_enable_interaction(bool value) {
    self.components.interaction.enabled = value;
}


/* ------------------------------------------------------------------------- */

static inline
void _draw_fps() {
    FPSComponent* comp = &self.components.fps;
    if (!comp->enabled)  return;
    
    sprintf(comp->value, "%d", time_get_fps());
    gfx_enqueue_ui_element(comp->value, self.gfx_data, (vec2){0.97, 0.02}, comp->color);
}

static inline
void _draw_interaction_component() {
    InteractionComponent* comp = &self.components.interaction;
    if (!comp->enabled)  return;

    gfx_enqueue_ui_element("Money", self.gfx_data, (vec2){0.46, 0.95}, comp->color);
    gfx_enqueue_ui_element("[E]Take", self.gfx_data, (vec2){0.45, 0.92}, comp->color);
}

void ui_draw() {
    _draw_interaction_component();
    _draw_fps();
}
