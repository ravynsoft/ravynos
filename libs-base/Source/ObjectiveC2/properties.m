#import "Foundation/NSObject.h"

#if	!defined(_NATIVE_OBJC_EXCEPTIONS)
/* If we don't have support for native exceptions then we can't use
 * @synchronized, so we use NSException and NSLock.
 * This is a horribly inefficient, but you probably shouldn't be using
 * the property functions anyway :-)
 */
#import	"Foundation/NSException.h"
#import	"Foundation/NSLock.h"

static NSRecursiveLock *propertyLock = nil;
static inline NSRecursiveLock*
pLock()
{
  if (propertyLock == nil)
    {
      [gnustep_global_lock lock];
      if (propertyLock == nil)
        {
	  propertyLock = [NSRecursiveLock new];
	}
      [gnustep_global_lock unlock];
    }
  return propertyLock;
}

#define	SYNCBEG(X) \
[pLock() lock]; \
NS_DURING

#define	SYNCEND() \
NS_HANDLER \
[pLock() unlock]; \
[localException raise]; \
NS_ENDHANDLER \
[pLock() unlock]

#else
 
#define	SYNCBEG(X) @synchronized(X) {
#define	SYNCEND() }

#endif

id
objc_getProperty(id obj, SEL _cmd, ptrdiff_t offset, BOOL isAtomic)
{
  char *addr;
  id ret;

  if (isAtomic)
    {
      id	result = nil;

      SYNCBEG(obj)
	result = objc_getProperty(obj, _cmd, offset, NO);
      SYNCEND();
      return result;
    }
  addr = (char*)obj;
  addr += offset;
  ret = *(id*)addr;
  return [[ret retain] autorelease];
}

void
objc_setProperty(id obj, SEL _cmd, ptrdiff_t offset, id arg, BOOL isAtomic,
  BOOL isCopy)
{
  char *addr;
  id old;

  if (isAtomic)
    {
      SYNCBEG(obj)
	objc_setProperty(obj, _cmd, offset, arg, NO, isCopy);
      SYNCEND();
      return;
    }
  if (isCopy)
    {
      arg = [arg copy];
    }
  else
    {
      arg = [arg retain];
    }
  addr = (char*)obj;
  addr += offset;
  old = *(id*)addr;
  *(id*)addr = arg;
  [old release];
}
