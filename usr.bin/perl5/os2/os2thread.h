#include <sys/builtin.h>
#include <sys/fmutex.h>
#include <sys/rmutex.h>
typedef int perl_os_thread;

typedef _rmutex perl_mutex;

/*typedef HEV perl_cond;*/	/* Will include os2.h into all C files.  */
typedef unsigned long perl_cond;
int os2_cond_wait(perl_cond *c, perl_mutex *m);

#ifdef USE_SLOW_THREAD_SPECIFIC
typedef int perl_key;
#else
typedef void** perl_key;
#endif

typedef unsigned long pthread_attr_t;
#define PTHREADS_INCLUDED
#define pthread_attr_init(arg) 0
#define pthread_attr_setdetachstate(arg1,arg2) 0
