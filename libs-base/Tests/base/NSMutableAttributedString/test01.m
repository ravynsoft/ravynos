#import "Testing.h"
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSAutoreleasePool.h>

/* get rid of compiler warnings */
@interface NSMutableAttributedString(evil)
-(void) _sanity;
@end
#if	!defined(GNUSTEP_BASE_LIBRARY)
@implementation NSMutableAttributedString(evil)
- (void) _sanity
{
}
@end
#endif

@interface NSMutableAttributedString (TestingAdditions)
-(BOOL)checkAttributes:(NSDictionary *)attr location:(int)location;
-(BOOL)checkAttributes:(NSDictionary *)attr range:(NSRange)range;
@end

@implementation NSMutableAttributedString (TestingAdditions)
-(BOOL)checkAttributes:(NSDictionary *)attr location:(int)loc
{
  return [[self attributesAtIndex:loc
               effectiveRange:NULL] isEqual:attr];
}

-(BOOL)checkAttributes:(NSDictionary *)attr range:(NSRange)range
{
  NSRange aRange = range;

  while (aRange.length > 0)
    {
      BOOL attrEqual;
      attrEqual= [[self attributesAtIndex:aRange.location + (aRange.length - 1)
                           effectiveRange:NULL] isEqual:attr];
      if (attrEqual == NO)
        return NO;

      aRange.length -= 1;
    }
  return YES;
}
@end

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableAttributedString *as;
  NSString *base1 = @"base-1";
  NSString *base2 = @"base-2";
  NSDictionary *attrE, *attr1, *attr2;
  int start,length,index;
  
  [[[NSMutableAttributedString new] autorelease] _sanity]; 

  as = [[NSMutableAttributedString alloc] initWithString:base1 attributes:nil];
  [as replaceCharactersInRange:NSMakeRange(2,2) withString:@""];
  [as _sanity];
  PASS([[as string] isEqual:@"ba-1"], 
       "-replaceCharactersInRange: withString: works with zero length string");

  as = [[NSMutableAttributedString alloc] initWithString:base1 attributes:nil];
  [as replaceCharactersInRange:NSMakeRange(2,2) withString:base2];
  [as _sanity];
  PASS([[as string] isEqual:@"babase-2-1"], 
       "-replaceCharactersInRange:withString: works in middle of string");

  as = [[NSMutableAttributedString alloc] initWithString:base1 attributes:nil];
  [as replaceCharactersInRange:NSMakeRange(6,0) withString:base2];
  [as _sanity];
  PASS([[as string] isEqual:@"base-1base-2"], 
       "-replaceCharactersInRange:withString: works at end of string works");
  
  as = [[NSMutableAttributedString alloc] initWithString:base1 attributes:nil];
  [as replaceCharactersInRange:NSMakeRange(0,0) withString:base2];
  [as _sanity];
  PASS([[as string] isEqual:@"base-2base-1"], 
       "-replaceCharactersInRange:withString: works at start of string works");
  
  attrE = [NSDictionary dictionary]; 
  attr1 = [NSDictionary dictionaryWithObject:@"a" forKey:@"1"];
  attr2 = [NSDictionary dictionaryWithObject:@"b" forKey:@"2"];
  as = [[NSMutableAttributedString alloc] initWithString:base1
                                              attributes:attr1];
  [as setAttributes:attr2 range:NSMakeRange(2,4)];
  [as replaceCharactersInRange:NSMakeRange(0,6) withString:@""];
  [as replaceCharactersInRange:NSMakeRange(0,0) withString:@"aa"];
  [as replaceCharactersInRange:NSMakeRange(2,0) withString:@"bb"];
  [as _sanity];
  PASS([as checkAttributes:attrE range:NSMakeRange(0,4)],
       "-replaceCharactersInRange:withString: keeps attributes if entire string is replaced");

  as = [[NSMutableAttributedString alloc] initWithString:base1
                                              attributes:attr1];
  [as replaceCharactersInRange:NSMakeRange(0,6) withString:base2];
  [as _sanity];
  PASS([[as string] isEqual:base2] &&
       [as checkAttributes:attr1 range:NSMakeRange(0,6)],
       "-replaceCharactersInRange:withString: keeps attributes if entire string is replaced");
  
  for (start=0;start != 9; start++)
    { 
      for (length = 0; (length + start) != 9; length++)
        { 
	 
	  BOOL removeAll,replaceAll;
	  NSDictionary *aBegin,*aEnd;
	  as = [[NSMutableAttributedString alloc] initWithString:@"aabbccdd" 
	                                              attributes:attr2];
          removeAll = (start == 0 && length == 8);
          [as setAttributes:attr1 range:NSMakeRange(2,2)];
          [as setAttributes:attrE range:NSMakeRange(4,2)];
          
	  if (removeAll)
	    {
              aBegin = attrE;
	      aEnd = attrE;
	    }
	  else 
	    {
              aBegin = [as attributesAtIndex: (start == 0) ? length : 0
	                      effectiveRange:NULL];
	      aEnd = [as attributesAtIndex: ((start + length) == 8) ? (start - 1) : 8
	                    effectiveRange:NULL];

              [as replaceCharactersInRange:NSMakeRange(start, length)
	                        withString:@""];
	      [as _sanity];
	      PASS([[as string] length] == (8 - length) &&
	           [as checkAttributes:aBegin location:0] &&
		   [as checkAttributes:aEnd location: (8 - length)],
		   "attribute/(replaceCharacters... with zero length string) interaction _sanity checks %i %i",start, length);
		   
	    }
	    as = [[NSMutableAttributedString alloc] initWithString:@"aabbccdd" 
	                                                attributes:attr2];
            replaceAll = (start == 0 && length == 8);
            [as setAttributes:attr1 range:NSMakeRange(2,2)];
            [as setAttributes:attrE range:NSMakeRange(4,2)];
            if (length == 0 && start == 0)
	      index = 0;
	    else if (length == 0)
	      index = (start - 1);
	    else
	      index = start;
	    
	    aBegin = [as attributesAtIndex:index effectiveRange:NULL];
	    [as replaceCharactersInRange:NSMakeRange(start,length) 
	                      withString:@"foo"];
	    [as _sanity];
	    PASS([[as string] length] == (11 - length) &&
	         [as checkAttributes:aBegin range:NSMakeRange(start,3)],
           	 "attribute/replaceCharacters... interaction _sanity checks %i %i",start,length);
	      
	}
    }

  [arp release]; arp = nil;
  return 0;
}
