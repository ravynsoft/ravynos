#import <CoreGraphics/CGAffineTransform.h>
#import <CoreGraphics/CGFunction.h>
#import <CoreGraphics/CGPath.h>
#import <CoreGraphics/CGPattern.h>
#import <Onyx2D/O2AffineTransform.h>
#import <Onyx2D/O2Function.h>
#import <Onyx2D/O2Pattern.h>
#import <Onyx2D/O2Path.h>

static inline CGAffineTransform CGAffineTransformFromO2(O2AffineTransform xform) {
    CGAffineTransform result = {xform.a, xform.b, xform.c, xform.d, xform.tx, xform.ty};

    return result;
}

static inline O2AffineTransform O2AffineTransformFromCG(CGAffineTransform xform) {
    O2AffineTransform result = {xform.a, xform.b, xform.c, xform.d, xform.tx, xform.ty};

    return result;
}

static inline const O2AffineTransform *O2AffineTransformPtrFromCG(const CGAffineTransform *xform) {
    return (const O2AffineTransform *)xform;
}

static inline const O2FunctionCallbacks *O2FunctionCallbacksFromCG(const CGFunctionCallbacks *callbacks) {
    return (const O2FunctionCallbacks *)callbacks;
}

static inline const O2PatternCallbacks *O2PatternCallbacksFromCG(const CGPatternCallbacks *callbacks) {
    return (const O2PatternCallbacks *)callbacks;
}

static inline O2PathApplierFunction O2PathApplierFunctionFromCG(const CGPathApplierFunction function) {
    return (O2PathApplierFunction)function;
}
