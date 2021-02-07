/* Test/example program for the base library

   Copyright (C) 2005 Free Software Foundation, Inc.
   
  Copying and distribution of this file, with or without modification,
  are permitted in any medium without royalty provided the copyright
  notice and this notice are preserved.

   This file is part of the GNUstep Base Library.
*/
#include <Foundation/NSDictionary.h>
#include <Foundation/NSConnection.h>
#include <Foundation/NSPort.h>
#include <Foundation/NSPortNameServer.h>
#include <Foundation/NSDistantObject.h>
#include <Foundation/NSString.h>
#include <Foundation/NSNotification.h>
#include <Foundation/NSData.h>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSProcessInfo.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSException.h>
#include <Foundation/NSProtocolChecker.h>

#define	IN_SERVER	1
#include "server.h"

#include "wgetopt.h"

@implementation Server

- (NSData*) authenticationDataForComponents: (NSMutableArray*)components
{
  unsigned	count = [components count];

  while (count-- > 0)
    {
      id	obj = [components objectAtIndex: count];

      if ([obj isKindOfClass: [NSData class]] == YES)
	{
	  NSMutableData	*d = [obj mutableCopy];
	  unsigned	l = [d length];
	  char		*p = (char*)[d mutableBytes];

	  while (l-- > 0)
	    p[l] ^= 42;
	  [components replaceObjectAtIndex: count withObject: d];
	  RELEASE(d);
	}
    }
  return [NSData data];
}

- init
{
  the_array = [[NSMutableArray alloc] init];
  return self;
}

- (unsigned) count
{
  return [the_array count];
}

- (void) addObject: o
{
  [the_array addObject:o];
}

- objectAt: (unsigned)i
{
  if (i < [the_array count])
    return [the_array objectAtIndex: i];
  else
    return nil;
}

- echoObject: obj
{
  return obj;
}

- (int) exceptionTest1
{
  [NSException raise: @"Test1" format: @"exception1"];
  return 111;
}

- (void) exceptionTest2
{
  [NSException raise: @"Test2" format: @"exception2"];
}

- (void) exceptionTest3
{
  [NSException raise: @"Test3" format: @"exception3"];
}

- print: (const char *)msg
{
  printf(">>%s<<\n", msg);
  fflush(stdout);
  return self;
}

- (BOOL) sendBoolean: (BOOL)b
{
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd), (int)b, (int)(!b));
  fflush(stdout);
  return !b;
}

/* This causes problems, because the runtime encodes this as "*", a string! */
- (void) getBoolean: (BOOL*)bp
{
  BOOL rbp = !(*bp);
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd),
	 (int)*bp, (int)rbp);
  fflush(stdout);
  *bp = rbp;
}

- (unsigned char) sendUChar: (unsigned char)num
{
  unsigned char rnum = num + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd),
	 (int)num, (int)rnum);
  fflush(stdout);
  return rnum;
}

/* This causes problems, because the runtime encodes this as "*", a string! */
- (void) getUChar: (unsigned char *)num
{
  unsigned char rnum = *num + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd),
	 (int)(*num), (int)rnum);
  *num = rnum;
  fflush(stdout);
}

- (char) sendChar: (char)num
{
  char rnum = num + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd),
	 (int)num, (int)rnum);
  fflush(stdout);
  return rnum;
}

- (void) getChar: (char *)num
{
  char rnum = *num + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd),
	 (int)(*num), (int)rnum);
  *num = rnum;
  fflush(stdout);
}

- (short) sendShort: (short)num
{
  short rnum = num + ADD_CONST;
  printf("(%s) got %hd, returning %hd\n", GSNameFromSelector(_cmd),
	 num, rnum);
  fflush(stdout);
  return rnum;
}

- (void) getShort: (short *)num
{
  short rnum = *num + ADD_CONST;
  printf("(%s) got %hd, returning %hd\n", GSNameFromSelector(_cmd),
	 (*num), rnum);
  *num = rnum;
  fflush(stdout);
}

- (int) sendInt: (int)num
{
  int rnum = num + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd), num, rnum);
  fflush(stdout);
  return rnum;
}

- (void) getInt: (int *)num
{
  int rnum = *num + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd), *num, rnum);
  *num = rnum;
  fflush(stdout);
}

- (long) sendLong: (long)num
{
  long rnum = num + ADD_CONST;
  printf("(%s) got %ld, returning %ld\n", GSNameFromSelector(_cmd), num, rnum);
  fflush(stdout);
  return rnum;
}

- (void) getLong: (long *)num
{
  long rnum = *num + ADD_CONST;
  printf("(%s) got %ld, returning %ld\n", GSNameFromSelector(_cmd), *num, rnum);
  *num = rnum;
  fflush(stdout);
}

- (float) sendFloat: (float)num
{
  float rnum = num + ADD_CONST;
  printf("(%s) got %f, returning %f\n", GSNameFromSelector(_cmd), num, rnum);
  fflush(stdout);
  return rnum;
}

