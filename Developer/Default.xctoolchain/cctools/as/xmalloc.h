extern void * xmalloc(
    size_t n);
extern void *xrealloc(
    void *ptr,
    size_t n);
#define xfree free
