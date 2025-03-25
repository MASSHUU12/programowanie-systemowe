#include <stdio.h>
#include <string.h>
#include "mem_man.h"

void print_error() {
  switch (mem_errno) {
    case MEM_SUCCESS:
      printf("No error\n");
      break;
    case MEM_ERR_MALLOC:
      printf("Error: malloc failed\n");
      break;
    case MEM_ERR_INVALID_PTR:
      printf("Error: invalid pointer\n");
      break;
    case MEM_ERR_FREE:
      printf("Error: free failed\n");
      break;
    case MEM_ERR_CALLOC:
      printf("Error: calloc failed\n");
      break;
    case MEM_ERR_REALLOC:
      printf("Error: realloc failed\n");
      break;
    default:
      printf("Error: unknown error code %d\n", mem_errno);
  }
}

void test_scenario_1() {
  printf("\n=== TEST SCENARIO 1: Basic allocation and deallocation ===\n");

  void *ptr = mem_alloc(NULL, 100);
  if (ptr != NULL) {
    printf("Memory allocated successfully at address %p\n", ptr);

    strcpy(ptr, "This is a test string");
    printf("Stored data: '%s'\n", (char*)ptr);

    if (mem_free(ptr) == 0) {
      printf("Memory freed successfully\n");
    } else {
      printf("Failed to free memory\n");
      print_error();
    }
  } else {
    printf("Failed to allocate memory\n");
    print_error();
  }
}

void test_scenario_2() {
  printf("\n=== TEST SCENARIO 2: Reallocation ===\n");

  void *ptr = mem_alloc(NULL, 20);
  if (ptr != NULL) {
    printf("Memory allocated successfully at address %p, size: 20\n", ptr);

    strcpy(ptr, "Initial string");
    printf("Stored data: '%s'\n", (char*)ptr);

    void *new_ptr = mem_alloc(ptr, 50);
    if (new_ptr != NULL) {
      printf("Memory reallocated successfully at address %p, size: 50\n", new_ptr);
      printf("Stored data after reallocation: '%s'\n", (char*)new_ptr);

      strcat(new_ptr, " with additional data");
      printf("Updated data: '%s'\n", (char*)new_ptr);

      if (mem_free(new_ptr) == 0) {
        printf("Memory freed successfully\n");
      } else {
        printf("Failed to free memory\n");
        print_error();
      }
    } else {
      printf("Failed to reallocate memory\n");
      print_error();
      mem_free(ptr);
    }
  } else {
    printf("Failed to allocate memory\n");
    print_error();
  }
}

void test_scenario_3() {
  printf("\n=== TEST SCENARIO 3: Invalid pointer handling ===\n");

  // Allocate memory through standard malloc (not tracked by manager)
  void *ptr = malloc(100);
  if (ptr != NULL) {
    printf("Memory allocated through standard malloc at address %p\n", ptr);

    // Try to free with manager
    printf("Attempting to free untracked memory with mem_free...\n");
    if (mem_free(ptr) != 0) {
      printf("Correctly detected invalid pointer\n");
      print_error();
    } else {
      printf("Error: mem_free should have detected invalid pointer\n");
    }

    free(ptr);
    printf("Memory freed with standard free\n");
  }

  printf("Testing mem_free with NULL pointer...\n");
  if (mem_free(NULL) != 0) {
    printf("Correctly detected NULL pointer\n");
    print_error();
  } else {
    printf("Error: mem_free should have detected NULL pointer\n");
  }
}

void test_scenario_4() {
  printf("\n=== TEST SCENARIO 4: Multiple allocations ===\n");

  void *ptrs[5];
  int i;

  // Allocate multiple memory blocks
  for (i = 0; i < 5; i++) {
    ptrs[i] = mem_alloc(NULL, 100 + i * 20);
    if (ptrs[i] != NULL) {
      printf("Allocated memory #%d at address %p, size: %d\n", i+1, ptrs[i], 100 + i * 20);
      sprintf(ptrs[i], "Memory block %d", i+1);
    } else {
      printf("Failed to allocate memory #%d\n", i+1);
      print_error();
    }
  }

  for (i = 0; i < 5; i++) {
    if (ptrs[i] != NULL) {
      printf("Memory block #%d contains: '%s'\n", i+1, (char*)ptrs[i]);
    }
  }

  // Free memory blocks (leaving some for auto-cleanup)
  for (i = 0; i < 3; i++) {
    if (ptrs[i] != NULL) {
      if (mem_free(ptrs[i]) == 0) {
        printf("Freed memory block #%d\n", i+1);
        ptrs[i] = NULL;
      } else {
        printf("Failed to free memory block #%d\n", i+1);
        print_error();
      }
    }
  }

  printf("Memory blocks #4 and #5 intentionally left allocated for auto-cleanup\n");
}

int main() {
  mem_init_manager();
  printf("Memory manager initialized\n");

  test_scenario_1();
  test_scenario_2();
  test_scenario_3();
  test_scenario_4();

  printf("\n=== Program terminating, automatic memory cleanup should occur ===\n");
  return 0;
}
