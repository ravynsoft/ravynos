#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSConnection.h>
#import <Foundation/NSException.h>
#import <Foundation/NSDebug.h>

#if	GNUSTEP
#import <GNUstepBase/GSObjCRuntime.h>


#define SRV_NAME @"nsmethodsignaturetest"

struct _MyLargeStruct
{
  double first;
  double second;
};
typedef struct _MyLargeStruct MyLargeStruct;

struct _MySmallStruct
{
  char first;
};
typedef struct _MySmallStruct MySmallStruct;

/*------------------------------------*/
@interface MyClass : NSObject
-(void)void_void;
-(id)id_void;

-(char)char_void;
-(unsigned char)uchar_void;
-(signed char)schar_void;

-(short)short_void;
-(unsigned short)ushort_void;
-(signed short)sshort_void;

-(int)int_void;
-(unsigned int)uint_void;
-(signed int)sint_void;

-(long)long_void;
-(unsigned long)ulong_void;
-(signed long)slong_void;

-(float)float_void;
-(double)double_void;

-(MyLargeStruct)largeStruct_void;
-(MySmallStruct)smallStruct_void;



-(void)void_id:(id)_id;

-(void)void_char:(char)_char;
-(void)void_uchar:(unsigned char)_char;
-(void)void_schar:(signed char)_char;

-(void)void_short:(short)_short;
-(void)void_ushort:(unsigned short)_short;
-(void)void_sshort:(signed short)_short;

-(void)void_int:(int)_int;
-(void)void_uint:(unsigned int)_int;
-(void)void_sint:(signed int)_int;

-(void)void_long:(long)_long;
-(void)void_ulong:(unsigned long)_long;
-(void)void_slong:(signed long)_long;

-(void)void_float:(float)_float;
-(void)void_double:(double)_double;

-(void)void_largeStruct:(MyLargeStruct)_str;
-(void)void_smallStruct:(MySmallStruct)_str;

-(void)void_float:(float)_float double:(double)_double;
-(void)void_double:(double)_double float:(float)_float;

-(MyLargeStruct)largeStruct_id:(id)_id
			  char:(char)_char
			 short:(short)_short
			   int:(int)_int
			  long:(long)_long
			 float:(float)_float
			double:(double)_double
		   largeStruct:(MyLargeStruct)_lstr
		   smallStruct:(MySmallStruct)_sstr;
-(MySmallStruct)smallStruct_id:(id)_id
			 uchar:(unsigned char)_uchar
			ushort:(unsigned short)_ushort
			  uint:(unsigned int)_uint
			 ulong:(unsigned long)_ulong
			 float:(float)_float
			double:(double)_double
		   largeStruct:(MyLargeStruct)_lstr
		   smallStruct:(MySmallStruct)_sstr;

-(const char *)runtimeSignatureForSelector:(SEL)selector;
@end

@implementation MyClass
-(void)void_void {}
-(id)id_void { return 0; }

-(char)char_void { return 0; }
-(unsigned char)uchar_void { return 0; }
-(signed char)schar_void { return 0; }

-(short)short_void { return 0; }
-(unsigned short)ushort_void { return 0; }
-(signed short)sshort_void { return 0; }

-(int)int_void { return 0; }
-(unsigned int)uint_void { return 0; }
-(signed int)sint_void { return 0; }

-(long)long_void { return 0; }
-(unsigned long)ulong_void { return 0; }
-(signed long)slong_void { return 0; }

-(float)float_void { return 0; }
-(double)double_void { return 0; }

-(MyLargeStruct)largeStruct_void { MyLargeStruct str; return str; }
-(MySmallStruct)smallStruct_void { MySmallStruct str; return str; }



-(void)void_id:(id)_id {}

-(void)void_char:(char)_char {}
-(void)void_uchar:(unsigned char)_char {}
-(void)void_schar:(signed char)_char {}

-(void)void_short:(short)_short {}
-(void)void_ushort:(unsigned short)_short {}
-(void)void_sshort:(signed short)_short {}

-(void)void_int:(int)_int {}
-(void)void_uint:(unsigned int)_int {}
-(void)void_sint:(signed int)_int {}

