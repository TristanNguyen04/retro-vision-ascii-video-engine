#include "minheap.h"

static void swap(HeapNode *a, HeapNode *b) {
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

static void bubble_up(MinHeap *heap, int idx) {
    int par;
    while (idx > 0) {
        par = (idx - 1) / 2;
        if (heap->array[par].priority <= heap->array[idx].priority)
            break;

        swap(&heap->array[par], &heap->array[idx]);
        idx = par;
    }
}

static void bubble_down(MinHeap *heap, int idx) {
    int smallest = idx;
    int left, right;

    while (1) {
        left = 2 * idx + 1;
        right = 2 * idx + 2;
        if (left < heap->size && heap->array[left].priority < heap->array[smallest].priority) {
            smallest = left;
        }
        if (right < heap->size && heap->array[right].priority < heap->array[smallest].priority) {
            smallest = right;
        }

        if (smallest == idx)
            break;

        swap(&heap->array[idx], &heap->array[smallest]);
        idx = smallest;
    }
}

MinHeap *minheap_create(int capacity) {
    MinHeap *heap = malloc(sizeof(MinHeap));
    if (!heap)
        return NULL;

    heap->size = 0;
    heap->capacity = 0;
    heap->array = (HeapNode *)malloc(capacity * sizeof(HeapNode));

    if (!heap->array) {
        free(heap);
        return NULL;
    }

    return heap;
}

void minheap_free(MinHeap *heap) {
    if (!heap)
        return;

    free(heap->array);
    free(heap);
}

void minheap_insert(MinHeap *heap, void *data, int priority) {
    int idx;
    if (heap->size == heap->capacity) {
        /* resize */
        heap->capacity *= 2;
        heap->array = realloc(heap->array, heap->capacity * sizeof(HeapNode));
    }

    idx = heap->size++;

    heap->array[idx].data = data;
    heap->array[idx].priority = priority;

    heapify_up(heap, idx);
}

HeapNode minheap_extract_min(MinHeap *heap) {
    HeapNode root;

    if (minheap_is_empty(heap)) {
        printf("Error: Heap underflow\n");
        exit(1);
    }

    root = heap->array[0];

    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;

    heapify_down(heap, 0);

    return root;
}

HeapNode minheap_peek(MinHeap *heap) {
    if (minheap_is_empty(heap)) {
        printf("Error: Heap underflow\n");
        exit(1);
    }

    return heap->array[0];
}

int minheap_is_empty(MinHeap *heap) {
    if (!heap)
        return 1;
    return heap->size == heap->capacity;
}