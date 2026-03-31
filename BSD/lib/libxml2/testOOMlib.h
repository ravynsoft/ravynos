#ifndef TEST_OOM_LIB_H
#define TEST_OOM_LIB_H

#include <stddef.h>

void* test_malloc  (size_t      bytes);
void* test_realloc (void       *memory,
                    size_t      bytes);
void  test_free    (void       *memory);
char* test_strdup  (const char *str);

/* returns true on success */
typedef int (* TestMemoryFunction)  (void *data);

/* returns true on success */
int test_oom_handling (TestMemoryFunction  func,
                       void               *data);

/* get number of blocks leaked */
int test_get_malloc_blocks_outstanding (void);

#endif
