#import <Foundation/NSObject.h>

@class NSMutableDictionary;

@interface CATransactionGroup : NSObject {
    NSMutableDictionary *_values;
}

- valueForKey:(NSString *)key;
- (void)setValue:value forKey:(NSString *)key;

@end
