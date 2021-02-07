#import <Foundation/Foundation.h>
#import "Testing.h"

static NSString *desc = @"[MyObject]";

static NSString *
myObjectDescription(id self, SEL _cmd)
{
  return desc;
}

int
main(void)
{
  id obj;
  Class cls;
  NSAutoreleasePool *pool;

  pool = [NSAutoreleasePool new];
  cls = (Class)objc_allocateClassPair([NSObject class], "MyObject", 0);
  if (cls != Nil)
    {
      objc_registerClassPair(cls);
      class_addMethod(cls, @selector(description),
		      (IMP)myObjectDescription, "@@:");
      obj = [cls new];
      pass([obj description] == desc, "New class's description method is called correctly");
      [obj release];
    }

  return 0;
}
