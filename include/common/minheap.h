#ifndef MINHEAP_H
#define MINHEAP_H

#include <stddef.h>

typedef struct {
    void *data;
    int priority;
} HeapNode;

typedef struct {
    int size;
    int capacity;
    HeapNode *array;
} MinHeap;

MinHeap *minheap_create(int capacity);
void minheap_free(MinHeap *heap);

void minheap_insert(MinHeap *heap, void *data, int priority);
HeapNode minheap_extract_min(MinHeap *heap);
HeapNode minheap_peek(MinHeap *heap);

int minheap_is_empty(MinHeap *heap);

#endif
