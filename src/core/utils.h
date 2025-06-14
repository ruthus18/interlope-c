#pragma once


static inline
int __count_(void** arr) {
    for (int i = 0; ; i++) {
        if (arr[i] == NULL)  return i;
    }
}

#define count_(arr)  __count_((void**)arr)


#define for_each(current, arr) \
    for (int _i = 0; (current = arr[_i]) && (current != NULL); _i++)
