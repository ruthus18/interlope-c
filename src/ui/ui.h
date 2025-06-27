#pragma once
#include <stdbool.h>


void ui_init();
void ui_cleanup();

void ui_enable_fps(bool value);
void ui_enable_interaction(bool value);

void ui_draw();
