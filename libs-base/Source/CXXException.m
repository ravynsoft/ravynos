#if defined(__has_include)
#if __has_include(<objc/hooks.h>)
#import "Foundation/NSObject.h"
#import "GNUstepBase/CXXException.h"
#include <objc/runtime.h>
#include <objc/hooks.h>

/** From the CodeSourcery ABI Spec, with C++ pointers turned to void*, and
 * other parts abridged. */

typedef enum
{
  _URC_FOREIGN_EXCEPTION_CAUGHT = 1
} _Unwind_Reason_Code;

struct _Unwind_Exception;

typedef void (*_Unwind_Exception_Cleanup_Fn) (_Unwind_Reason_Code,
                                              struct _Unwind_Exception *);
struct _Unwind_Exception
{
  uint64_t exception_class;
  _Unwind_Exception_Cleanup_Fn exception_cleanup;
  unsigned long private_1;
  unsigned long private_2;
} __attribute__((__aligned__));

_Unwind_Reason_Code _Unwind_Resume_or_Rethrow(struct _Unwind_Exception *);


struct __cxa_exception
{
  void *exceptionType;
  void (*exceptionDestructor) (void *); 
  void (*unexpectedHandler) (void *); 
  void (*terminateHandler) (void *); 
  void *nextException;

  int			handlerCount;
  int			handlerSwitchValue;
  const char *		actionRecord;
  const char *		languageSpecificData;
  void *			catchTemp;
  void *			adjustedPtr;
  struct _Unwind_Exception	unwindHeader;
};

@implementation CXXException
static Class CXXExceptionClass;
// TODO: Add an API for registering other classes for other exception types
static Class boxClass(int64_t foo)
{
  // Big endian platforms
  if (foo == *(int64_t*)(void*)"GNUCC++\0")
    {
      return CXXExceptionClass;
    }
  // Little endian platforms
  if (foo == *(int64_t*)(void*)"\0++CCUNG")
    {
      return CXXExceptionClass;
    }
  return Nil;
}
+ (void) load
{
  CXXExceptionClass = self;
  _objc_class_for_boxing_foreign_exception = boxClass;
}
+ (id) exceptionWithForeignException: (struct _Unwind_Exception*)ex
{
  CXXException *box = [self new];
  box->ex = ex;
  return [box autorelease];
}
- (void*) thrownValue
{
  return ex + 1;
}
- (void*) cxx_type_info
{
  char *ptr = (char*)ex;
  ptr -= __builtin_offsetof(struct __cxa_exception, unwindHeader);
  return ((struct __cxa_exception*)(void*)ptr)->exceptionType;
}
- (void) rethrow
{
#if defined(WITH_UNWIND)
  struct _Unwind_Exception *re = ex;
  // We aren't allowed to hold onto the exception if it's been rethrown.
  ex = 0;
  _Unwind_Resume_or_Rethrow(re);
#endif
}
- (void) dealloc
{
  if (0 != ex && 0 != ex->exception_cleanup)
    {
      ex->exception_cleanup(_URC_FOREIGN_EXCEPTION_CAUGHT, ex);
    }
  [super dealloc];
}
@end

#endif
#endif
