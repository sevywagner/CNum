#ifndef __ARENA_H
#define __ARENA_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARENA_BLOCK_SIZE 64
  
  struct linked_list_node {
    void *ptr;
    struct linked_list_node *next;
  };

  typedef struct arena {
    void *heap_base;
    void *heap_ptr;
    size_t capacity;
    size_t bytes_available;
    struct linked_list_node *heap_allocations_head;
    struct linked_list_node *heap_allocations_current;
  } arena_t;

  typedef struct arena_view {
    void *ptr;
    size_t range;
    size_t type_size;
  } arena_view_t;
  
  arena_t *arena_init(uint32_t blocks_to_allocate);
  arena_view_t arena_malloc(arena_t *arena, size_t bytes, size_t type_size);

  void arena_view_reset(arena_view_t arena_view);
  void arena_clear(arena_t *arena);
  void arena_free(arena_t *arena);

#ifdef __cplusplus
}
#endif
  
#endif
