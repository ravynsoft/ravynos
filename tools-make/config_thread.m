/* Test whether Objective-C runtime was compiled with thread support */

#ifdef GNU_RUNTIME
/* Dummy NXConstantString impl for so libobjc that doesn't include it */
#include <objc/NXConstStr.h>
@implementation NXConstantString
@end
#endif

#ifdef GNU_RUNTIME
#include <objc/thr.h>
#else
#include <pthread.h>
void *dummy_thread_function(void *dummy)
{
  pthread_exit(NULL);
}
#endif

#include <objc/Object.h>

int
main()
{
#ifdef GNU_RUNTIME
  id o = [Object new];

  return (objc_thread_detach (@selector(hash), o, nil) == NULL) ? -1 : 0;
#else

  /* On Apple, there is no ObjC-specific thread library.  We need to
   * use pthread directly.
   */
  pthread_t thread_id;
  int error = pthread_create (&thread_id, NULL, dummy_thread_function, NULL);

  if (error != 0)
    {
      return -1;
    }

  return 0;

#endif
}
