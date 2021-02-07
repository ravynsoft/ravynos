#import "ObjectTesting.h"
#import <Foundation/Foundation.h>

id a,b,c;

int main(void)
{
  NSAutoreleasePool     *p = [NSAutoreleasePool new];
  NSFileHandle  *a;
  NSFileHandle  *b;
  NSFileHandle  *c;

  START_SET("handle creation")
  a = [NSFileHandle fileHandleWithStandardInput];
  b = [NSFileHandle fileHandleWithStandardOutput];
  c = [NSFileHandle fileHandleWithStandardError];
  END_SET("handle creation")
  PASS([a retainCount]> 0, "stdin persists");
  PASS([b retainCount]> 0, "stdout persists");
  PASS([c retainCount]> 0, "strerr persists");
  PASS_EXCEPTION([a release], NSGenericException, "Cannot dealloc stdin");
  PASS_EXCEPTION([b release], NSGenericException, "Cannot dealloc stdout");
  PASS_EXCEPTION([c release], NSGenericException, "Cannot dealloc stderr");
  PASS([a retainCount]> 0, "stdin persists");
  PASS([b retainCount]> 0, "stdout persists");
  PASS([c retainCount]> 0, "strerr persists");

  [p drain];
  return 0;
}
