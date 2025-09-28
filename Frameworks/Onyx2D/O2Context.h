/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>

#ifdef __cplusplus
extern "C" {
#endif

@class O2Context, O2Color, O2Shading, O2Image, O2GState, O2MutablePath, O2Path, O2Pattern, O2Layer, O2PDFPage, NSMutableArray, CGWindow, O2Surface, NSDictionary, NSData, O2Font, O2Encoding, O2PDFCharWidths, O2ClipState;

typedef O2Context *O2ContextRef;

typedef enum {
    kO2EncodingFontSpecific,
    kO2EncodingMacRoman,
    // private
    kO2EncodingMacExpert,
    kO2EncodingWinAnsi,
    kO2EncodingUnicode,
} O2TextEncoding;

typedef enum {
    kO2LineCapButt,
    kO2LineCapRound,
    kO2LineCapSquare,
} O2LineCap;

typedef enum {
    kO2LineJoinMiter,
    kO2LineJoinRound,
    kO2LineJoinBevel,
} O2LineJoin;

typedef enum {
    kO2PathFill,
    kO2PathEOFill,
    kO2PathStroke,
    kO2PathFillStroke,
    kO2PathEOFillStroke
} O2PathDrawingMode;

typedef enum {
    kO2InterpolationDefault,
    kO2InterpolationNone,
    kO2InterpolationLow,
    kO2InterpolationHigh,
} O2InterpolationQuality;

typedef enum {
    // seperable
    kO2BlendModeNormal,
    kO2BlendModeMultiply,
    kO2BlendModeScreen,
    kO2BlendModeOverlay,
    kO2BlendModeDarken,
    kO2BlendModeLighten,
    kO2BlendModeColorDodge,
    kO2BlendModeColorBurn,
    kO2BlendModeHardLight,
    kO2BlendModeSoftLight,
    kO2BlendModeDifference,
    kO2BlendModeExclusion,
    // nonseperable
    kO2BlendModeHue,
    kO2BlendModeSaturation,
    kO2BlendModeColor,
    kO2BlendModeLuminosity,
    // Porter-Duff
    kO2BlendModeClear,
    kO2BlendModeCopy,
    kO2BlendModeSourceIn,
    kO2BlendModeSourceOut,
    kO2BlendModeSourceAtop,
    kO2BlendModeDestinationOver,
    kO2BlendModeDestinationIn,
    kO2BlendModeDestinationOut,
    kO2BlendModeDestinationAtop,
    kO2BlendModeXOR,
    kO2BlendModePlusDarker,
    kO2BlendModePlusLighter,
} O2BlendMode;

typedef enum {
    // These correspond directly to PDF spec. values
    kO2TextFill = 0,
    kO2TextStroke = 1,
    kO2TextFillStroke = 2,
    kO2TextInvisible = 3,
    kO2TextFillClip = 4,
    kO2TextStrokeClip = 5,
    kO2TextFillStrokeClip = 6,
    kO2TextClip = 7,
} O2TextDrawingMode;

#import <Onyx2D/O2Font.h>
#import <Onyx2D/O2Layer.h>
#import <Onyx2D/O2Color.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Image.h>
#import <Onyx2D/O2Path.h>
#import <Onyx2D/O2Pattern.h>
#import <Onyx2D/O2Shading.h>
#import <Onyx2D/O2PDFPage.h>

typedef void (*O2ContextShowTextFunction)(O2ContextRef, const char *, unsigned);
typedef void (*O2ContextShowGlyphsFunction)(O2ContextRef, SEL, const O2Glyph *, const O2Size *, unsigned);

@interface O2Context : NSObject {
    O2AffineTransform _userToDeviceTransform;
    NSMutableArray *_layerStack;
    NSMutableArray *_stateStack;
    O2GState *_currentState;
    O2MutablePath *_path;
    BOOL _allowsAntialiasing;
    O2AffineTransform _textMatrix;
    O2AffineTransform _textLineMatrix;

    O2ContextShowTextFunction _showTextFunction;
    O2ContextShowGlyphsFunction _showGlyphsFunction;
}

+ (O2Context *)createContextWithSize:(O2Size)size window:(CGWindow *)window;
+ (O2Context *)createBackingContextWithSize:(O2Size)size context:(O2Context *)context deviceDictionary:(NSDictionary *)deviceDictionary;

+ (BOOL)canInitWithWindow:(CGWindow *)window;
+ (BOOL)canInitBackingWithContext:(O2Context *)context deviceDictionary:(NSDictionary *)deviceDictionary;
+ (BOOL)canInitBitmap;

- initWithSize:(O2Size)size window:(CGWindow *)window;
- initWithSize:(O2Size)size context:(O2Context *)context;

- initWithGraphicsState:(O2GState *)state;
- init;

- (O2Surface *)surface;
- (O2Surface *)createSurfaceWithWidth:(size_t)width height:(size_t)height;

- (void)beginTransparencyLayerWithInfo:(NSDictionary *)unused;
- (void)endTransparencyLayer;

O2ColorRef O2ContextStrokeColor(O2ContextRef self);
O2ColorRef O2ContextFillColor(O2ContextRef self);

- (void)setStrokeAlpha:(O2Float)alpha;
- (void)setGrayStrokeColor:(O2Float)gray;
- (void)setStrokeColorRed:(O2Float)r green:(O2Float)g blue:(O2Float)b;
- (void)setStrokeColorC:(O2Float)c m:(O2Float)m y:(O2Float)y k:(O2Float)k;

- (void)setFillAlpha:(O2Float)alpha;
- (void)setGrayFillColor:(O2Float)gray;
- (void)setFillColorRed:(O2Float)r green:(O2Float)g blue:(O2Float)b;
- (void)setFillColorC:(O2Float)c m:(O2Float)m y:(O2Float)y k:(O2Float)k;

- (void)drawPath:(O2PathDrawingMode)pathMode;

- (void)showGlyphs:(const O2Glyph *)glyphs advances:(const O2Size *)advances count:(unsigned)count;

- (void)drawShading:(O2Shading *)shading;
- (void)drawImage:(O2Image *)image inRect:(O2Rect)rect;
- (void)drawLayer:(O2LayerRef)layer inRect:(O2Rect)rect;

- (void)flush;
- (void)synchronize;
- (BOOL)resizeWithNewSize:(O2Size)size;

- (void)beginPage:(const O2Rect *)mediaBox;
- (void)endPage;
- (void)close;

- (O2Size)size;
- (O2ContextRef)createCompatibleContextWithSize:(O2Size)size unused:(NSDictionary *)unused;

- (BOOL)getImageableRect:(O2Rect *)rect;

// temporary

- (void)setAntialiasingQuality:(int)value;

- (void)copyBitsInRect:(O2Rect)rect toPoint:(O2Point)point gState:(int)gState;

- (void)clipToState:(O2ClipState *)clipState;

- (BOOL)supportsGlobalAlpha;
- (NSData *)captureBitmapInRect:(NSRect)rect;

O2ContextRef O2ContextRetain(O2ContextRef self);
void O2ContextRelease(O2ContextRef self);

// context state
void O2ContextSetAllowsAntialiasing(O2ContextRef self, BOOL yesOrNo);

// layers
void O2ContextBeginTransparencyLayer(O2ContextRef self, NSDictionary *unused);
void O2ContextEndTransparencyLayer(O2ContextRef self);

// path
BOOL O2ContextIsPathEmpty(O2ContextRef self);
O2Point O2ContextGetPathCurrentPoint(O2ContextRef self);
O2Rect O2ContextGetPathBoundingBox(O2ContextRef self);
BOOL O2ContextPathContainsPoint(O2ContextRef self, O2Point point, O2PathDrawingMode pathMode);

void O2ContextBeginPath(O2ContextRef self);
void O2ContextClosePath(O2ContextRef self);
void O2ContextMoveToPoint(O2ContextRef self, O2Float x, O2Float y);
void O2ContextAddLineToPoint(O2ContextRef self, O2Float x, O2Float y);
void O2ContextAddCurveToPoint(O2ContextRef self, O2Float cx1, O2Float cy1, O2Float cx2, O2Float cy2, O2Float x, O2Float y);
void O2ContextAddQuadCurveToPoint(O2ContextRef self, O2Float cx1, O2Float cy1, O2Float x, O2Float y);

void O2ContextAddLines(O2ContextRef self, const O2Point *points, unsigned count);
void O2ContextAddRect(O2ContextRef self, O2Rect rect);
void O2ContextAddRects(O2ContextRef self, const O2Rect *rects, unsigned count);

void O2ContextAddArc(O2ContextRef self, O2Float x, O2Float y, O2Float radius, O2Float startRadian, O2Float endRadian, BOOL clockwise);
void O2ContextAddArcToPoint(O2ContextRef self, O2Float x1, O2Float y1, O2Float x2, O2Float y2, O2Float radius);
void O2ContextAddEllipseInRect(O2ContextRef self, O2Rect rect);

void O2ContextAddPath(O2ContextRef self, O2PathRef path);

O2Path *O2ContextCopyPath(O2ContextRef self);
void O2ContextReplacePathWithStrokedPath(O2ContextRef self);

// gstate

void O2ContextSaveGState(O2ContextRef self);
void O2ContextRestoreGState(O2ContextRef self);

O2AffineTransform O2ContextGetUserSpaceToDeviceSpaceTransform(O2ContextRef self);
O2AffineTransform O2ContextGetCTM(O2ContextRef self);
O2Rect O2ContextGetClipBoundingBox(O2ContextRef self);
O2AffineTransform O2ContextGetTextMatrix(O2ContextRef self);
O2InterpolationQuality O2ContextGetInterpolationQuality(O2ContextRef self);
O2Point O2ContextGetTextPosition(O2ContextRef self);

O2Point O2ContextConvertPointToDeviceSpace(O2ContextRef self, O2Point point);
O2Point O2ContextConvertPointToUserSpace(O2ContextRef self, O2Point point);
O2Size O2ContextConvertSizeToDeviceSpace(O2ContextRef self, O2Size size);
O2Size O2ContextConvertSizeToUserSpace(O2ContextRef self, O2Size size);
O2Rect O2ContextConvertRectToDeviceSpace(O2ContextRef self, O2Rect rect);
O2Rect O2ContextConvertRectToUserSpace(O2ContextRef self, O2Rect rect);

void O2ContextConcatCTM(O2ContextRef self, O2AffineTransform matrix);
void O2ContextTranslateCTM(O2ContextRef self, O2Float translatex, O2Float translatey);
void O2ContextScaleCTM(O2ContextRef self, O2Float scalex, O2Float scaley);
void O2ContextRotateCTM(O2ContextRef self, O2Float radians);

void O2ContextClip(O2ContextRef self);
void O2ContextEOClip(O2ContextRef self);
void O2ContextClipToMask(O2ContextRef self, O2Rect rect, O2ImageRef image);
void O2ContextClipToRect(O2ContextRef self, O2Rect rect);
void O2ContextClipToRects(O2ContextRef self, const O2Rect *rects, unsigned count);

void O2ContextSetStrokeColorSpace(O2ContextRef self, O2ColorSpaceRef colorSpace);
void O2ContextSetFillColorSpace(O2ContextRef self, O2ColorSpaceRef colorSpace);

void O2ContextSetStrokeColor(O2ContextRef self, const O2Float *components);
void O2ContextSetStrokeColorWithColor(O2ContextRef self, O2ColorRef color);
void O2ContextSetGrayStrokeColor(O2ContextRef self, O2Float gray, O2Float alpha);
void O2ContextSetRGBStrokeColor(O2ContextRef self, O2Float r, O2Float g, O2Float b, O2Float alpha);
void O2ContextSetCMYKStrokeColor(O2ContextRef self, O2Float c, O2Float m, O2Float y, O2Float k, O2Float alpha);

void O2ContextSetFillColor(O2ContextRef self, const O2Float *components);
void O2ContextSetFillColorWithColor(O2ContextRef self, O2ColorRef color);
void O2ContextSetGrayFillColor(O2ContextRef self, O2Float gray, O2Float alpha);
void O2ContextSetRGBFillColor(O2ContextRef self, O2Float r, O2Float g, O2Float b, O2Float alpha);
void O2ContextSetCMYKFillColor(O2ContextRef self, O2Float c, O2Float m, O2Float y, O2Float k, O2Float alpha);
void O2ContextSetCalibratedGrayFillColor(O2ContextRef self, O2Float gray, O2Float alpha);
void O2ContextSetCalibratedRGBFillColor(O2ContextRef self, O2Float red, O2Float green, O2Float blue, O2Float alpha);

void O2ContextSetAlpha(O2ContextRef self, O2Float alpha);

void O2ContextSetPatternPhase(O2ContextRef self, O2Size phase);
void O2ContextSetStrokePattern(O2ContextRef self, O2PatternRef pattern, const O2Float *components);
void O2ContextSetFillPattern(O2ContextRef self, O2PatternRef pattern, const O2Float *components);

void O2ContextSetTextMatrix(O2ContextRef self, O2AffineTransform matrix);

void O2ContextSetTextPosition(O2ContextRef self, O2Float x, O2Float y);
void O2ContextSetCharacterSpacing(O2ContextRef self, O2Float spacing);
void O2ContextSetTextDrawingMode(O2ContextRef self, O2TextDrawingMode textMode);

void O2ContextSetFont(O2ContextRef self, O2FontRef font);
void O2ContextSetFontSize(O2ContextRef self, O2Float size);
void O2ContextSelectFont(O2ContextRef self, const char *name, O2Float size, O2TextEncoding encoding);
void O2ContextSetShouldSmoothFonts(O2ContextRef self, BOOL yesOrNo);

void O2ContextSetLineWidth(O2ContextRef self, O2Float width);
void O2ContextSetLineCap(O2ContextRef self, O2LineCap lineCap);
void O2ContextSetLineJoin(O2ContextRef self, O2LineJoin lineJoin);
void O2ContextSetMiterLimit(O2ContextRef self, O2Float miterLimit);
void O2ContextSetLineDash(O2ContextRef self, O2Float phase, const O2Float *lengths, unsigned count);

void O2ContextSetRenderingIntent(O2ContextRef self, O2ColorRenderingIntent renderingIntent);
void O2ContextSetBlendMode(O2ContextRef self, O2BlendMode blendMode);

void O2ContextSetFlatness(O2ContextRef self, O2Float flatness);

void O2ContextSetInterpolationQuality(O2ContextRef self, O2InterpolationQuality quality);

void O2ContextSetShadowWithColor(O2ContextRef self, O2Size offset, O2Float blur, O2ColorRef color);
void O2ContextSetShadow(O2ContextRef self, O2Size offset, O2Float blur);

void O2ContextSetShouldAntialias(O2ContextRef self, BOOL yesOrNo);

// drawing
void O2ContextStrokeLineSegments(O2ContextRef self, const O2Point *points, unsigned count);

void O2ContextStrokeRect(O2ContextRef self, O2Rect rect);
void O2ContextStrokeRectWithWidth(O2ContextRef self, O2Rect rect, O2Float width);
void O2ContextStrokeEllipseInRect(O2ContextRef self, O2Rect rect);

void O2ContextFillRect(O2ContextRef self, O2Rect rect);
void O2ContextFillRects(O2ContextRef self, const O2Rect *rects, unsigned count);
void O2ContextFillEllipseInRect(O2ContextRef self, O2Rect rect);

void O2ContextDrawPath(O2ContextRef self, O2PathDrawingMode pathMode);
void O2ContextStrokePath(O2ContextRef self);
void O2ContextFillPath(O2ContextRef self);
void O2ContextEOFillPath(O2ContextRef self);

void O2ContextClearRect(O2ContextRef self, O2Rect rect);

void O2ContextShowGlyphs(O2ContextRef self, const O2Glyph *glyphs, unsigned count);
void O2ContextShowGlyphsAtPoint(O2ContextRef self, O2Float x, O2Float y, const O2Glyph *glyphs, unsigned count);
void O2ContextShowGlyphsWithAdvances(O2ContextRef self, const O2Glyph *glyphs, const O2Size *advances, unsigned count);

void O2ContextShowText(O2ContextRef self, const char *text, unsigned count);
void O2ContextShowTextAtPoint(O2ContextRef self, O2Float x, O2Float y, const char *text, unsigned count);

void O2ContextDrawShading(O2ContextRef self, O2ShadingRef shading);
void O2ContextDrawImage(O2ContextRef self, O2Rect rect, O2ImageRef image);
void O2ContextDrawLayerAtPoint(O2ContextRef self, O2Point point, O2LayerRef layer);
void O2ContextDrawLayerInRect(O2ContextRef self, O2Rect rect, O2LayerRef layer);
void O2ContextDrawPDFPage(O2ContextRef self, O2PDFPageRef page);

void O2ContextFlush(O2ContextRef self);
void O2ContextSynchronize(O2ContextRef self);

// pagination

void O2ContextBeginPage(O2ContextRef self, const O2Rect *mediaBox);
void O2ContextEndPage(O2ContextRef self);

void O2ContextSetTextLineMatrix(O2ContextRef self, O2AffineTransform matrix);
O2AffineTransform O2ContextGetTextLineMatrix(O2ContextRef self);
O2Float O2ContextGetTextLeading(O2ContextRef self);
void O2ContextSetTextLeading(O2ContextRef self, O2Float value);
void O2ContextSetWordSpacing(O2ContextRef self, O2Float value);
void O2ContextSetTextRise(O2ContextRef self, O2Float value);
void O2ContextSetTextHorizontalScaling(O2ContextRef self, O2Float value);
void O2ContextSetEncoding(O2ContextRef self, O2Encoding *value);
void O2ContextSetPDFCharWidths(O2ContextRef self, O2PDFCharWidths *value);

// **PRIVATE** These are private in Apple's implementation as well as ours.

void O2ContextSetCTM(O2ContextRef self, O2AffineTransform matrix);
void O2ContextResetClip(O2ContextRef self);

O2AffineTransform O2ContextGetTextRenderingMatrix(O2ContextRef self);

void O2ContextGetDefaultAdvances(O2ContextRef self, const O2Glyph *glyphs, O2Size *advances, size_t count);
void O2ContextConcatAdvancesToTextMatrix(O2ContextRef self, const O2Size *advances, size_t count);

O2GState *O2ContextCurrentGState(O2ContextRef self);

// Temporary hacks

void O2ContextCopyBits(O2ContextRef self, O2Rect rect, O2Point point, int gState);
bool O2ContextSupportsGlobalAlpha(O2ContextRef self);
bool O2ContextIsBitmapContext(O2ContextRef self);
NSData *O2ContextCaptureBitmap(O2ContextRef self, O2Rect rect);

@end

#ifdef __cplusplus
}
#endif
