#import "Testing.h"
#import "ObjectTesting.h"
#import "InvokeProxyProtocol.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSGarbageCollector.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSProxy.h>

int main()
{ 
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSInvocation *inv = nil; 
  NSObject <InvokeTarget>*tar;
  NSMethodSignature *sig;
  id ret;
  Class tClass = Nil;
  NSString *bundlePath;
  NSBundle *bundle; 
  int retc; 
  bundlePath = [[[NSFileManager defaultManager] 
                              currentDirectoryPath] 
			       stringByAppendingPathComponent:@"Resources"];
  bundlePath = [[NSBundle bundleWithPath:bundlePath]
                  pathForResource:@"InvokeProxy"
	                   ofType:@"bundle"];
  bundle = [NSBundle bundleWithPath:bundlePath];
  PASS([bundle load],
       "loading resources from bundle");
  tClass = NSClassFromString(@"InvokeTarget");
   
  
  tar = [tClass new];
  
  /* 
    Test if the return value is retained. It is in the Apple OpenStep edition
    for Windows (YellowBox)
    matt: this doesn't seem like a valid test as PASS/fail will vary on
    platforms
   */
  sig = [tar methodSignatureForSelector:@selector(retObject)];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  retc = [[tar retObject] retainCount];
  [inv setSelector:@selector(retObject)];
  [inv invokeWithTarget:tar];
  if (nil == [NSGarbageCollector defaultCollector])
    {
      PASS(retc + 1 == [[tar retObject] retainCount],
       "Retain return value")
    }
  
  sig = [tar methodSignatureForSelector:@selector(loopObject:)];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  retc = [tar retainCount];
  [inv setSelector:@selector(loopObject:)];
  [inv invokeWithTarget:tar];
  [inv retainArguments];
  [inv setArgument:&tar atIndex:2];
  if (nil == [NSGarbageCollector defaultCollector])
    {
      PASS(retc + 1 == [tar retainCount],
       "Will Retain arguments after -retainArguments")
    }
  
  sig = [tar methodSignatureForSelector:@selector(loopObject:)];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  retc = [tar retainCount];
  [inv setSelector:@selector(loopObject:)];
  [inv invokeWithTarget:tar];
  [inv setArgument:&tar atIndex:2];
  PASS(retc == [tar retainCount],
       "default will not retain arguments");
  
  sig = [tar methodSignatureForSelector:@selector(retObject)];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setSelector:@selector(retObject)];
  [inv invokeWithTarget:nil];
  [inv getReturnValue:&ret];
  PASS(ret == nil,"Check if nil target works");
  
  sig = [tar methodSignatureForSelector:@selector(returnIdButThrowException)];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setSelector:@selector(returnIdButThrowException)];
  PASS_EXCEPTION([inv invokeWithTarget:tar];,@"AnException","Exception in invocation #1");
  PASS_EXCEPTION([inv getReturnValue:&ret];,NSGenericException,"Exception getting return value #1");
 
  /* same as above but with a successful call first */
  sig = [tar methodSignatureForSelector:@selector(returnIdButThrowException)];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setSelector:@selector(retObject)];
  
  [inv invokeWithTarget:tar]; /* these two lines */
  [inv getReturnValue:&ret];
  
  [inv setSelector:@selector(returnIdButThrowException)];
  PASS_EXCEPTION([inv invokeWithTarget:tar];,@"AnException","Exception in invocation #2");
  PASS_EXCEPTION([inv getReturnValue:&ret];,NSGenericException,"Exception getting return value #2");
    
  
  [arp release]; arp = nil;
  return 0;
}
