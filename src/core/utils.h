#pragma once


#define count_to_null(arr, count) \
    for (int i = 0; ; i++) { if (arr[i] == NULL) { count = i; break; }}
