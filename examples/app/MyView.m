#import "MyView.h"
#import <CoreGraphics/CGContext.h>

@implementation MyView

-initWithFrame:(NSRect)frame {
    [super initWithFrame:frame];
    NSColorList *web = [NSColorList colorListNamed:@"Web"];
    NSArray *keys = [web allKeys];
    _colors = [[NSMutableArray arrayWithCapacity:[keys count]] retain];
    for(int i=0; i<[keys count]; ++i) {
        NSColor *color = [[web colorWithKey:[keys objectAtIndex:i]] color];
        [_colors addObject:[color retain]];
    }
    return self;
}

-(void)setText:(NSString *)string {
    _text = string;
}

-(void)setFont:(NSFont *)font {
    _font = font;
}

-(void)drawRect:(NSRect)dirtyRect {
    NSSize size = [self frame].size;
    NSRect swatch = NSMakeRect(6,size.height - 56,48,48);
    for(int i=0; i<[_colors count]; ++i) {
        [[_colors objectAtIndex:i] drawSwatchInRect:swatch];
        swatch.origin.x += 56;
        if(swatch.origin.x > size.width) {
            swatch.origin.x = 6;
            swatch.origin.y -= 56;
            if(swatch.origin.y < 6) break;
        }
    }
}

@end


