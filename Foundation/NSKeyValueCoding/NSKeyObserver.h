#import <Foundation/NSObject.h>

@class NSKeyPathObserver, NSArray;

@interface NSKeyObserver : NSObject {
    id _object;
    NSString *_key;
    NSKeyPathObserver *_keyPathObserver;
    NSString *_branchPath;
    NSKeyObserver *_branchObserver;
    NSArray *_dependantKeyObservers;
    BOOL _isValid;
}

- initWithObject:object key:(NSString *)key keyPathObserver:(NSKeyPathObserver *)keyPathObserver restOfPath:(NSString *)restOfPath;

- (BOOL)isValid;
- (void)invalidate;

- object;
- (NSString *)key;
- (NSKeyPathObserver *)keyPathObserver;

- (NSString *)restOfPath;

- (NSKeyObserver *)restOfPathObserver;
- (void)setRestOfPathObserver:(NSKeyObserver *)keyObserver;

- (NSArray *)dependantKeyObservers;
- (void)setDependantKeyObservers:(NSArray *)keyObservers;

@end
