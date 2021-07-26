#import <Foundation/NSObject.h>
#import <Foundation/NSKeyValueObserving.h>

@class NSMutableDictionary;

@interface NSKeyPathObserver : NSObject {
    id _object;
    id _observer;
    NSString *_keyPath;
    NSKeyValueObservingOptions _options;
    void *_context;

    NSInteger _willChangeCount;
    NSMutableDictionary *_changeDictionary;
}

- initWithObject:object observer:observer keyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(void *)context;

- object;
- observer;
- (NSString *)keyPath;
- (NSKeyValueObservingOptions)options;
- (void *)context;

- (BOOL)willChangeAlreadyChanging;
- (BOOL)didChangeAlreadyChanging;

- (NSMutableDictionary *)changeDictionaryWithInfo:(NSDictionary *)info;
- (NSMutableDictionary *)changeDictionary;
- (void)clearChangeDictionary;

@end
