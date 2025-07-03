#include "map.h"
#include <string.h>
#include <stdlib.h>


map* map_new(map_hash_type htype) {
    map* mp = malloc(sizeof(map));
    tommy_hashlin_init(mp);
    mp->htype = htype;

    return mp;
}

void _map_item_free(void* obj) { free(obj); }

void map_free(map* mp) {
    tommy_hashlin_foreach(mp, _map_item_free);
    tommy_hashlin_done(mp);
    free(mp);
}

/* ------------------------------------------------------------------------- */

int _compare_str(const void* arg, const void* item) {
    return strcmp(arg, ((_map_item*)item)->key);
}

int _compare_int(const void* arg, const void* item) {
    return (intptr_t)arg - (intptr_t)((_map_item*)item)->key;
}

#define _hash_str(key)  tommy_strhash_u32(0, key)
#define _hash_int(key)  tommy_inthash_u32((intptr_t)(key))


void map_set(map* mp, void* data, void* key) {
    u32 hash = mp->htype == MHASH_STR ? _hash_str(key) : _hash_int(key);

    _map_item* item = malloc(sizeof(_map_item));
    item->key = key;
    item->data = data;

    tommy_hashlin_insert(mp, &item->node, item, hash);
}

void* map_get(map* mp, void* key) {
    auto compare_f = mp->htype == MHASH_STR ? _compare_str : _compare_int;
    u32 hash = mp->htype == MHASH_STR ? _hash_str(key) : _hash_int(key);

    _map_item* item = tommy_hashlin_search(mp, compare_f, key, hash);
    return item ? item->data : NULL;
}

void map_remove(map* mp, void* key) {
    auto compare_f = mp->htype == MHASH_STR ? _compare_str : _compare_int;
    u32 hash = mp->htype == MHASH_STR ? _hash_str(key) : _hash_int(key);

    tommy_hashlin_remove(mp, compare_f, key, hash);
}

u64 map_size(map* mp) {
    return tommy_hashlin_count(mp);
}

/* ------------------------------------------------------------------------- */

_map_iter _map_iter_begin(map* mp) {
    _map_iter it;
    it.mp = mp;
    it.pos = 0;
    it.node = NULL;
    
    // Calculate valid bucket count like tommy_hashlin_foreach
    tommy_size_t bucket_max = mp->low_max + mp->split;
    
    // Find first non-empty bucket
    for (tommy_size_t i = 0; i < bucket_max; i++) {
        tommy_hashlin_node* node = *tommy_hashlin_pos(mp, i);
        if (node) {
            it.pos = i;
            it.node = node;
            break;
        }
    }
    
    return it;
}

void* _map_iter_next(_map_iter* it) {
    tommy_size_t bucket_max = it->mp->low_max + it->mp->split;
    
    while (it->pos < bucket_max) {
        if (it->node) {
            _map_item* current_item = (_map_item*)it->node->data;
            it->node = it->node->next;
            return current_item->data;
        }
        
        // Move to next bucket
        it->pos++;
        while (it->pos < bucket_max) {
            tommy_hashlin_node* node = *tommy_hashlin_pos(it->mp, it->pos);
            if (node) {
                it->node = node;
                break;
            }
            it->pos++;
        }
    }
    return NULL;
}
