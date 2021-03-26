#import <Foundation/NSObject.h>

@protocol NSChangeSpelling
- (void)changeSpelling:sender;
@end

@protocol NSIgnoreMisspelledWords
- (void)ignoreSpelling:sender;
@end
