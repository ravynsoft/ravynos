#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSProxy.h>
#import <Foundation/NSString.h>

@interface MyString : NSString
{
  id    _remote;
}
@end

@interface MyProxy : NSProxy
{
  id    _remote;
}
@end

@implementation MyString
- (id) init
{
  _remote = nil;
  return self;
}
- (void) dealloc
{
  [_remote release];
  DEALLOC
}
- (unichar) characterAtIndex: (NSUInteger)i
{
  return [_remote characterAtIndex: i];
}
- (NSUInteger) length
{
  return [_remote length];
}
- (void) setRemote:(id)remote
{
  ASSIGN(_remote,remote);
}
- (id) remote
{
  return _remote;
}
@end

@implementation MyProxy
- (id) init
{
  _remote = nil;
  return self;
}
- (void) dealloc
{
  [_remote release];
  DEALLOC
}
- (NSUInteger) hash
{
  if (_remote)
    return [_remote hash];
  else
    return [super hash];
}
- (BOOL) isEqual: (id)other
{
  if (_remote)
    return [_remote isEqual: other];
  else
    return [super isEqual: other];
}
- (void) setRemote:(id)remote
{
  ASSIGN(_remote,remote);
}
- (NSString *) description
{
  return [_remote description];
}
- (id) remote
{
  return _remote;
}
- (NSMethodSignature *) methodSignatureForSelector:(SEL)aSelector
{
  NSMethodSignature *sig = [_remote methodSignatureForSelector:aSelector];
  if (sig == nil)
    sig = [self methodSignatureForSelector:aSelector];
  return sig;
}
- (void) forwardInvocation:(NSInvocation *)inv
{
  [inv setTarget:_remote];
  [inv invoke];
}
@end

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  char *prefix = "The class 'NSProxy' ";
  Class theClass = NSClassFromString(@"NSProxy");
  id obj = nil;
  id rem = @"Remote";
  id sub = nil;
  
  PASS(theClass == [NSProxy class], "uses +class to return self");
  PASS([[NSProxy alloc] isProxy] == YES,
       "%s implements -isProxy to return YES",prefix);
  PASS([[NSProxy alloc] description] != nil, "%s implements -description",prefix);
  obj = [[MyProxy alloc] init];
  PASS(obj != nil, "Can create a MyProxy instance");
  PASS([obj isEqual: obj], "proxy isEqual: to self without remote");
  [obj setRemote: rem];
  PASS([obj remote] == rem, "Can set the remote object for the proxy");
  sub = [[MyString alloc] init];
  PASS(sub != nil, "Can create a MyString instance");
  [sub setRemote: rem];
  PASS([sub remote] == rem, "Can set the remote object for the subclass");
  PASS([obj length] == [rem length], "Get the length of the remote object");
  PASS([sub length] == [rem length], "Get the length of the subclass object");
  PASS([obj isEqual: rem], "proxy isEqual: to remote");
  PASS([obj isEqual: sub], "proxy isEqual: to subclass");
  PASS([sub isEqual: rem], "subclass isEqual: to remote");
  PASS([sub isEqual: obj], "subclass isEqual: to proxy");
  PASS([rem isEqual: obj], "remote isEqual: to proxy");
  PASS([rem isEqual: sub], "remote isEqual: to subclass");
  PASS([obj isEqualToString: rem], "proxy isEqualToString: to remote");
  PASS([obj isEqualToString: sub], "proxy isEqualToString: to subclass");
  PASS([sub isEqualToString: rem], "subclass isEqualToString: to remote");
  PASS([sub isEqualToString: obj], "subclass isEqualToString: to proxy");
  PASS([rem isEqualToString: obj], "remote isEqualToString: to proxy");
  PASS([rem isEqualToString: sub], "remote isEqualToString: to subclass");
  PASS([obj compare: rem] == NSOrderedSame, "proxy compare: remote");
  PASS([obj compare: sub] == NSOrderedSame, "proxy compare: subclass");
  PASS([sub compare: rem] == NSOrderedSame, "subclass compare: remote");
  PASS([sub compare: obj] == NSOrderedSame, "subclass compare: proxy");
  PASS([rem compare: obj] == NSOrderedSame, "remote compare: proxy");
  PASS([rem compare: sub] == NSOrderedSame, "remote compare: subclass");
  
  [arp release]; arp = nil;
  return 0;
}
