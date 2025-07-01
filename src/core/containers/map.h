#pragma once

#include <string.h>
#include <stdlib.h>
#include "tommyds/tommyhashlin.h"

#include "core/types.h"


typedef tommy_hashlin map;
#define map(type)  map*

typedef struct {
    void* data;
    void* key;
    tommy_node node;
} _map_item;

typedef struct {
    map* mp;
    tommy_size_t pos;
    tommy_hashlin_node* node;
} _map_iter;

map* map_new(map_hash_type);
void map_free(map*);
void map_set(map*, void* data, void* key);
void* map_get(map*, void* key);
void map_remove(map*, void* key);
u64 map_size(map*);

_map_iter _map_iter_begin(map*);
void* _map_iter_next(_map_iter*);

#define map_for_each(data, mp) \
    for (_map_iter _it = _map_iter_begin(mp); data = _map_iter_next(&_it); )
