#import <Foundation/NSObject.h>
#include <pthread.h>

@class NSMutableDictionary, NSMutableArray, NSArray, NSKeyObserver;

@interface NSKVOInfoPerObject : NSObject {
    pthread_mutex_t _lock;
    NSMutableDictionary *_dictionary;
}

- init;

- (BOOL)isEmpty;

- objectForKey:key;
- (void)setObject:object forKey:key;

- (NSArray *)keyObserversForKey:(NSString *)key;
- (void)addKeyObserver:(NSKeyObserver *)keyObserver;
- (void)removeKeyObserver:(NSKeyObserver *)keyObserver;

@end
