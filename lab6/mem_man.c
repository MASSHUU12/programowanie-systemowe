#include "mem_man.h"
#include <string.h>

typedef struct Allocation {
  void *ptr;
  unsigned int size;
  struct Allocation *next;
} Allocation;

static Allocation *alloc_list = NULL;
int mem_errno = MEM_SUCCESS;

static int add_allocation(void *ptr, unsigned int size) {
  Allocation *node = (Allocation *)malloc(sizeof(Allocation));
  if (!node) {
    mem_errno = MEM_ERR_MALLOC;
    return -1;
  }
  node->ptr = ptr;
  node->size = size;
  node->next = alloc_list;
  alloc_list = node;
  return 0;
}

static int remove_allocation(void *ptr) {
  Allocation *current = alloc_list, *prev = NULL;
  while (current) {
    if (current->ptr == ptr) {
      if (prev)
        prev->next = current->next;
      else
        alloc_list = current->next;
      free(current);
      return 0;
    }
    prev = current;
    current = current->next;
  }
  return -1;
}

static void free_all_allocations(void) {
  Allocation *current = alloc_list;
  while (current) {
    Allocation *next = current->next;
    free(current->ptr);
    free(current);
    current = next;
  }
  alloc_list = NULL;
}

void mem_init_manager(void) { atexit(free_all_allocations); }

void *mem_alloc(void *ptr, unsigned int size) {
  mem_errno = MEM_SUCCESS;

  if (size == 0) {
    return NULL;
  }

  void *new_ptr = NULL;

  if (ptr == NULL) {
    new_ptr = calloc(1, size);
    if (!new_ptr) {
      mem_errno = MEM_ERR_CALLOC;
      return NULL;
    }

    if (add_allocation(new_ptr, size) != 0) {
      free(new_ptr);
      return NULL;
    }
  } else {
    Allocation *current = alloc_list;
    int found = 0;

    while (current) {
      if (current->ptr == ptr) {
        found = 1;
        break;
      }
      current = current->next;
    }

    if (!found) {
      mem_errno = MEM_ERR_INVALID_PTR;
      return NULL;
    }

    new_ptr = realloc(ptr, size);
    if (!new_ptr) {
      mem_errno = MEM_ERR_REALLOC;
      return NULL;
    }

    if (size > current->size) {
      memset((char *)new_ptr + current->size, 0, size - current->size);
    }

    if (ptr != new_ptr) {
      if (remove_allocation(ptr) != 0) {
        mem_errno = MEM_ERR_FREE;
        free(new_ptr);
        return NULL;
      }
      if (add_allocation(new_ptr, size) != 0) {
        mem_errno = MEM_ERR_MALLOC;
        free(new_ptr);
        return NULL;
      }
    } else {
      current->size = size;
    }
  }

  return new_ptr;
}

int mem_free(void *ptr) {
  mem_errno = MEM_SUCCESS;

  if (!ptr) {
    mem_errno = MEM_ERR_INVALID_PTR;
    return -1;
  }

  Allocation *current = alloc_list;
  int found = 0;

  while (current) {
    if (current->ptr == ptr) {
      found = 1;
      break;
    }
    current = current->next;
  }

  if (!found) {
    mem_errno = MEM_ERR_INVALID_PTR;
    return -1;
  }

  free(ptr);

  if (remove_allocation(ptr) != 0) {
    mem_errno = MEM_ERR_FREE;
    return -1;
  }

  return 0;
}
