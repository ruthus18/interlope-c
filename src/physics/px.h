#pragma once
#include <ode/ode.h>

#include "physics/px_object.h"

#include "core/types.h"

#define PHYSICS_GRAVITY -9.81

/* ------------------------------------------------------------------------- */

void px_init();
void px_destroy();
void px_update();

// FIXME
void px_add_object(PxObject* obj);
// void px_create_object(PxObject* obj);
void px_delete_object(PxObject* obj);

u32 px_next_id();
dWorldID px_get_world();
dSpaceID px_get_space();
