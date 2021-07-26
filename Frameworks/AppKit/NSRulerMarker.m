/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/NSRulerMarker.h>
#import <AppKit/NSRulerView.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScrollView.h>

@implementation NSRulerMarker

+ (NSImage *)defaultMarkerImage
{
    return [NSImage imageNamed:@"NSRulerMarkerTab"];
}

- (id)initWithRulerView:(NSRulerView *)ruler markerLocation:(float)location image:(NSImage *)image imageOrigin:(NSPoint)point
{
    _ruler = ruler;
    _markerLocation = location;
    _image = [image retain];
    _imageOrigin = point;
    _isMovable = YES;
    _isRemovable = YES;
    _isPinned = YES;
    
    return self;
}

- (void)dealloc
{
    [_image release];
    [_representedObject release];
    
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    NSRulerMarker *copy = NSCopyObject(self, 0, zone);
    
    copy->_ruler = [_ruler retain];
    copy->_image = [_image retain];
    copy->_representedObject = [_representedObject copy];
    
    return copy;
}

- (NSRulerView *)ruler
{
    return _ruler;
}

- (float)markerLocation
{
    return _markerLocation;
}

- (NSImage *)image
{
    return _image;
}

- (NSPoint)imageOrigin
{
    return _imageOrigin;
}

- (id <NSCopying>)representedObject
{
    return _representedObject;
}

- (BOOL)isRemovable
{
    return _isRemovable;
}

- (BOOL)isMovable
{
    return _isMovable;
}

- (void)setMarkerLocation:(float)location
{
    _markerLocation = location;
}

- (void)setImage:(NSImage *)image
{
    [_image release];
    _image = [image retain];
}

- (void)setImageOrigin:(NSPoint)point
{
    _imageOrigin = point;
}

- (void)setRepresentedObject:(id <NSCopying>)object
{
    object = [object copyWithZone:nil];
    [_representedObject release];
    _representedObject = object;
}

- (void)setRemovable:(BOOL)flag
{
    _isRemovable = flag;
}

- (void)setMovable:(BOOL)flag
{
    _isMovable = flag;
}

- (float)thicknessRequiredInRuler
{
    float thickness = 0;
    
    if ([_ruler orientation] == NSVerticalRuler) {
        thickness += [[self image] size].width;
        thickness += _imageOrigin.x;
    }
    else if ([_ruler orientation] == NSHorizontalRuler) {
        thickness += [[self image] size].height;
        thickness += _imageOrigin.y;
    }
    
    return thickness;
}

- (NSRect)imageRectInRuler
{
    NSRect rect = [_ruler bounds];
    
    NSPoint location = NSMakePoint(_markerLocation, _markerLocation);
    if ([_ruler clientView]) {
        location = [_ruler convertPoint:location fromView:[_ruler clientView]];        
    } else {
        location = [_ruler convertPoint:location fromView:[[_ruler enclosingScrollView] documentView]];
    }

    if ([_ruler orientation] == NSHorizontalRuler) {
        rect.origin.x += location.x;
    } else {
        rect.origin.y += location.y;       // how does a flipped system affect this? hm
    }
    rect.origin.x -= _imageOrigin.x;
    rect.origin.y -= _imageOrigin.y;

    rect.size = [_image size];

    return rect;
}

- (void)drawRect:(NSRect)rect
{
    if (_isDragging && _isPinned == NO) {
        // The marker is outside of the ruler - we might actually want to draw it
        // at the dragged mouse location
        return;
    }
    NSPoint location = [self imageRectInRuler].origin;
    if ([_ruler isFlipped]) {
        location.y += [_image size].height;
    }
    [[self image] compositeToPoint:location operation:NSCompositeSourceOver];
}

- (BOOL)isDragging
{
    return _isDragging;
}

- (float)_markerLocationFromLocation:(NSPoint)point
{
    NSView *trackedView = _ruler.clientView;
    if (trackedView == nil) {
        trackedView = _ruler.enclosingScrollView.documentView;
    }
    point = [_ruler convertPoint:point toView:trackedView];
    float newLocation = [_ruler orientation] == NSHorizontalRuler ? point.x : point.y;

    return newLocation;
}

