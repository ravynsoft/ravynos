#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2AffineTransform.h>

#ifdef __cplusplus
extern "C" {
#endif

@class O2Path, O2Image;

typedef enum {
    O2ClipPhaseNonZeroPath,
    O2ClipPhaseEOPath,
    O2ClipPhaseMask,
} O2ClipPhaseType;

@interface O2ClipPhase : NSObject {
    O2ClipPhaseType _type;
    id _object;
    O2Rect _rect;
    O2AffineTransform _transform;
}

O2ClipPhase *O2ClipPhaseInitWithNonZeroPath(O2ClipPhase *self, O2Path *path);
- initWithEOPath:(O2Path *)path;
- initWithMask:(O2Image *)mask rect:(O2Rect)rect transform:(O2AffineTransform)transform;

O2ClipPhaseType O2ClipPhasePhaseType(O2ClipPhase *self);
id O2ClipPhaseObject(O2ClipPhase *self);

@end

#ifdef __cplusplus
}
#endif
