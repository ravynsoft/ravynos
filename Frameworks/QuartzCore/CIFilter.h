#import <Foundation/NSObject.h>

@class NSMutableDictionary;

@interface CIFilter : NSObject {
    NSMutableDictionary *_keyValues;
}

+ (CIFilter *)filterWithName:(NSString *)name;
- (void)setDefaults;

- valueForKey:(NSString *)key;
- (void)setValue:value forKey:(NSString *)key;

@end
