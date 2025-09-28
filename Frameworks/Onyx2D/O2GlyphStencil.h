#import <Foundation/NSObject.h>

typedef struct O2GlyphStencil {
    size_t _width;
    size_t _height;
    size_t _left;
    size_t _top;
    uint8_t *_coverage;
} * O2GlyphStencilRef;

O2GlyphStencilRef O2GlyphStencilCreate(size_t width, size_t height, uint8_t *coverage, size_t bytesPerRow, size_t left, size_t top);
void O2GlyphStencilDealloc(O2GlyphStencilRef self);

size_t O2GlyphStencilGetWidth(O2GlyphStencilRef self);
size_t O2GlyphStencilGetHeight(O2GlyphStencilRef self);
size_t O2GlyphStencilGetLeft(O2GlyphStencilRef self);
size_t O2GlyphStencilGetTop(O2GlyphStencilRef self);
uint8_t *O2GlyphStencilGetCoverage(O2GlyphStencilRef self);
