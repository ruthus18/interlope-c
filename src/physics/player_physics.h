#pragma once
#include "physics/physics.h"


void px_player_init(vec3 pos, vec3 rot, f32 width, f32 height);
void px_player_destroy();

bool px_player_get_grounded();
bool px_player_get_ceiled();
bool px_player_set_position(vec3 pos);

void px_player_translate(vec3 dest_pos);
void px_player_update();
