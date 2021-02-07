/* Test/example program for the base library

   Copyright (C) 2005 Free Software Foundation, Inc.
   
  Copying and distribution of this file, with or without modification,
  are permitted in any medium without royalty provided the copyright
  notice and this notice are preserved.

   This file is part of the GNUstep Base Library.
*/
#ifndef _server_h
#define _server_h

#include <Foundation/NSConnection.h>

@interface	NSConnection (Debug)
+ (int) setDebug: (int)aFlag;
@end
@interface	NSDistantObject (Debug)
+ (int) setDebug: (int)aFlag;
@end

typedef struct _small_struct {
  unsigned char z;
} small_struct;

typedef struct _foo {
  char c;
  double d;
  int i;
  char *s;
  unsigned long l;
} foo;

struct myarray {
  int a[3];
};

#define ADD_CONST 47
 
@protocol ClientProtocol 
- (BOOL) callback;
@end

@protocol ServerProtocol 
- (void) addObject: (id)o;
- (BOOL) sendBoolean: (BOOL)b;
- (void) getBoolean: (BOOL*)bp;
- (unsigned char) sendUChar: (unsigned char)uc;
- (void) getUChar: (unsigned char *)ucp;
- (char) sendChar: (char)uc;
- (void) getChar: (char *)ucp;
- (short) sendShort: (short)num;
- (void) getShort: (short *)num;
- (int) sendInt: (int)num;
- (void) getInt: (int *)num;
- (long) sendLong: (long)num;
- (void) getLong: (long *)num;
- (float) sendFloat: (float)num;
- (void) getFloat: (float *)num;
- (double) sendDouble: (double)num;
- (void) getDouble: (double *)num;
- sendDouble: (double)dbl andFloat: (float)flt;

- (small_struct) sendSmallStruct: (small_struct)str;
- (void) getSmallStruct: (small_struct *)str;
- (foo) sendStruct: (foo)str;
- (void) getStruct: (foo *)str;
- (id) sendObject: (id)str;
- (void) getObject: (id *)str;
- (char *) sendString: (char *)str;
- (void) getString: (char **)str;

- print: (const char *)str;

- objectAt: (unsigned)i;
- (unsigned) count;
- echoObject: obj;

- (oneway void) shout;
- bounce: sender count: (int)c;
- (oneway void) outputStats:obj;

- sendArray: (int[3])a;
- sendStructArray: (struct myarray)ma;

- sendBycopy: (bycopy id)o;
#ifdef	_F_BYREF
- sendByref: (byref id)o;
- modifyByref: (byref NSMutableString *)o;
#endif
- manyArgs: (int)i1 : (int)i2 : (int)i3 : (int)i4 : (int)i5 : (int)i6
: (int)i7 : (int)i8 : (int)i9 : (int)i10 : (int)i11 : (int)i12;
- (int) exceptionTest1;
- (void) exceptionTest2;
- (oneway void) exceptionTest3;

- (oneway void) registerClient: (id<ClientProtocol>)client;
- (oneway void) unregisterClient: (id<ClientProtocol>)client;
- (BOOL) tryClientCallback;
@end

#ifdef	IN_SERVER
/*
 * We don't want the client to know about some methods, so we can
 * check that they work when it doesn't know them.
 */
@protocol privateServer
- quietBycopy: (bycopy id)o;
@end
@interface Server : NSObject <ServerProtocol,privateServer>
{
  id the_array;
  id<ClientProtocol> registered_client;
}
@end
#else
@interface Server : NSObject <ServerProtocol>
{
  id the_array;
  id<ClientProtocol> registered_client;
}
@end
#endif

#endif /* _server_h */
