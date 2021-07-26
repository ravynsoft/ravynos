/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2Font.h>

#ifdef __cplusplus
extern "C" {
#endif

@class O2Image, O2ColorSpace, O2Color, O2Pattern, O2MutablePath, O2Path, NSArray, NSMutableArray, O2Font, O2Encoding, O2PDFCharWidths, O2ClipState;

@interface O2GState : NSObject {
  @public
    O2AffineTransform _deviceSpaceTransform;
    O2AffineTransform _userSpaceTransform;

    O2ClipState *_clipState;
    O2ColorRef _strokeColor;
    O2ColorRef _fillColor;
    O2FontRef _font;
    O2Float _pointSize;
    BOOL _fontIsDirty;
    BOOL _fillColorIsDirty;

    O2Float _characterSpacing;
    O2Float _wordSpacing;
    O2Float _textLeading;
    O2Float _textRise;
    O2Float _textHorizontalScaling;

    O2Encoding *_encoding;
    O2PDFCharWidths *_pdfCharWidths;
    id _fontState;
    O2Size _patternPhase;
    int _textDrawingMode;
    BOOL _shouldSmoothFonts;
    float _lineWidth;
    O2LineCap _lineCap;
    O2LineJoin _lineJoin;
    float _miterLimit;
    float _dashPhase;
    int _dashLengthsCount;
    float *_dashLengths;
    O2ColorRenderingIntent _renderingIntent;
    O2BlendMode _blendMode;
    float _alpha;
    float _flatness;
    O2InterpolationQuality _interpolationQuality;
    O2Size _shadowOffset;
    float _shadowBlur;
    O2ColorRef _shadowColor;
    void *_shadowKernel;
    BOOL _shouldAntialias;

    int _antialiasingQuality;
}

- initWithDeviceTransform:(O2AffineTransform)deviceTransform;
- initFlippedWithDeviceHeight:(O2Float)height;
- initFlippedWithDeviceHeight:(O2Float)height concat:(O2AffineTransform)concat;
- init;

O2GState *O2GStateCopyWithZone(O2GState *self, NSZone *zone);

- (O2AffineTransform)userSpaceToDeviceSpaceTransform;
O2AffineTransform O2GStateUserSpaceTransform(O2GState *self);
- (O2Rect)clipBoundingBox;
- (O2InterpolationQuality)interpolationQuality;
- (O2Point)convertPointToDeviceSpace:(O2Point)point;
- (O2Point)convertPointToUserSpace:(O2Point)point;
- (O2Size)convertSizeToDeviceSpace:(O2Size)size;
- (O2Size)convertSizeToUserSpace:(O2Size)size;
- (O2Rect)convertRectToDeviceSpace:(O2Rect)rect;
- (O2Rect)convertRectToUserSpace:(O2Rect)rect;

void O2GStateSetDeviceSpaceCTM(O2GState *self, O2AffineTransform transform);
void O2GStateSetUserSpaceCTM(O2GState *self, O2AffineTransform transform);
void O2GStateConcatCTM(O2GState *self, O2AffineTransform transform);

O2ClipState *O2GStateClipState(O2GState *self);
void O2GStateResetClip(O2GState *self);
void O2GStateAddClipToPath(O2GState *self, O2Path *path);
void O2GStateAddEvenOddClipToPath(O2GState *self, O2Path *path);
void O2GStateAddClipToMask(O2GState *self, O2Image *image, O2Rect rect);

O2ColorRef O2GStateStrokeColor(O2GState *self);
O2ColorRef O2GStateFillColor(O2GState *self);

void O2GStateSetStrokeColor(O2GState *self, O2ColorRef color);
void O2GStateSetFillColor(O2GState *self, O2ColorRef color);

O2Size O2GStatePatternPhase(O2GState *self);
void O2GStateSetPatternPhase(O2GState *self, O2Size value);

- (void)setStrokePattern:(O2Pattern *)pattern components:(const float *)components;
- (void)setFillPattern:(O2Pattern *)pattern components:(const float *)components;

- (void)setTextDrawingMode:(int)textMode;
O2FontRef O2GStateFont(O2GState *self);
O2Float O2GStatePointSize(O2GState *self);

O2Encoding *O2GStateEncoding(O2GState *self);
O2PDFCharWidths *O2GStateCharWidths(O2GState *self);

- (O2Encoding *)encoding;
- (O2PDFCharWidths *)pdfCharWidths;
- (void)setPDFCharWidths:(O2PDFCharWidths *)value;

void O2GStateClearFontIsDirty(O2GState *self);
- (id)fontState;
- (void)setFontState:(id)fontState;
void O2GStateSetFont(O2GState *self, O2Font *font);
void O2GStateSetFontSize(O2GState *self, float size);
void O2GStateSetFontEncoding(O2GState *self, O2Encoding *encoding);
- (void)selectFontWithName:(const char *)name size:(float)size encoding:(O2TextEncoding)encoding;

CGFloat O2GStateCharacterSpacing(O2GState *self);
CGFloat O2GStateWordSpacing(O2GState *self);
CGFloat O2GStateTextLeading(O2GState *self);
CGFloat O2GStateTextRise(O2GState *self);
CGFloat O2GStateTextHorizontalScaling(O2GState *self);

void O2GStateSetCharacterSpacing(O2GState *self, CGFloat value);
void O2GStateSetWordSpacing(O2GState *self, CGFloat value);
void O2GStateSetTextLeading(O2GState *self, CGFloat value);
void O2GStateSetTextRise(O2GState *self, CGFloat value);
void O2GStateSetTextHorizontalScaling(O2GState *self, CGFloat value);

- (void)setShouldSmoothFonts:(BOOL)yesOrNo;

void O2GStateSetLineWidth(O2GState *self, float width);
void O2GStateSetLineCap(O2GState *self, int lineCap);
void O2GStateSetLineJoin(O2GState *self, int lineJoin);
void O2GStateSetMiterLimit(O2GState *self, float limit);
void O2GStateSetLineDash(O2GState *self, float phase, const float *lengths, unsigned count);

- (void)setRenderingIntent:(O2ColorRenderingIntent)intent;
O2BlendMode O2GStateBlendMode(O2GState *self);
void O2GStateSetBlendMode(O2GState *self, O2BlendMode mode);
float O2GStateAlpha(O2GState *self);
void O2GStateSetAlpha(O2GState *self, float alpha);

- (void)setFlatness:(float)flatness;
- (void)setInterpolationQuality:(O2InterpolationQuality)quality;

- (void)setShadowOffset:(O2Size)offset blur:(float)blur color:(O2ColorRef)color;
- (void)setShadowOffset:(O2Size)offset blur:(float)blur;

- (void)setShouldAntialias:(BOOL)flag;

// temporary?
- (void)setAntialiasingQuality:(int)value;

@end

#ifdef __cplusplus
}
#endif