- (BOOL)_trackAddMarker:(NSEvent *)event
{
    NSPoint point = [_ruler convertPoint:[event locationInWindow] fromView:nil];
    _isPinned = YES;
    
    if ([[_ruler clientView] respondsToSelector:@selector(rulerView:shouldAddMarker:)])
        if ([[_ruler clientView] rulerView:_ruler shouldAddMarker:self] == NO)
            return NO;
    
    BOOL respondToShouldAddMarker = [[_ruler clientView] respondsToSelector:@selector(rulerView:shouldAddMarker:)];
    _isDragging = YES;
    do {
        point = [_ruler convertPoint:[event locationInWindow] fromView:nil];
        _isPinned = NO;
        if (NSMouseInRect(point, [_ruler bounds], [_ruler isFlipped]) == YES) {
            if (respondToShouldAddMarker) {
                _isPinned = [[_ruler clientView] rulerView:_ruler shouldAddMarker:self];
            } else {
                _isPinned = YES;
            }
        }
        
        _markerLocation = [self _markerLocationFromLocation:point];
        if ([[_ruler clientView] respondsToSelector:@selector(rulerView:willAddMarker:atLocation:)]) {
            _markerLocation = [[_ruler clientView] rulerView:_ruler willAddMarker:self atLocation:_markerLocation];
        }

        // Draw the ruler + the new marker
        [_ruler lockFocus];
        [_ruler drawRect:[_ruler bounds]];
        [self drawRect:[_ruler bounds]];
        [_ruler unlockFocus];
        [[_ruler window] flushWindow];
        
        event = [[_ruler window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];
    } while ([event type] != NSLeftMouseUp);
    _isDragging = NO;
    
    // check for adding...
    if (_isPinned) {
        [_ruler addMarker:self];
        
        if ([[_ruler clientView] respondsToSelector:@selector(rulerView:didAddMarker:)])
            [[_ruler clientView] rulerView:_ruler didAddMarker:self];
        
        return YES;
    }
    _isPinned = YES;
    
    return NO;
}

- (BOOL)_trackMoveMarker:(NSEvent *)event
{
    NSPoint point = [_ruler convertPoint:[event locationInWindow] fromView:nil];
    _isPinned = YES;
    if ([[_ruler clientView] respondsToSelector:@selector(rulerView:shouldMoveMarker:)])
        if ([[_ruler clientView] rulerView:_ruler shouldMoveMarker:self] == NO)
            return NO;
    
    BOOL respondToShouldRemoveMarker = [[_ruler clientView] respondsToSelector:@selector(rulerView:shouldRemoveMarker:)];
    _isDragging = YES;
    do {
        if ([self isMovable]) {
            point = [_ruler convertPoint:[event locationInWindow] fromView:nil];
            _isPinned = YES;
            if (NSMouseInRect(point, [_ruler bounds], [_ruler isFlipped]) == NO) {
                if (respondToShouldRemoveMarker) {
                    _isPinned =  [[_ruler clientView] rulerView:_ruler shouldRemoveMarker:self] == NO;
                } else {
                    _isPinned = NO;
                }
            }
            _markerLocation = [self _markerLocationFromLocation:point];
            if ([[_ruler clientView] respondsToSelector:@selector(rulerView:willMoveMarker:toLocation:)]) {
                _markerLocation = [[_ruler clientView] rulerView:_ruler willMoveMarker:self toLocation:_markerLocation];
            }
            [_ruler setNeedsDisplay:YES];
        }
        event = [[_ruler window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];
    } while ([event type] != NSLeftMouseUp);
    _isDragging = NO;
    
    // check for removing...
    BOOL removed = NO;
    if (_isPinned == NO) {
        [[self retain] autorelease]; // Be sure we survive the removal until we're done
        [_ruler removeMarker:self];
        removed = YES;
        
        if ([[_ruler clientView] respondsToSelector:@selector(rulerView:didRemoveMarker:)])
            [[_ruler clientView] rulerView:_ruler didRemoveMarker:self];
    }
    _isPinned = YES;
    if (!removed) {
        // Call didMoveMarker only after the dragging is done - that's what Cocoa is doing
        if ([[_ruler clientView] respondsToSelector:@selector(rulerView:didMoveMarker:)]) {
            [[_ruler clientView] rulerView:_ruler didMoveMarker:self];
        }
    }
    
    return YES;
}

- (BOOL)trackMouse:(NSEvent *)event adding:(BOOL)adding
{
    if (adding == YES) {
        return [self _trackAddMarker:event];
    } else {
        return [self _trackMoveMarker:event];
    }
}

@end
