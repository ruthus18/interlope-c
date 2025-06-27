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
    GfxUI* gfx_data;
    Font* fonts;

    struct {
        struct FPSComponent fps;
        struct InteractionComponent interaction;
    } components;
} self;


void ui_init() {
    font_load_default();
    self.gfx_data = gfx_load_ui_data();

    self.components.fps.enabled = false;
    glm_vec3_copy(COLOR_YELLOW, self.components.fps.color);

    self.components.interaction.enabled = false;
    glm_vec3_copy(COLOR_AMBER, self.components.interaction.color);
}

void ui_destroy() {
    gfx_unload_ui_data(self.gfx_data);
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
    gfx_draw_ui(comp->value, self.gfx_data, (vec2){0.97, 0.02}, comp->color);
}

static inline
void _draw_interaction_component() {
    InteractionComponent* comp = &self.components.interaction;
    if (!comp->enabled)  return;

    gfx_draw_ui("Money", self.gfx_data, (vec2){0.46, 0.95}, comp->color);
    gfx_draw_ui("[E]Take", self.gfx_data, (vec2){0.45, 0.92}, comp->color);
}

void ui_draw() {
    gfx_begin_draw_ui();

    _draw_interaction_component();
    _draw_fps();

    gfx_end_draw_ui();
}
