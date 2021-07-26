#import <Foundation/NSNumber.h>
#import <CoreFoundation/CFNumber.h>

@interface NSNumber_CF : NSNumber {
  @public
    CFNumberType _type;
}

@end
