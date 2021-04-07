#import <AppKit/AppKit.h>

@interface MyView: NSView {
	NSString * _text;
	NSFont * _font;
}

-(void)setText:(NSString *)string;
-(void)setFont:(NSFont *)font;
@end


