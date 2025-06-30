#include "map.h"
#include <string.h>
#include <stdlib.h>

#include "core/log.h"
#include "core/types.h"


map* map_new(map_hash_type htype) {
    map* mp = malloc(sizeof(map));
    tommy_hashdyn_init(mp);
    mp->htype = htype;

    return mp;
}

void _map_item_free(void* obj) { free(obj); }

void map_free(map* mp) {
    tommy_hashdyn_foreach(mp, _map_item_free);
    tommy_hashdyn_done(mp);
    free(mp);
}

/* ------------------------------------------------------------------------- */

int _compare_str(const void* arg, const void* item) {
    return strcmp(arg, ((_map_item*)item)->key);
}

int _compare_int(const void* arg, const void* item) {
    return *(const int*)arg - (intptr_t)((_map_item*)item)->key;
}

#define _hash_str(key)  tommy_strhash_u32(0, key)
#define _hash_int(key)  tommy_inthash_u32((intptr_t)(key))


void map_set(map* mp, void* data, void* key) {
    u32 hash = mp->htype == MHASH_STR ? _hash_str(key) : _hash_int(key);

    _map_item* item = malloc(sizeof(_map_item));
    item->key = key;
    item->data = data;

    tommy_hashdyn_insert(mp, &item->node, item, hash);
}

void* map_get(map* mp, void* key) {
    auto compare_f = mp->htype == MHASH_STR ? _compare_str : _compare_int;
    u32 hash = mp->htype == MHASH_STR ? _hash_str(key) : _hash_int(key);

    _map_item* item = tommy_hashdyn_search(mp, compare_f, key, hash);
    return item ? item->data : NULL;
}

/* ------------------------------------------------------------------------- */

_map_iter _map_iter_begin(map* mp) {
    _map_iter it;
    it.mp = mp;
    it.pos = 0;
    it.node = mp->bucket_max > 0 ? mp->bucket[0] : NULL;
    return it;
}

void* _map_iter_next(_map_iter* it) {
    while (it->pos < it->mp->bucket_max) {
        if (it->node) {
            _map_item* current_item = (_map_item*)it->node->data;

            it->node = it->node->next;
            return current_item->data;
        }
        it->pos++;
        if (it->pos < it->mp->bucket_max) {
            it->node = it->mp->bucket[it->pos];
        }
    }
    return NULL;
}
