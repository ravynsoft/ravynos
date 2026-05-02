#ifndef _SHIMS_H
#define _SHIMS_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define O_VERIFY 0x10000000 /* dummy value */

typedef int (*fn_t)(const void *, const void *, void *);

char * strchrnul(char * s, char ch);
int eaccess(const char * p, int mode);
void * reallocarray(void * p, size_t n, size_t size);
int qsort_s(void * base, size_t nmemb, size_t size, fn_t * compar, void * thunk);

#endif

