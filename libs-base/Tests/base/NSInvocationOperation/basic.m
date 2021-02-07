#import <Foundation/NSInvocationOperation.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSOperation.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"


int main()
{
  START_SET("NSInvocationOperation - basic")
  NSInvocationOperation *op;
  NSInvocation          *inv1;
  NSInvocation          *inv2;
  NSValue               *val;
  NSInteger             length;
  NSString              *hello = @"hello";
  NSString              *uppercaseHello;
  NSOperationQueue      *queue = [NSOperationQueue new];

  op = [[NSInvocationOperation alloc] initWithTarget: hello
					    selector: @selector(length)
					      object: nil];
  [queue addOperations: [NSArray arrayWithObject: op]
     waitUntilFinished: YES];
  val = [op result];
  [val getValue: &length];
  PASS((length == 5), "Can invoke a selector on a target");
  RELEASE(op);

  inv1 = [NSInvocation invocationWithMethodSignature: 
    [hello methodSignatureForSelector: @selector(uppercaseString)]];
  [inv1 setTarget: hello];
  [inv1 setSelector: @selector(uppercaseString)];
  op = [[NSInvocationOperation alloc] initWithInvocation: inv1];
  inv2 = [op invocation];
  PASS(([inv2 isEqual: inv1]), "Can recover an operation's invocation");
  [queue addOperations: [NSArray arrayWithObject: op]
     waitUntilFinished: YES];
  uppercaseHello = [op result];
  PASS_EQUAL(uppercaseHello, @"HELLO", "Can schedule an NSInvocation");
  RELEASE(op);

  op = [[NSInvocationOperation alloc] initWithTarget: hello
					    selector: @selector(release)
					      object: nil];
  [queue addOperations: [NSArray arrayWithObject: op]
     waitUntilFinished: YES];
  PASS_EXCEPTION(([op result]), NSInvocationOperationVoidResultException, 
    "Can't get result of a void invocation");
  RELEASE(op);

  op = [[NSInvocationOperation alloc] initWithTarget: hello
					    selector: @selector(length)
					      object: nil];
  [op cancel];
  [queue addOperations: [NSArray arrayWithObject: op]
     waitUntilFinished: YES];
  PASS_EXCEPTION(([op result]), NSInvocationOperationCancelledException,
    "Can't get the result of a cancelled invocation");
  RELEASE(op);

  op = [[NSInvocationOperation alloc] initWithTarget: hello
					    selector: @selector(length)
					      object: nil];
  PASS(([op result] == nil),
    "Result is nil before the invocation has completed");
  RELEASE(op);

  RELEASE(queue);
  END_SET("NSInvocationOperation - basic")
  return 0;
}
