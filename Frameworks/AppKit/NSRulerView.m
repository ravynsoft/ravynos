/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/NSRulerView.h>
#import <AppKit/NSRulerMarker.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSMeasurementUnit.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSText.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSImage.h>

#define DEFAULT_RULE_THICKNESS      16.0
#define DEFAULT_MARKER_THICKNESS    15.0

#define HASH_MARK_THICKNESS_FACTOR  0.5
#define HASH_MARK_WIDTH             1.0
#define HASH_MARK_REQUIRED_WIDTH    2.0

#define LABEL_TEXT_CORRECTION       2.0
#define LABEL_TEXT_PRIMARY_OFFSET   3.0
#define LABEL_TEXT_SECONDARY_OFFSET 3.0

@interface NSRulerView(PrivateMethods)
- (float)_drawingOrigin;
- (float)_drawingScale;
@end

@implementation NSRulerView

+ (void)registerUnitWithName:(NSString *)name abbreviation:(NSString *)abbreviation unitToPointsConversionFactor:(float)conversionFactor stepUpCycle:(NSArray *)stepUpCycle stepDownCycle:(NSArray *)stepDownCycle
{
    [NSMeasurementUnit registerUnit:[NSMeasurementUnit measurementUnitWithName:name abbreviation:abbreviation pointsPerUnit:conversionFactor stepUpCycle:stepUpCycle stepDownCycle:stepDownCycle]];
}

- (id)initWithFrame:(NSRect)frame
{
    // Stack trace inspection on the Mac side indicates this is how a
    // ruler is created before being added to a scrollview
    return [self initWithScrollView: nil orientation: NSHorizontalRuler];
}

- initWithScrollView:(NSScrollView *)scrollView orientation:(NSRulerOrientation)orientation
{
    NSRect frame = NSMakeRect(0, 0, 1, 1);
    if (scrollView) {
        frame = [scrollView frame];
    }
    
    if (orientation == NSHorizontalRuler)
        frame.size.height = DEFAULT_RULE_THICKNESS;
    else
        frame.size.width = DEFAULT_RULE_THICKNESS;
    
    [super initWithFrame:frame];
    
    _scrollView = [scrollView retain];
    _orientation = orientation;
        
    _measurementUnit = [[NSMeasurementUnit measurementUnitNamed:@"Inches"] retain];
    
    // Don't invoke the setters - they trigger tiling which can cause recursion if
    // the scrollview is creating the ruler
    _ruleThickness = DEFAULT_RULE_THICKNESS;
    _thicknessForMarkers = DEFAULT_MARKER_THICKNESS;
    _thicknessForAccessoryView = 0.f;
    
    _markers = [[NSMutableArray alloc] init];
    
    _rulerlineLocations = [[NSMutableArray alloc] init];
    
    [self invalidateHashMarks];

    return self;
}

- (void)dealloc
{
    [_scrollView release];
    [_accessoryView release];
    [_measurementUnit release];
    
    [_markers release];
    [_rulerlineLocations release];
    
    [super dealloc];
}

- (NSMeasurementUnit *)measurementUnit
{
    return _measurementUnit;
}

- (NSScrollView *)scrollView
{
    return _scrollView;
}

- (NSView *)clientView
{
    return _clientView;
}

- (NSView *)accessoryView
{
    return _accessoryView;
}

- (NSArray *)markers
{
    return _markers;
}

- (NSString *)measurementUnits
{
    return [_measurementUnit name];
}

- (NSRulerOrientation)orientation
{
    return _orientation;
}

- (float)ruleThickness
{
    return _ruleThickness;
}

- (float)reservedThicknessForMarkers
{
    if (_thicknessForMarkers == 0.0) {
        int i, count = [_markers count];
        
        for (i = 0; i < count; ++i)
            if ([[_markers objectAtIndex:i] thicknessRequiredInRuler] > _thicknessForMarkers)
                _thicknessForMarkers = [[_markers objectAtIndex:i] thicknessRequiredInRuler];
    }
    
    return _thicknessForMarkers;
}

- (float)reservedThicknessForAccessoryView
{
    return _thicknessForAccessoryView;
}

- (float)originOffset
{
    return _originOffset;
}

- (float)baselineLocation
{
    // That should be something depending of the markers thickness, etc.
    return _ruleThickness;
}

