#include "shims.h"
#include <errno.h>

char *
strchrnul(char * s, char ch)
{
	char * result = strchr(s, ch);
	if (!result)
		return &s[strlen(s)];
	return result;
}

int
eaccess(const char *p, int mode)
{
	return access(p, mode);
}

void *
reallocarray(void * p, size_t n, size_t size)
{
	return realloc(p, n * size);
}

int
qsort_s(void * base, size_t nmemb, size_t size, fn_t * compar, void * thunk)
{
	if (nmemb > RSIZE_MAX || size > RSIZE_MAX)
		return EINVAL;
	if (nmemb > 0 && compar == NULL)
		return EINVAL;
	qsort_r(base, nmemb, size, compar, thunk);
	return 0;
}

