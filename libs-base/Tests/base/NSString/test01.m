#import "Testing.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  PASS([[@"" pathComponents] count] == 0, "pathComponents ''");
  PASS([[@"usr" pathComponents] count] == 1, "pathComponents 'usr'");
  PASS([[@"usr/" pathComponents] count] == 2, "pathComponents 'usr/'");
  PASS([[@"usr/bin" pathComponents] count] == 2, "pathComponents 'usr/bin'");
  PASS([[@"usr//bin" pathComponents] count] == 2, "pathComponents 'usr//bin'");
  PASS([[@"usr///bin" pathComponents] count] == 2, "pathComponents 'usr///bin'");
  PASS([[@"/" pathComponents] count] == 1, "pathComponents '/'");
  PASS([[@"/usr" pathComponents] count] == 2, "pathComponents '/usr'");
  PASS([[@"/usr/" pathComponents] count] == 3, "pathComponents '/usr/'");
  PASS([[@"/usr/bin" pathComponents] count] == 3, "pathComponents '/usr/bin'");
  PASS([[@"/usr//bin" pathComponents] count] == 3, "pathComponents '/usr//bin'");
  PASS([[@"/usr///bin" pathComponents] count] == 3, "pathComponents '/usr///bin'");
  PASS([[@"" stringByAppendingPathComponent:@""] isEqual:@""],
       "'' stringByAppendingPathComponent: ''");
  PASS([[@"" stringByAppendingPathComponent:@"usr"] isEqual:@"usr"],
       "'' stringByAppendingPathComponent: 'usr'");
  PASS([[@"" stringByAppendingPathComponent:@"usr/"] isEqual:@"usr"],
       "'' stringByAppendingPathComponent: 'usr/'");
  PASS([[@"" stringByAppendingPathComponent:@"/usr"] isEqual:@"/usr"],
       "'' stringByAppendingPathComponent: '/usr'");
  PASS([[@"" stringByAppendingPathComponent:@"/usr/"] isEqual:@"/usr"],
       "'' stringByAppendingPathComponent: '/usr/'");
  PASS([[@"/" stringByAppendingPathComponent:@""] isEqual:@"/"],
       "'/' stringByAppendingPathComponent: ''");
  PASS([[@"/" stringByAppendingPathComponent:@"usr"] isEqual:@"/usr"],
       "'/' stringByAppendingPathComponent: 'usr'");
  PASS([[@"/" stringByAppendingPathComponent:@"usr/"] isEqual:@"/usr"],
       "'/' stringByAppendingPathComponent: 'usr/'");
  PASS([[@"/" stringByAppendingPathComponent:@"/usr"] isEqual:@"/usr"],
       "'/' stringByAppendingPathComponent: '/usr'");
  PASS([[@"/" stringByAppendingPathComponent:@"/usr/"] isEqual:@"/usr"],
       "'/' stringByAppendingPathComponent: '/usr/'");
  PASS([[@"usr" stringByAppendingPathComponent:@""] isEqual:@"usr"],
       "'usr' stringByAppendingPathComponent: ''");
  PASS([[@"usr" stringByAppendingPathComponent:@"bin"] isEqual:@"usr/bin"],
       "'usr' stringByAppendingPathComponent: 'bin'");
  PASS([[@"usr" stringByAppendingPathComponent:@"bin/"] isEqual:@"usr/bin"],
       "'usr' stringByAppendingPathComponent: 'bin/'");
  PASS([[@"usr" stringByAppendingPathComponent:@"/bin"] isEqual:@"usr/bin"],
       "'usr' stringByAppendingPathComponent: '/bin'");
  PASS([[@"usr" stringByAppendingPathComponent:@"/bin/"] isEqual:@"usr/bin"],
       "'usr' stringByAppendingPathComponent: '/bin/'");
  PASS([[@"/usr" stringByAppendingPathComponent:@""] isEqual:@"/usr"],
       "'/usr' stringByAppendingPathComponent: ''");
  PASS([[@"/usr" stringByAppendingPathComponent:@"bin"] isEqual:@"/usr/bin"],
       "'/usr' stringByAppendingPathComponent: 'bin'");
  PASS([[@"/usr" stringByAppendingPathComponent:@"bin/"] isEqual:@"/usr/bin"],
       "'/usr' stringByAppendingPathComponent: 'bin/'");
  PASS([[@"/usr" stringByAppendingPathComponent:@"/bin"] isEqual:@"/usr/bin"],
       "'/usr' stringByAppendingPathComponent: '/bin'");
  PASS([[@"/usr" stringByAppendingPathComponent:@"/bin/"] isEqual:@"/usr/bin"],
       "'/usr' stringByAppendingPathComponent: '/bin/'");
  PASS([[@"/usr/" stringByAppendingPathComponent:@""] isEqual:@"/usr"],
       "'/usr/' stringByAppendingPathComponent: ''");
  PASS([[@"/usr/" stringByAppendingPathComponent:@"bin"] isEqual:@"/usr/bin"],
       "'/usr/' stringByAppendingPathComponent: 'bin'");
  PASS([[@"/usr/" stringByAppendingPathComponent:@"bin/"] isEqual:@"/usr/bin"],
       "'/usr/' stringByAppendingPathComponent: 'bin/'");
  PASS([[@"/usr/" stringByAppendingPathComponent:@"/bin/"] isEqual:@"/usr/bin"],
       "'/usr/' stringByAppendingPathComponent: '/bin/'");
  [arp release]; arp = nil;
  return 0;
}
