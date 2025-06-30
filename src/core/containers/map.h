#pragma once

#include <string.h>
#include <stdlib.h>
#include "tommyds/tommyhashdyn.h"


typedef tommy_hashdyn map;
#define map(type)  map*

typedef struct {
    void* data;
    void* key;
    tommy_node node;
} _map_item;

typedef struct {
    map* mp;
    tommy_size_t pos;
    tommy_hashdyn_node* node;
} _map_iter;

map* map_new(map_hash_type);
void map_free(map*);
void map_set(map*, void* data, void* key);
void* map_get(map*, void* key);

_map_iter _map_iter_begin(map*);
void* _map_iter_next(_map_iter*);

#define map_for_each(data, mp) \
    for (_map_iter _it = _map_iter_begin(mp); data = _map_iter_next(&_it); )
