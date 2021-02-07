#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSProxy.h>
#import <Foundation/NSString.h>
#import <Foundation/NSGeometry.h>
#import "InvokeProxyProtocol.h"

/* these macros should only be used in the scope of main */
#define TEST_INVOKE(selx) \
  { \
    NS_DURING \
      NSMethodSignature *sig = nil; \
      sig = [tar methodSignatureForSelector:selx]; \
      inv = [NSInvocation invocationWithMethodSignature:sig]; \
      [inv setSelector:selx]; \
      [inv invokeWithTarget:tar]; \
      PASS(1,"Invoke %s",[NSStringFromSelector(selx) UTF8String]); \
    NS_HANDLER \
      PASS(0,"Invoke %s",[NSStringFromSelector(selx) UTF8String]); \
      [localException raise]; \
    NS_ENDHANDLER \
  }

#define TEST_INVOKE_ARG(selx,argp) \
  { \
    NS_DURING \
      NSMethodSignature *sig = nil; \
      sig = [tar methodSignatureForSelector:selx]; \
      inv = [NSInvocation invocationWithMethodSignature:sig]; \
      [inv setSelector:selx]; \
      [inv setArgument:argp atIndex:2]; \
      [inv invokeWithTarget:tar]; \
      PASS(1,"Invoke %s",[NSStringFromSelector(selx) UTF8String]); \
    NS_HANDLER \
      PASS(0,"Invoke %s",[NSStringFromSelector(selx) UTF8String]); \
      [localException raise]; \
    NS_ENDHANDLER \
  }

int main()
{ 
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSInvocation *inv = nil; 
  NSObject <InvokeTarget>*tar;
  NSObject <InvokeProxy,InvokeTarget>*pxy;
  char cret,carg;
  short sret,sarg;
  int iret,iarg;
  long lret,larg;
  float fret,farg;
  double dret,darg;
  NSRect rret, rarg, rprx;
  id oret,oarg;
  char *cpret,*cparg,*cparg2;
  smallStruct ssret,ssarg;
  largeStruct lsret,lsarg;
  Class tClass = Nil;
  Class pClass = Nil;
  NSString *bundlePath;
  NSBundle *bundle; 
  
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
  pClass = NSClassFromString(@"InvokeProxy");
   
  
  tar = [tClass new];
  pxy = [[pClass alloc] initWithTarget:tar]; 
  TEST_INVOKE(@selector(retChar));
  [inv getReturnValue:&cret];
  PASS(cret == 99 &&
       [pxy retChar] == 99, 
       "Can return chars");
  
  TEST_INVOKE(@selector(retShort));
  [inv getReturnValue:&sret];
  PASS(sret == 12345 &&
       [pxy retShort] == 12345, 
       "Can return short");
  
  TEST_INVOKE(@selector(retInt));
  [inv getReturnValue:&iret];
  PASS(iret == 123456 &&
       [pxy retInt] == 123456, 
       "Can return int");
   
  TEST_INVOKE(@selector(retLong));
  [inv getReturnValue:&lret];
  PASS(lret == 123456 &&
       [pxy retLong] == 123456, 
       "Can return long");
  
  TEST_INVOKE(@selector(retFloat));
  [inv getReturnValue:&fret];
  PASS(fabs(123.456 - fret) <= 0.001 &&
       fabs(123.456 - [pxy retFloat]) <= 0.001, 
       "Can return float");
   
  TEST_INVOKE(@selector(retDouble));
  [inv getReturnValue:&dret];
  PASS(fabs(123.456 - dret) <= 0.001 &&
       fabs(123.456 - [pxy retDouble]) <= 0.001, 
       "Can return double");
   
  TEST_INVOKE(@selector(retObject));
  [inv getReturnValue:&oret];
  PASS(oret == tar &&
       tar == [pxy retObject], 
       "Can return object");
  
  carg = 1; 
  TEST_INVOKE_ARG(@selector(loopChar:),&carg);
  [inv getReturnValue:&cret];
  PASS(cret == 2 &&
       2 == [pxy loopChar:carg],
       "Can send/return chars");

  sarg = 1;
  TEST_INVOKE_ARG(@selector(loopShort:),&sarg);
  [inv getReturnValue:&sret];
  PASS(sret == 2 &&
       [pxy loopShort:sarg] == 2,
       "Can send/return shorts");
  
  iarg = 1;
  TEST_INVOKE_ARG(@selector(loopInt:),&iarg);
  [inv getReturnValue:&iret];
  PASS(iret == 2 &&
       [pxy loopInt:iarg] == 2,
       "Can send/return ints");
   
  larg = 1;
  TEST_INVOKE_ARG(@selector(loopLong:),&larg);
  [inv getReturnValue:&lret];
  PASS(lret == 2 &&
       [pxy loopLong:larg] == 2,
       "Can send/return longs");
  
  farg = 1;
  TEST_INVOKE_ARG(@selector(loopFloat:),&farg);
  [inv getReturnValue:&fret];
  PASS(fabs(2 - fret) <= 0.001 &&
       fabs(2 - [pxy loopFloat:farg]) <= 0.001,
       "Can send/return floats");
  
  darg = 1;
  TEST_INVOKE_ARG(@selector(loopDouble:),&darg);
  [inv getReturnValue:&dret];
  PASS(fabs(2 - dret) <= 0.001 &&
       fabs(2 - [pxy loopFloat:darg]) <= 0.001,
       "Can send/return double");
  
  oarg = pxy;
  TEST_INVOKE_ARG(@selector(loopObject:),&oarg);
  [inv getReturnValue:&oret];
  PASS(oret == pxy &&
       [pxy loopObject:oarg] == pxy,
       "Can send/return objects");
  
  /* unlinke the rest of these loopString: modifies its arg */
  cparg = malloc(sizeof("Hello There"));
  strcpy(cparg,"Hello There");
  cparg2 = malloc(sizeof("Hello There"));
  strcpy(cparg2,"Hello There");
  
  TEST_INVOKE_ARG(@selector(loopString:),&cparg);
  [inv getReturnValue:&cpret];
  PASS(strcmp(cpret,"Iello There") == 0 &&
       strcmp([pxy loopString:cparg2],"Iello There") == 0,
       "Can send/return char *");

  ssarg.c = 7;
  ssarg.i = 9;
  TEST_INVOKE_ARG(@selector(loopSmall:),&ssarg);
  [inv getReturnValue:&ssret];
  PASS(ssret.c == 7 && ssret.i == 10,
       "Can send/return small structs"); 
  
  lsarg.i = 8;
  lsarg.s = "Hello";
  lsarg.f = 11.0;
  TEST_INVOKE_ARG(@selector(loopLarge:),&lsarg);
  [inv getReturnValue:&lsret];
  PASS(lsret.i == 9 && 
       strcmp(lsret.s, "Hello") == 0 &&
       lsret.f == 11.0,
       "Can send/return large structs");
  
  rarg = NSMakeRect(1,2,3,4);
  TEST_INVOKE_ARG(@selector(loopRect:),&rarg);
  [inv getReturnValue:&rret];
  rprx = [pxy loopRect:rarg];
  PASS(NSEqualRects(rret, rarg) && NSEqualRects(rprx, rarg),
       "Can send/return NSRect");
  
  [arp release]; arp = nil;
  return 0;
}
