/* Common information for all objc runtime tests.
 */
#include <stdlib.h>
#include <objc/objc.h>

#if __GNU_LIBOBJC__
# include <objc/runtime.h>
# include <objc/message.h>
#else
# include <objc/objc-api.h>
#endif

#include <objc/Object.h>

#ifdef __GNUSTEP_RUNTIME__
#include <objc/hooks.h>
#endif

/* Provide an implementation of NXConstantString for an old libobjc when
   built stand-alone without an NXConstantString implementation.  */
#if !defined(NeXT_RUNTIME) && !defined(__GNUSTEP_RUNTIME__)
@implementation NXConstantString
- (const char*) cString
{
  return 0;
}
- (unsigned int) length
{
  return 0;
}
@end
#endif

#if HAVE_OBJC_ROOT_CLASS_ATTRIBUTE
#define GS_OBJC_ROOT_CLASS __attribute__((objc_root_class))
#else
#define GS_OBJC_ROOT_CLASS
#endif

#if     !defined(__APPLE__)

/* Provide dummy implementations for NSObject and NSConstantString
 * for libobjc2 which needs them.
 */
GS_OBJC_ROOT_CLASS @interface NSObject
{ 
 id isa;
}
@end
@implementation NSObject
+ (id)new
{
  NSObject *obj = calloc(sizeof(id), 1);
  obj->isa = self;
  return obj;
}
#if defined(NeXT_RUNTIME)
/* The Apple runtime always calls this method */
+ (void)initialize { }
#endif
@end

@interface NSConstantString : NSObject
@end
@implementation NSConstantString
@end
#endif  /* __APPLE__ */

