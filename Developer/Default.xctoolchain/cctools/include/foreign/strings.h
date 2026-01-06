#include_next <strings.h>

#define HAVE_BCMP
#define HAVE_BZERO
#define HAVE_BCOPY
#define HAVE_INDEX
#define HAVE_RINDEX

#undef bcmp
#undef bzero
#undef bcopy

#ifndef HAVE_BCMP
static inline int bcmp(const void *s1, const void *s2, size_t n)
{
    return __builtin_memcmp(s1, s2, n);
}
#endif

#ifndef HAVE_BZERO
static inline void bzero(void *s, size_t n)
{
    __builtin_memset(s, '\0', n);
}
#endif

#ifndef HAVE_BCOPY
static inline void bcopy(const void *src, void *dest, size_t n)
{
    __builtin_memcpy(dest, src, n);
}
#endif

#ifndef HAVE_INDEX
static inline char *index(const char *s, int c)
{
    return __builtin_strchr(s, c);
}
#endif

#ifndef HAVE_RINDEX
static inline char *rindex(const char *s, int c)
{
    return __builtin_strrchr(s, c);
}
#endif
