#define object 1
#define function(x) 1

#if object
once
#endif
#if object
twice
#endif

#if function(0)
once
#endif
#if function(0)
once again
#endif
