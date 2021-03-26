#import <Onyx2D/O2Context.h>

// This is used by PDFKit, it should be possible to implement this functionality
// using the exposed CG* PDF API's with the operator table, but it is more work

@interface O2Context_distill : O2Context {
    id _delegate;
}

- delegate;
- (void)setDelegate:delegate;

@end

@interface NSObject (O2Context_distill)

- (void)distiller:(O2Context_distill *)distiller unicode:(unichar *)unicode rects:(O2Rect *)rect count:(NSUInteger)count;

@end
