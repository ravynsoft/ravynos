#ifdef INT
typedef int ret_t;
#elif CHAR
typedef char *ret_t;
#else
typedef short *ret_t;
#endif

ret_t FUN (void) { return 0; }
