#pragma once
#include "uthash.h"

#define map_item_meta  UT_hash_handle hh
#define map_type(type)  type *

#define map(type)  map_type(type)

#define map_get(mp, value, dest) \
    HASH_FIND_STR(mp, value, dest)

#define map_set(mp, key, item) \
    HASH_ADD_STR(mp, key, item)

#define map_for_each(item, mp) \
    typeof(item) _tmp; \
    HASH_ITER(hh, mp, item, _tmp)

#define map_delete(mp, item) \
    HASH_DEL(mp, item)

#define map_size(mp) \
    HASH_COUNT(mp)