-(void)void_long:(long)_long {}
-(void)void_ulong:(unsigned long)_long {}
-(void)void_slong:(signed long)_long {}

-(void)void_float:(float)_float {}
-(void)void_double:(double)_double {}

-(void)void_largeStruct:(MyLargeStruct)_str {}
-(void)void_smallStruct:(MySmallStruct)_str {}

-(void)void_float:(float)_float double:(double)_double {}
-(void)void_double:(double)_double float:(float)_float {}


-(MyLargeStruct)largeStruct_id:(id)_id
			  char:(char)_char
			 short:(short)_short
			   int:(int)_int
			  long:(long)_long
			 float:(float)_float
			double:(double)_double
		   largeStruct:(MyLargeStruct)_lstr
		   smallStruct:(MySmallStruct)_sstr { return _lstr; }

-(MySmallStruct)smallStruct_id:(id)_id
			 uchar:(unsigned char)_uchar
			ushort:(unsigned short)_ushort
			  uint:(unsigned int)_uint
			 ulong:(unsigned long)_ulong
			 float:(float)_float
			double:(double)_double
		   largeStruct:(MyLargeStruct)_lstr
		   smallStruct:(MySmallStruct)_sstr { return _sstr; }

-(const char *)runtimeSignatureForSelector:(SEL)selector
{
  GSMethod meth = GSGetMethod(isa, selector, YES, YES);
  return method_getTypeEncoding (meth);
}

@end

/*------------------------------------*/


/*
   This test is useful if the nsmethodsignatureserver is running which
   was compiled with either a different GNUstep-base version or a different
   version of gcc.  It the server isn't found the test is skipped.
*/
void
test_compare_server_signature(void)
{
  id objct = [MyClass new];
  id proxy = [NSConnection rootProxyForConnectionWithRegisteredName: SRV_NAME
			   host: nil];
  if (proxy)
    {
      const char *rmtSig;
      const char *lclSig;
      const char *msg;

#define TEST_SEL(SELNAME) { \
      BOOL ok; \
      lclSig = [objct runtimeSignatureForSelector: @selector(SELNAME)]; \
      rmtSig = [proxy runtimeSignatureForSelector: @selector(SELNAME)]; \
      msg = [[NSString stringWithFormat: @"runtime: sel:%s\nlcl:%s\nrmt:%s", \
	GSNameFromSelector(@selector(SELNAME)), lclSig, rmtSig] UTF8String]; \
      ok = GSSelectorTypesMatch(lclSig, rmtSig); \
      PASS(ok, "%s", msg) \
      }

      TEST_SEL(void_void);
      TEST_SEL(id_void);
      TEST_SEL(char_void);
      TEST_SEL(uchar_void);
      TEST_SEL(schar_void);
      TEST_SEL(short_void);
      TEST_SEL(ushort_void);
      TEST_SEL(sshort_void);
      TEST_SEL(int_void);
      TEST_SEL(uint_void);
      TEST_SEL(sint_void);
      TEST_SEL(long_void);
      TEST_SEL(ulong_void);
      TEST_SEL(slong_void);
      TEST_SEL(float_void);
      TEST_SEL(double_void);
      TEST_SEL(largeStruct_void);
      TEST_SEL(smallStruct_void);

      TEST_SEL(void_id:);
      TEST_SEL(void_char:);
      TEST_SEL(void_uchar:);
      TEST_SEL(void_schar:);
      TEST_SEL(void_short:);
      TEST_SEL(void_ushort:);
      TEST_SEL(void_sshort:);
      TEST_SEL(void_int:);
      TEST_SEL(void_uint:);
      TEST_SEL(void_sint:);
      TEST_SEL(void_long:);
      TEST_SEL(void_ulong:);
      TEST_SEL(void_slong:);
      TEST_SEL(void_float:);
      TEST_SEL(void_double:);
      TEST_SEL(void_largeStruct:);
      TEST_SEL(void_smallStruct:);
      TEST_SEL(void_float:double:);
      TEST_SEL(void_double:float:);
      TEST_SEL(largeStruct_id:char:short:int:long:float:double:largeStruct:smallStruct:);
      TEST_SEL(smallStruct_id:uchar:ushort:uint:ulong:float:double:largeStruct:smallStruct:);

    }
  else
    {
      NSLog(@"Skipping test_compare_server_signature: proxy not found.");
    }
}

