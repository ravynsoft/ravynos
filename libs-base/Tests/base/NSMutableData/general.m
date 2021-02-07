#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>

int main()
{ 
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  char *str1,*str2,*tmp;
  NSData *data1;
  NSMutableData *mutable;
  unsigned char *hold;
   
  str1 = "Test string for data classes";
  str2 = (char *) malloc(sizeof("insert")); 
  strcpy(str2,"insert");
  
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
  
  mutable = [NSMutableData data];
  PASS(mutable != nil && 
       [mutable isKindOfClass:[NSMutableData class]] &&
       [mutable length] == 0,
       "+data creates empty mutable data");
  
  [mutable setData:data1];
  PASS(mutable != nil &&
       [mutable length] == (strlen(str1) * sizeof(void*)),
       "-setData: works");
  
  [mutable replaceBytesInRange:NSMakeRange(22,6) withBytes:str2];
  tmp = (char *)malloc([mutable length]);
  [mutable getBytes:tmp range:NSMakeRange(22,6)];
  tmp[6] = '\0';
  PASS(mutable != nil &&
       strcmp(tmp,str2) == 0,
       "-replaceBytesInRange:withBytes suceeds");
  free(tmp);
  PASS_EXCEPTION([mutable replaceBytesInRange:NSMakeRange([mutable length]+1,6) 
                                    withBytes:str2];,
		 NSRangeException,"-replaceBytesInRange:withBytes out of range raises exception");
                 
  
  [arp release]; arp = nil;
  return 0;
}
