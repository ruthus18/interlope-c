#pragma once
#include <stdlib.h>


static inline
int __tuple_size(void** tup) {
    for (int i = 0; ; i++) {if (tup[i] == NULL)  return i;}
}

#define tuple_size(tup)  __tuple_size((void**)tup)


#define tuple_for_each(current, tup) \
    for (int _i = 0; (current = tup[_i]) && (current != NULL); _i++)