- (float)requiredThickness
{
    float result = [self ruleThickness];
    
    if ([_markers count] > 0)
        result += [self reservedThicknessForMarkers];
    
    if (_accessoryView != nil)
        result += [self reservedThicknessForAccessoryView];
    
    return result;
}

- (void)setScrollView:(NSScrollView *)scrollView
{
    [_scrollView release];
    _scrollView = [scrollView retain];

    [self invalidateHashMarks];
}

- (void)setClientView:(NSView *)view
{
    [_clientView rulerView:self willSetClientView:view];
    [_markers removeAllObjects];
    _clientView = view;
    
    [self invalidateHashMarks];
    [[self enclosingScrollView] tile];
}

- (void)setAccessoryView:(NSView *)view
{
    [_accessoryView release];
    _accessoryView = [view retain];

    [[self enclosingScrollView] tile];
}

- (void)setMarkers:(NSArray *)markers
{
    [_markers release];
    _markers = [markers mutableCopy];

    [[self enclosingScrollView] tile];
}

- (void)addMarker:(NSRulerMarker *)marker
{
    [_markers addObject:marker];
    
    [[self enclosingScrollView] tile];
}

- (void)removeMarker:(NSRulerMarker *)marker
{
    [_markers removeObject:marker];
    
    [[self enclosingScrollView] tile];
}

- (void)setMeasurementUnits:(NSString *)unitName
{
    [_measurementUnit release];
    _measurementUnit = [[NSMeasurementUnit measurementUnitNamed:unitName] retain];

    [self invalidateHashMarks];
    [[self enclosingScrollView] tile];
}

- (void)setOrientation:(NSRulerOrientation)orientation
{
    _orientation = orientation;
}

- (void)setRuleThickness:(float)value
{
    _ruleThickness = value;
    
    [[self enclosingScrollView] tile];
}

- (void)setReservedThicknessForMarkers:(float)value
{
    _thicknessForMarkers = value;

    [[self enclosingScrollView] tile];
}

- (void)setReservedThicknessForAccessoryView:(float)value
{
    _thicknessForAccessoryView = value;

    [[self enclosingScrollView] tile];
}

- (void)setOriginOffset:(float)value
{
    _originOffset = value;
    
    [self invalidateHashMarks];
}

- (BOOL)trackMarker:(NSRulerMarker *)marker withMouseEvent:(NSEvent *)event
{
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    
    if(NSMouseInRect(point, [self bounds], [self isFlipped])){
        [marker trackMouse:event adding:YES];
        [self setNeedsDisplay:YES];
    }        
    
    return NO;
}

- (void)mouseDown:(NSEvent *)event
{
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    int i, count = [_markers count];
    float location;
    
    for (i = 0; i < count; ++i) {
        NSRulerMarker *marker = [_markers objectAtIndex:i];
        
        if (NSMouseInRect(point, [marker imageRectInRuler], [self isFlipped])) {
            [marker trackMouse:event adding:NO];
            [self setNeedsDisplay:YES];
            return;
        }
    }
    if ([_clientView respondsToSelector:@selector(rulerView:handleMouseDown:)]) {
        [_clientView rulerView:self handleMouseDown:event];
    }
}

- (void)moveRulerlineFromLocation:(float)fromLocation toLocation:(float)toLocation
{    
    NSNumber *old = [NSNumber numberWithFloat:fromLocation];
    NSNumber *new = [NSNumber numberWithFloat:toLocation];
    
    if ([_rulerlineLocations containsObject:old])
        [_rulerlineLocations removeObject:old];
    if ([_rulerlineLocations containsObject:new] == NO)
        [_rulerlineLocations addObject:new];
    
    [self setNeedsDisplay:YES];
}

- (void)invalidateHashMarks
{
    [self setNeedsDisplay:YES];
}

