#ifndef MEM_MAN_H_
#define MEM_MAN_H_

#include <stdlib.h>

extern int mem_errno;

#define MEM_SUCCESS 0
#define MEM_ERR_MALLOC 1
#define MEM_ERR_INVALID_PTR 2
#define MEM_ERR_FREE 3
#define MEM_ERR_CALLOC 4
#define MEM_ERR_REALLOC 5

int mem_free(void *ptr);
void mem_init_manager(void);
void *mem_alloc(void *ptr, unsigned int size);

#endif // MEM_MAN_H_
