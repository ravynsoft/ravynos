#import <Foundation/NSObject.h>
#import <Onyx2D/O2Font.h>

@interface O2Encoding : NSObject <NSCopying, NSMutableCopying> {
    O2Glyph _glyphs[256];
    unichar _unicode[256];
}

- initWithGlyphs:(const O2Glyph *)glyphs unicode:(uint16_t *)unicode;

- (void)getGlyphs:(O2Glyph *)glyphs;
- (void)getUnicode:(uint16_t *)unicode;

void O2EncodingGetGlyphsForBytes(O2Encoding *self, O2Glyph *glyphs, const uint8_t *bytes, unsigned length);
void O2EncodingGetUnicodeForBytes(O2Encoding *self, uint16_t *unicode, const uint8_t *bytes, unsigned length);

void O2EncodingGetMacRomanUnicode(unichar *codes);
void O2EncodingGetMacExpertUnicode(unichar *codes);
void O2EncodingGetWinAnsiUnicode(unichar *codes);

@end

@interface O2MutableEncoding : O2Encoding

- (void)setGlyph:(O2Glyph)glyph unicode:(uint16_t)code atIndex:(int)index;

@end
