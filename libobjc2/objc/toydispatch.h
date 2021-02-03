/**
 * toydispatch implements a (tiny) subset of the libdispatch interfaces.  It
 * can produce FIFO work queues, but not concurrent ones (although queues are
 * concurrent with respect to each other, as with libdispatch).  Unlike
 * libdispatch, queues all run on the same system thread.  This is less
 * efficient, so the real libdispatch should be used on platforms where it is
 * available.
 *
 * Toydispatch symbol names are prefixed with toy_ so programs can be linked to
 * both libdispatch and toydispatch.  
 */

/* If the real libdispatch exists, use that instead of the toy one. */
#if !defined(__has_include)
#define __has_include(x) 0
#endif
#if __has_include(<dispatch/dispatch.h>) && !defined(__TOY_DISPATCH__)
#	include <dispatch/dispatch.h>
#else

/**
 * Function type for functions that can be added to dispatch queues.
 */
typedef void (*dispatch_function_t)(void *);

typedef struct dispatch_queue * dispatch_queue_t;

#define dispatch_queue_create toy_dispatch_queue_create
/**
 * Create a new queue.  Both parameters are ignored by toydispatch.
 */
dispatch_queue_t dispatch_queue_create(const char *label, void *attr);

#define dispatch_async_f toy_dispatch_async_f
/**
 * Add a function to the queue.  
 */
void dispatch_async_f(dispatch_queue_t queue,
                      void *context,
                      dispatch_function_t work);

#define dispatch_release toy_dispatch_release
void dispatch_release(dispatch_queue_t queue);

#define dispatch_retain toy_dispatch_retain
void dispatch_retain(dispatch_queue_t queue);
#endif
