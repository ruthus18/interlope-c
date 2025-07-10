#pragma once
#include <stdbool.h>
#include <cglm/cglm.h>

#include "physics/px.h"
#include "physics/px_ray.h"


void px_player_init(vec3 pos, vec3 rot, f32 width, f32 height);
void px_player_destroy();

bool px_player_get_grounded();
bool px_player_get_ceiled();
PxRayTarget* px_player_get_interact_target();

void px_player_set_position(vec3 pos);
void px_player_set_interact_ray(vec3 pos, vec3 dir);

void px_player_translate(vec3 dest_pos);
void px_player_update();