- (void) getFloat: (float *)num
{
  float rnum = *num + ADD_CONST;
  printf("(%s) got %f, returning %f\n", GSNameFromSelector(_cmd), *num, rnum);
  *num = rnum;
  fflush(stdout);
}

- (double) sendDouble: (double)num
{
  double rnum = num + ADD_CONST;
  printf("(%s) got %g, returning %g\n", GSNameFromSelector(_cmd), num, rnum);
  fflush(stdout);
  return rnum;
}

- (void) getDouble: (double *)num
{
  double rnum = *num + ADD_CONST;
  printf("(%s) got %g, returning %g\n", GSNameFromSelector(_cmd), *num, rnum);
  *num = rnum;
  fflush(stdout);
}

- (small_struct) sendSmallStruct: (small_struct)str
{
  char rnum = str.z + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd), str.z, rnum);
  fflush(stdout);
  str.z = rnum;
  return str;
}

- (void) getSmallStruct: (small_struct *)str
{
  char rnum = str->z + ADD_CONST;
  printf("(%s) got %d, returning %d\n", GSNameFromSelector(_cmd), str->z, rnum);
  fflush(stdout);
  str->z = rnum;
}

- (foo) sendStruct: (foo)f
{
  foo f2 = {'A', 123.456, 1, "horse", 987654};
  printf("(%s) got c='%c', d=%g, i=%d, s=%s, l=%lu",
    GSNameFromSelector(_cmd), f.c, f.d, f.i, f.s, f.l);
  fflush(stdout);
  printf(" returning c='%c', d=%g, i=%d, s=%s, l=%lu\n",
    f2.c, f2.d, f2.i, f2.s, f2.l);
  fflush(stdout);
  return f2;
}

- (void) getStruct: (foo *)f
{
  foo f2 = {'A', 123.456, 1, "horse", 987654};
  printf("(%s) got c='%c', d=%g, i=%d, s=%s, l=%lu",
    GSNameFromSelector(_cmd), f->c, f->d, f->i, f->s, f->l);
  fflush(stdout);
  printf(" returning c='%c', d=%g, i=%d, s=%s, l=%lu\n",
    f2.c, f2.d, f2.i, f2.s, f2.l);
  fflush(stdout);
  *f = f2;
}

- sendObject: (id)str
{
  printf ("(%s) got object (%s)\n", GSNameFromSelector(_cmd),
    GSClassNameFromObject(str));
  fflush(stdout);
  return str;
}

- (void) getObject: (id *)str
{
  static NSString *ret = @"hello";
  printf ("(%s) got object (%s)\n", GSNameFromSelector(_cmd),
    GSClassNameFromObject(*str));
  *str = ret;
  printf(" returning (%s)\n", [*str cString]);
  fflush(stdout);
}

- (char *) sendString: (char *)str
{
  printf ("(%s) got string (%s)", GSNameFromSelector(_cmd), str);
  str[0] = 'N';
  printf(" returning (%s)\n", str);
  fflush(stdout);
  return str;
}

- (void) getString: (char **)str
{
  static char *ret = "hello";
  printf ("(%s) got string (%s)", GSNameFromSelector(_cmd), *str);
  *str = ret;
  printf(" returning (%s)\n", *str);
  fflush(stdout);
}

- (oneway void) shout
{
  printf ("(%s) got it\n", GSNameFromSelector(_cmd));
  fflush(stdout);
}

/* sender must also respond to 'bounce:count:' */
- bounce: sender count: (int)c
{
  printf ("(%s) got message %d, bouncing back %d", GSNameFromSelector(_cmd), c, c-1);
  fflush(stdout);
  if (--c)
    [sender bounce:self count:c];
  return self;
}

- (void) outputStats:obj
{
  id	c = [obj connectionForProxy];
  id	o = [c statistics];
  id	a = [o allKeys];
  int	j;

  printf("------------------------------------------------------------\n");
  printf("Printing Statistics\n");
  printf("  Number of connections - %d\n", [[NSConnection allConnections] count]);
  printf("  This connection -\n");
  for (j = 0; j < [a count]; j++)
    {
      id k = [a objectAtIndex:j];
      id v = [o objectForKey:k];
      printf("  %s - %s\n", [k cString], [[v description] cString]);
    }
  printf("------------------------------------------------------------\n");
  fflush(stdout);
}

/* Doesn't work because GCC generates the wrong encoding: "@0@+8:+12^i+16" */
- sendArray: (int[3])a
{
  printf("  >> array %d %d %d\n", a[0], a[1], a[2]);
  fflush(stdout);
  return self;
}

- sendStructArray: (struct myarray)ma
{
  printf("  >>struct array %d %d %d\n", ma.a[0], ma.a[1], ma.a[2]);
  fflush(stdout);
  return self;
}

- sendDouble: (double)d andFloat: (float)f
{
  printf("(%s) got double %f, float %f\n", GSNameFromSelector(_cmd), d, f);
  fflush(stdout);
  return self;
}

