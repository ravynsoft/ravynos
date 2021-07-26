#include <stdlib.h>

void *valloc(size_t);

// Stubs that just call the libc implementations when you call these.

void *objc_malloc(size_t size)
{
	return malloc(size);
}

void *objc_atomic_malloc(size_t size)
{
	return malloc(size);
}

#ifdef __MINGW32__
void *objc_valloc(size_t size)
{
	return malloc(size);
}
#else
void *objc_valloc(size_t size)
{
	return valloc(size);
}
#endif

void *objc_realloc(void *mem, size_t size)
{
	return realloc(mem, size);
}

void * objc_calloc(size_t nelem, size_t size)
{
	return calloc(nelem, size);
}

void objc_free(void *mem)
{
	free(mem);
}

