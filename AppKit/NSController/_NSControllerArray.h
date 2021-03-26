#import <Foundation/NSMutableArray.h>

@class NSMutableIndexSet;

@interface _NSControllerArray : NSMutableArray {
    NSMutableArray *_array;
    NSMutableArray *_observationProxies;
    NSMutableIndexSet *_roi;
}

@end
