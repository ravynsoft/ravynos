#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  char *str1,*str2;
  NSData *data1, *data2;
  NSMutableData *mutable;
  char *hold;

  str1 = "Test string for data classes";
  str2 = (char *) malloc(sizeof("Test string for data classes not copied"));
  strcpy(str2,"Test string for data classes not copied");

  mutable = [NSMutableData dataWithLength:100];
  hold = [mutable mutableBytes];

  /* hmpf is this correct */
  data1 = [NSData dataWithBytes:str1 length:(strlen(str1) * sizeof(void*))];
  PASS(data1 != nil &&
       [data1 isKindOfClass:[NSData class]] &&
       [data1 length] == (strlen(str1) * sizeof(void*)) &&
       [data1 bytes] != str1 &&
       strcmp(str1,[data1 bytes]) == 0,
       "+dataWithBytes:length: works");

  data2 = [NSData dataWithBytesNoCopy:str2 length:(strlen(str2) * sizeof(void*))];
  PASS(data2 != nil && [data2 isKindOfClass:[NSData class]] &&
       [data2 length] == (strlen(str2) * sizeof(void*)) &&
       [data2 bytes] == str2,
       "+dataWithBytesNoCopy:length: works");

  data1 = [NSData dataWithBytes:nil length:0];
  PASS(data1 != nil && [data1 isKindOfClass:[NSData class]] &&
       [data1 length] == 0,
       "+dataWithBytes:length works with 0 length");

  [data2 getBytes:hold range:NSMakeRange(2,6)];
  PASS(strcmp(hold,"st str") == 0, "-getBytes:range works");

  PASS_EXCEPTION([data2 getBytes:hold
                           range:NSMakeRange(strlen(str2)*sizeof(void*),1)];,
                 NSRangeException,
		 "getBytes:range: with bad location");

  PASS_EXCEPTION([data2 getBytes:hold
                           range:NSMakeRange(1,(strlen(str2)*sizeof(void*)))];,
                 NSRangeException,
		 "getBytes:range: with bad length");

  PASS_EXCEPTION([data2 subdataWithRange:NSMakeRange((strlen(str2)*sizeof(void*)),1)];,
                 NSRangeException,
		 "-subdataWithRange: with bad location");

  PASS_EXCEPTION([data2 subdataWithRange:NSMakeRange(1,(strlen(str2)*sizeof(void*)))];,
                 NSRangeException,
		 "-subdataWithRange: with bad length");

  data2 = [NSData dataWithBytesNoCopy:str1
                               length:(strlen(str1) * sizeof(void*))
			 freeWhenDone:NO];
  PASS(data2 != nil && [data2 isKindOfClass:[NSData class]] &&
       [data2 length] == (strlen(str1) * sizeof(void*)) &&
       [data2 bytes] == str1,
       "+dataWithBytesNoCopy:length:freeWhenDone: works");

  [arp release]; arp = nil;

  {
    BOOL didNotSegfault = YES;
    PASS(didNotSegfault, "+dataWithBytesNoCopy:length:freeWhenDone:NO doesn't free memory");
  }


  START_SET("deallocator blocks")
  # ifndef __has_feature
  # define __has_feature(x) 0
  # endif
  # if __has_feature(blocks)
  uint8_t stackBuf[4] = { 1, 2, 3, 5 };
  __block NSUInteger called = 0;
  NSData *immutable =
    [[NSData alloc] initWithBytesNoCopy: stackBuf
                                 length: 4
                            deallocator: ^(void* bytes, NSUInteger length) {
      called++;
  }];
  PASS_RUNS([immutable release]; immutable = nil;,
      "No free() error with custom deallocator");
  PASS(called == 1, "Deallocator block called");
  uint8_t *buf = malloc(4 * sizeof(uint8_t));
  NSMutableData *mutable =
    [[NSMutableData alloc] initWithBytesNoCopy: buf
                                        length: 4
                                   deallocator: ^(void *bytes, NSUInteger len)
                                   {
                                      free(bytes);
                                      called++;
                                   }
  ];
  PASS_RUNS([mutable release]; mutable = nil;,
    "No free() error with custom deallocator on mutable data");
  PASS(called == 2, "Deallocator block called on -dealloc of mutable data");
  buf = malloc(4 * sizeof(uint8_t));
  mutable =
    [[NSMutableData alloc] initWithBytesNoCopy: buf
                                        length: 4
                                   deallocator: ^(void *bytes, NSUInteger len)
                                    {
                                       free(bytes);
                                       called++;
                                    }
   ];
  PASS_RUNS([mutable setCapacity: 10];,
    "Can set capactiy with custom deallocator on mutable data");
  PASS(called == 3,
      "Deallocator block called on -setCapacity: of mutable data");
  PASS_RUNS([mutable release]; mutable = nil;,
    "No free() error with custom deallocator on mutable data "
    "after capacity change");
   PASS(called == 3, "Deallocator block not called on -dealloc of mutable data "
     "after its capacity has been changed");
  # else
    SKIP("No Blocks support in the compiler.")
  # endif
    END_SET("deallocator blocks")
  return 0;
}
