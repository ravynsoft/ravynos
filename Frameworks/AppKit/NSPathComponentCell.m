#import <AppKit/NSPathComponentCell.h>
#import "AppKit/NSRaise.h"
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSAttributedString.h>

@implementation NSPathComponentCell

-(NSURL *)URL {
   return _URL;
}

-(void)setURL:(NSURL *)value {
   value=[value retain];
   [_URL release];
   _URL=value;
}

-(NSString *)description {
   return [NSString stringWithFormat: @"<%@: %p %@ URL: %@, Image: %@)>", [self class], self, [self title], _URL, [self image]];
}

-(NSDictionary *) textAttributes {
   NSFont *font = [self font];

   return [NSDictionary dictionaryWithObjectsAndKeys: font, NSFontAttributeName, nil];
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)view {
	NSDictionary *attributes = [self textAttributes];
	NSString *title = [self stringValue];
	NSSize textSize = [title sizeWithAttributes: attributes];
	
	frame.origin.y += (frame.size.height - textSize.height) / 2.0;
	frame.size.height = textSize.height;
	
	NSImage *icon = [self image];
	if (icon) {
		NSRect iconRect = frame;
		iconRect.size.width = frame.size.height;
		
		frame.size.width -= iconRect.size.width + 2;
		frame.origin.x += iconRect.size.width + 2;
		
		[icon drawInRect: iconRect fromRect: NSZeroRect operation: NSCompositeSourceOver fraction: 1.0];
	}
	
	[title drawAtPoint: frame.origin withAttributes: [self textAttributes]];
}

-(NSSize) cellSize {
	NSSize textSize = [[self stringValue] sizeWithAttributes: [self textAttributes]];
    
	if ([self image])
     textSize.width += textSize.height;
     
	textSize.width += 14;
	
	return textSize;
}

@end