#import <CoreData/NSEntityDescription.h>

@interface NSEntityDescription (private)
- (BOOL)_isKindOfEntity:(NSEntityDescription *)other;
- (NSPropertyDescription *)_propertyForSelector:(SEL)selector;
@end
