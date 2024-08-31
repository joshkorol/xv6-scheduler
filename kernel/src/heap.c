#include "asm/x86.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

// stats.execution_time is a little too verbose for me :(
#define key stats.execution_time

// Global minheap h, this is so the pointer array is properly allocated
// proc_heap is the pointer we actually use
minheap h;
minheap* proc_heap;
int heap_initialized = 0;
// Initialize:
minheap* heap_init() {
  // multiple cores causes issues because the heap will be initialized multiple
  // times, leading to all sorts of panics. In order to prevent this we use
  // heap_initialized flag. Another important note: heap_init() must be called
  // within a code section that has a lock on ptable.lock
  if (heap_initialized) {
    return &h;
  }
  h.size = 0;
  h.capacity = NPROC;
  for (int i = 0; i < NPROC; i++) {
    h.arr[i] = NILL;
  }
  heap_initialized = 1;
  return &h;
}

// Given a pointer to a proc pointer (proc pointer is entry in heap array), swap
// the two pointers
void swap(struct proc** a, struct proc** b) {
  struct proc* temp = *a;
  *a = *b;
  *b = temp;
}

int parent(int i) { return (i - 1) / 2; }

int left(int i) { return 2 * i + 1; }

int right(int i) { return 2 * i + 2; }

// Adds a proc pointer to the heap
// Must be called inside a code section w/ the ptable.lock held to prevent race
// conditions
void heap_push(minheap* heap, struct proc* p) {
  if (heap->size == heap->capacity) {
    panic("Proc heap is full :( NPROC exceeded or you seriously messed up\n");
    return;
  }
  int i = heap->size;
  heap->arr[i] = p;
  heap->size++;
  while (i != 0 && heap->arr[parent(i)]->key > heap->arr[i]->key) {
    swap(&(heap->arr[i]), &(heap->arr[parent(i)]));
    i = parent(i);
  }
}

// Takes me back to CS1332
void heapify(minheap* heap, int i) {
  int smallest = i;
  int l = left(i);
  int r = right(i);
  if (l < heap->size && heap->arr[l]->key < heap->arr[smallest]->key) {
    smallest = l;
  }
  if (r < heap->size && heap->arr[r]->key < heap->arr[smallest]->key) {
    smallest = r;
  }

  if (smallest != i) {
    swap(&(heap->arr[i]), &(heap->arr[smallest]));
    heapify(heap, smallest);
  }
}

// Must be called inside a code section w/ the ptable.lock held to prevent race
// conditions
struct proc* heap_pop(minheap* heap) {
  if (heap->size == 0) {
    return NILL;
  }
  if (heap->size == 1) {
    heap->size--;
    struct proc* ret = heap->arr[0];
    heap->arr[0] = NILL;
    return ret;
  }
  struct proc* root = heap->arr[0];
  // Swap last element w first
  heap->arr[0] = heap->arr[heap->size - 1];
  heap->size--;
  heapify(heap, 0);
  return root;
}