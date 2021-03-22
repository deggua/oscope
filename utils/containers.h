#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <stdint.h>

typedef struct {
    size_t len;
    void* vals[];
} array_t;

typedef struct LINKEDLIST_T{
    void* val;
    struct LINKEDLIST_T* next;
    struct LINKEDLIST_T* prev;
} linkedlist_t;

array_t* Array_New(size_t len, size_t type_size);

array_t* Array_Resize(array_t* list, size_t len, size_t type_size);

void Array_Delete(array_t* list);

#endif