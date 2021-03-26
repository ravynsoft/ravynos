#import <Foundation/NSAffineTransform.h>

@class NSBezierPath;

@interface NSAffineTransform (AppKit)

- (void)concat;
- (void)set;

- (NSBezierPath *)transformBezierPath:(NSBezierPath *)bezierPath;

@end