- quietBycopy: (bycopy id)o
{
  printf("  >> quiet bycopy class is %s\n", GSClassNameFromObject(o));
  fflush(stdout);
  return self;
}

- sendBycopy: (bycopy id)o
{
  printf("  >> bycopy class is %s\n", GSClassNameFromObject(o));
  fflush(stdout);
  return self;
}

#ifdef	_F_BYREF
- sendByref: (byref id)o
{
  printf("  >> byref class is %s\n", GSClassNameFromObject(o));
  fflush(stdout);
  return self;
}
- modifyByref: (byref NSMutableString *)o
{
  printf("  >> byref class is %s\n", GSClassNameFromObject(o));
  fflush(stdout);
  [o appendString: @"hello"];
  return self;
}
#endif

- manyArgs: (int)i1 : (int)i2 : (int)i3 : (int)i4 : (int)i5 : (int)i6
: (int)i7 : (int)i8 : (int)i9 : (int)i10 : (int)i11 : (int)i12
{
  printf("manyArgs: got %d %d %d %d %d %d %d %d %d %d %d %d\n",
	 i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12);
  fflush(stdout);
  return self;
}

- connectionBecameInvalid: notification
{
  id anObj = [notification object];
  if ([anObj isKindOfClass: [NSConnection class]])
    {
      int i, count = [the_array count];
      for (i = count-1; i >= 0; i--)
	{
	  id o = [the_array objectAtIndex: i];
	  if ([o isProxy] && [o connectionForProxy] == anObj)
	    [the_array removeObjectAtIndex: i];
	}
      if (count != [the_array count])
	printf("$$$$$ connectionBecameInvalid: removed from the_array\n");
    }
  else
    {
      [self error:"non Connection is invalid"];
    }
  return self;
}

- (NSConnection*) connection: ancestor didConnect: newConn
{
  printf("%s\n", GSNameFromSelector(_cmd));
  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(connectionBecameInvalid:)
    name: NSConnectionDidDieNotification
    object: newConn];
  [newConn setDelegate: self];
  return newConn;
}

- (oneway void) registerClient: (id<ClientProtocol>)client
{
  ASSIGN(registered_client, client);
}

- (oneway void) unregisterClient: (id<ClientProtocol>)client
{
  DESTROY(registered_client);
}

- (BOOL) tryClientCallback
{
  return [registered_client callback];
}

@end

void
usage(const char *program)
{
  printf("Usage: %s [-d -t#] [server_name]\n", program);
  printf("  -d     - Debug connection\n");
  printf("  -t     - Timeout after # seconds\n");
}

int main(int argc, char *argv[], char **env)
{
  int i, debug, timeout;
  id s = [[Server alloc] init];
  id l;
  id o = [[NSObject alloc] init];
  NSConnection *c;
  NSPortNameServer *ns;
  NSPort *port;
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
#if	!defined(__MINGW__)
  extern int optind;
  extern char *optarg;
#endif

  l = [NSProtocolChecker protocolCheckerWithTarget: s
					  protocol: @protocol(ServerProtocol)];

  [NSProcessInfo initializeWithArguments: argv count: argc environment: env];
  debug = 0;
  timeout = 0;
  while ((i = wgetopt(argc, argv, "hdt:")) != EOF)
    switch (i)
      {
      case 'd':
	debug++;
	break;
      case 't':
	timeout = atoi(optarg);;
	break;
      case 'h':
	usage(argv[0]);
	exit(0);
	break;
      default:
#if 0
	usage(argv[0]);
	exit(1);
#endif
	break;
      }

#if NeXT_runtime
  [NSDistantObject setProtocolForProxies:@protocol(AllProxies)];
#endif

  if (debug > 0)
    {
      [NSDistantObject setDebug: debug];
      [NSConnection setDebug: debug];
      [NSObject enableDoubleReleaseCheck: YES];
    }
  ns = [NSSocketPortNameServer sharedInstance];
  port = [NSSocketPort port];
  c = [NSConnection connectionWithReceivePort: port sendPort: port];
  [c setRootObject: l];

  if (optind < argc)
    [c registerName: [NSString stringWithUTF8String: argv[optind]]
      withNameServer: ns];
  else
    [c registerName: @"test2server" withNameServer: ns];

  [[NSNotificationCenter defaultCenter]
    addObserver: s
    selector: @selector(connectionBecameInvalid:)
    name: NSConnectionDidDieNotification
    object: c];
  [c setDelegate: s];

  [s addObject: o];
  printf("  list's hash is 0x%x\n", (unsigned)[l hash]);
  printf("  object's hash is 0x%x\n", (unsigned)[o hash]);
  printf("Running...\n");

  if (timeout)
    [[NSRunLoop currentRunLoop] runUntilDate:
      [NSDate dateWithTimeIntervalSinceNow: timeout]];
  else
    [[NSRunLoop currentRunLoop] run];
  printf("Finished\n");

  [arp release];
  exit(0);
}