- (NSDictionary *)attributesForLabel
{
    NSMutableParagraphStyle *style = [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
    
    [style setLineBreakMode:NSLineBreakByClipping];
    [style setAlignment:NSLeftTextAlignment];
    
    return [NSDictionary dictionaryWithObjectsAndKeys:
        [NSFont systemFontOfSize:10.0], NSFontAttributeName,
        style, NSParagraphStyleAttributeName,
        nil];
}

- (void)drawHashMarksAndLabelsInRect:(NSRect)dirtyRect
{
    NSRect originalFrame = self.bounds; // The ruler area

    float scale = [self _drawingScale];
    float offset = [self _drawingOrigin];

    // Adjust originalFrame so it matches the ruler origin 
    if (_orientation == NSHorizontalRuler) {
        originalFrame.origin.x += offset;
        originalFrame.size.width -= offset;
    } else {
        originalFrame.origin.y += offset;
        originalFrame.size.height -= offset;
    }

    if (self.clientView) {
        // Adjust the frame size to the clientView
        NSRect clientFrame = self.clientView.frame;
        if (_orientation == NSHorizontalRuler) {
            originalFrame.size.width = clientFrame.size.width * scale;
        } else {
            originalFrame.size.height = clientFrame.size.height * scale;
        }
        
    }
    // No need to draw outside of this area
    dirtyRect = NSIntersectionRect(dirtyRect, originalFrame);

    NSRect frame = originalFrame;
    float pointsPerUnit = [_measurementUnit pointsPerUnit];
    float length = (_orientation == NSHorizontalRuler ? frame.size.width : frame.size.height);
    int i, count = ceil(length / (pointsPerUnit * scale));
    NSMutableArray *cycles = [[[_measurementUnit stepDownCycle] mutableCopy] autorelease];
    float extraThickness = 0;
    BOOL scrollViewHasOtherRuler = (_orientation == NSHorizontalRuler ? [[self enclosingScrollView] hasVerticalRuler] : [[self enclosingScrollView] hasHorizontalRuler]);

    if ([_markers count] > 0)
        extraThickness += [self reservedThicknessForMarkers];
    if (_accessoryView != nil)
        extraThickness += [self reservedThicknessForAccessoryView];
    
    // Some basic calculations.
    if (_orientation == NSHorizontalRuler) {
        originalFrame.size.width = HASH_MARK_WIDTH;
        originalFrame.size.height *= HASH_MARK_THICKNESS_FACTOR;
        originalFrame.origin.y += extraThickness;
    }
    else {
        originalFrame.size.width *= HASH_MARK_THICKNESS_FACTOR;
        originalFrame.size.height = HASH_MARK_WIDTH;
        originalFrame.origin.x += extraThickness;
    }
    
    // Draw major hash marks with labels.
    frame = originalFrame;
    [[NSColor controlShadowColor] setStroke];
    for (i = 0; i < count; ++i) {
        if (_orientation == NSHorizontalRuler) {
            if (frame.origin.x > NSMaxX(dirtyRect)) {
                break;
            }
        } else {
            if (frame.origin.y > NSMaxY(dirtyRect)) {
                break;
            }
        }

        NSString *label = [NSString stringWithFormat:@"%d", i];
        NSPoint textOrigin = frame.origin;

        // A little visual nudge.. I think it looks better.
        if (i == 0 && scrollViewHasOtherRuler == NO) {
            // Nothing to do
        } else {
            NSFrameRect(frame);
        }
        
        textOrigin.x += LABEL_TEXT_CORRECTION; // minor correction
        if (_orientation == NSHorizontalRuler) {
            textOrigin.x += LABEL_TEXT_PRIMARY_OFFSET;
            textOrigin.y += LABEL_TEXT_SECONDARY_OFFSET;
        }
        else {
            textOrigin.y += LABEL_TEXT_PRIMARY_OFFSET;
            textOrigin.x += LABEL_TEXT_SECONDARY_OFFSET;
        }
        [label drawAtPoint:textOrigin withAttributes:[self attributesForLabel]];

        if (_orientation == NSHorizontalRuler) {
            frame.origin.x += pointsPerUnit*scale;
        } else {
            frame.origin.y += pointsPerUnit*scale;
        }
    }
    
    // Start minor hash mark processing. size.width still contains the width of major marks.
    do {
        float thisCycle = [[cycles objectAtIndex:0] floatValue];
        float pointsPerMark = pointsPerUnit * thisCycle;
        
        frame.origin = originalFrame.origin;

#if 0
        if (_orientation == NSHorizontalRuler)
            frame.size.height *= HASH_MARK_THICKNESS_FACTOR;
        else
            frame.size.width *= HASH_MARK_THICKNESS_FACTOR;
#endif
        if (_orientation == NSHorizontalRuler)
            frame.size.height *= thisCycle;
        else
            frame.size.width *= thisCycle;        
                
        frame.size.height = floor(frame.size.height);
        
        if (HASH_MARK_REQUIRED_WIDTH < pointsPerMark*scale) {
            count = length / (pointsPerMark * scale);
            
            for (i = 0; i < count; ++i) {
                if (_orientation == NSHorizontalRuler) {
                    if (frame.origin.x > NSMaxX(dirtyRect)) {
                        break;
                    }
                } else {
                    if (frame.origin.y > NSMaxY(dirtyRect)) {
                        break;
                    }
                }

                // A little visual nudge.. I think it looks better.
                if (i == 0 && scrollViewHasOtherRuler == NO) {
                    // Nothing to do
                } else {
                    NSFrameRect(frame);
                }

                if (_orientation == NSHorizontalRuler) {
                    frame.origin.x += pointsPerMark*scale;
                } else {
                    frame.origin.y += pointsPerMark*scale;
                }
            }
        }
        
        [cycles removeObjectAtIndex:0];
    } while ([cycles count] > 0);
}

- (void)drawMarkersInRect:(NSRect)dirtyRect
{
    for (NSRulerMarker *marker in _markers) {
        [marker drawRect:dirtyRect];
    }
}

- (void)drawRulerlineLocationsInRect:(NSRect)rect
{
    int i, count = [_rulerlineLocations count];
    
    rect = [self bounds];
    
    if (_orientation == NSHorizontalRuler)
        rect.size.width = 1;
    else
        rect.size.height = 1;
    
    [[NSColor controlShadowColor] setStroke];
    for (i = 0; i < count; ++i) {
        if (_orientation == NSHorizontalRuler) {
            rect.origin.x = [[_rulerlineLocations objectAtIndex:i] floatValue] + 0.5;
            [NSBezierPath strokeLineFromPoint: NSMakePoint(NSMinX(rect), NSMinY(rect))
                                      toPoint: NSMakePoint(NSMinX(rect), NSMaxY(rect))];
        }
        else {
            rect.origin.y = [[_rulerlineLocations objectAtIndex:i] floatValue] + 0.5;
            [NSBezierPath strokeLineFromPoint: NSMakePoint(NSMinX(rect), NSMinY(rect))
                                      toPoint: NSMakePoint(NSMaxX(rect), NSMinY(rect))];
        }
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect rect = self.bounds;
    
    // Clear whole area.
    [[NSColor windowBackgroundColor] setFill];
    NSRectFill(dirtyRect);

    [[NSColor controlShadowColor] setStroke];
    if (_orientation == NSHorizontalRuler) {
        rect.origin.y += rect.size.height - 1;
        rect.size.height = 1;
    }
    else {
        rect.origin.x += rect.size.width - 1;
        rect.size.width = 1;
    }    
    NSFrameRect(rect);
    
    [self drawHashMarksAndLabelsInRect:dirtyRect];
    
    if ([_markers count] > 0)
        [self drawMarkersInRect:dirtyRect];
    
    if ([_rulerlineLocations count] > 0)
        [self drawRulerlineLocationsInRect:dirtyRect];
}

- (BOOL)isFlipped
{
    if (_orientation == NSHorizontalRuler)
        return YES;
    
    return [[_scrollView documentView] isFlipped];
}

@end

@implementation NSRulerView(PrivateMethods)
// The offset to use in the ruler view for the 0 location
- (float)_drawingOrigin
{
    float origin = 0;
    NSView *trackedView = self.clientView;
    if (trackedView == nil) {
        trackedView = _scrollView.documentView;
    }
    NSPoint viewOrigin = [self convertPoint:NSZeroPoint fromView:trackedView];
    if (self.orientation == NSHorizontalRuler) {
        origin += viewOrigin.x;
    } else {
        origin += viewOrigin.y;
    }
    origin += self.originOffset;
    return origin;
}

// The scale to use for drawing
- (float)_drawingScale
{
    float scale = 1.;
    NSView *documentView = [_scrollView documentView];
    if (documentView) {
        NSSize curDocFrameSize = documentView.frame.size;
        NSSize curDocBoundsSize = documentView.bounds.size;
        
        if ([self orientation] == NSHorizontalRuler) {
            scale = curDocFrameSize.width / curDocBoundsSize.width;
        } else {
            scale = curDocFrameSize.height / curDocBoundsSize.height;
        }
    }
    return scale;
}
@end

