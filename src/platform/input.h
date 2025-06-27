#pragma once
#include <cglm/cglm.h>

#include "platform/input_keys.h"


void input_init();
void input_destroy();
void input_update();

bool input_is_keyp(int key);
bool input_is_keyrp(int key);
void input_get_mouse_delta(vec2 dest);
bool cursor_is_visible();
void cursor_set_visible(bool visible);
