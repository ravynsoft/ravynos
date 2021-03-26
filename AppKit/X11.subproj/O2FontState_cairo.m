#import "O2FontState_cairo.h"
#import "O2Font_FT.h"

@implementation O2FontState_cairo

-initWithFreeTypeFont:(O2Font_FT *)font size:(O2Float)size {
   _font=[font retain];
   _size=size;
   _cairo_font_face=cairo_ft_font_face_create_for_ft_face([font face],0);
   return self;
}

-(void)dealloc {
   [_font release];
   cairo_font_face_destroy(_cairo_font_face);
   [super dealloc];
}

-(cairo_font_face_t  *)cairo_font_face {
   return _cairo_font_face;
}

@end