void
test_GSSelectorTypesMatch(void)
{
  const char *pairs[][2] = { {"@@::", "@12@0:4:8"},
			     {"@@::", "@12@+0:+4:+8"},
			     {"@@::", "@12@-0:-4:-8"},
			     {"@12@0:4:8", "@@::"},
			     {"@12@+0:+4:+8", "@@::"},
			     {"@12@-0:-4:-8", "@@::"},

			     {"@12@0:4:8", "@12@+0:+4:+8"},
			     {"@12@0:4:8", "@12@-0:-4:-8"},
			     {"@12@+0:+4:+8", "@12@0:4:8"},
			     {"@12@-0:-4:-8", "@12@0:4:8"},

			     {"@12@0:4:8", "@16@+4:+8:+12"},
			     {"@12@0:4:8", "@16@-4:-8:-12"},
			     {"@12@+0:+4:+8", "@16@4:8:12"},
			     {"@12@-0:-4:-8", "@16@4:8:12"},

/* NB Use of a backslash in a ? ? = sequence below is to prevent the sequence
 * from being interpreted as a trigraph by the compiler/preprocessor.
 */
			     {"{_MyLargeStruct2={_MyLargeStruct=dd}dd}@:",
			      "{?\?={?\?=dd}dd}16@0:4"},

			     {"{_MyLargeStruct=dd}56@+8:+12@+16c+23s+26i+28l24f28d32{_MyLargeStruct=dd}40{_MySmallStruct=c}44",
			      "{_MyLargeStruct=dd}46@+8:+12@+16c+17s+16i+20l+24f+28d24{_MyLargeStruct=dd}32{_MySmallStruct=c}45"},
			     {"{_MyLargeStruct=dd}56@+8:+12@+16c+23s+26i+28l24f28d32{_MyLargeStruct=dd}40{_MySmallStruct=c}44",
			      "{?\?=dd}46@+8:+12@+16c+17s+16i+20l+24f+28d24{?\?=dd}32{?\?=c}45"},
			     {0, 0} };
  unsigned int i = 0;

  while (pairs[i][0])
    {
      const char *s;
      BOOL ok;

      s = [[NSString stringWithFormat: @"pair %d matches:\n%s\n%s",
	i, pairs[i][0], pairs[i][1]] UTF8String];
      ok = GSSelectorTypesMatch(pairs[i][0], pairs[i][1]);
      PASS(ok, "%s", s)
      i++;
    }
}

void
run_server(void)
{
  id obj = [MyClass new];
  NSConnection *conn = [NSConnection defaultConnection];

  [conn setRootObject: obj];
  if ([conn registerName: SRV_NAME] == NO)
    {
      NSLog(@"Failed to register name: " SRV_NAME );
      abort();
    }
  [[NSRunLoop currentRunLoop] run];
}
#endif

@interface      SimpleClass : NSObject
- (const char *) sel1;
@end

@implementation SimpleClass
- (const char *) sel1
{
  return "";
}
@end

int
main(int argc, char *argv[])
{
  NSAutoreleasePool *pool;
  pool = [[NSAutoreleasePool alloc] init];
  const char	*e;
  id    o;
  id    s;

  o = [SimpleClass new];
  s = [o methodSignatureForSelector: @selector(sel1)];
  e = @encode(const char*);
  PASS(strcmp(e, "r*") == 0, "@encode(const char*) makes 'r*' type encoding")
  PASS(strcmp([s methodReturnType], "r*") == 0,
    "sel1 return type is 'r*'")

#if	GNUSTEP
  if ([[[[NSProcessInfo processInfo] arguments] lastObject] isEqual: @"srv"])
    {
      run_server();
      abort();
    }

  NS_DURING
    {
      test_compare_server_signature();
      test_GSSelectorTypesMatch();
    }
  NS_HANDLER
    {
      NSLog(@"MethodSignature Test Failed:");
      NSLog(@"%@ %@ %@",
	    [localException name],
	    [localException reason],
	    [localException userInfo]);
    }
  NS_ENDHANDLER
#endif

  [pool release];

  exit(0);
}

