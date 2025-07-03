#pragma once
#include <stdbool.h>


void ui_init();
void ui_destroy();

void ui_enable_fps(bool value);
void ui_enable_interaction(bool value);
void ui_set_interaction_text(char* value);

void ui_draw();
