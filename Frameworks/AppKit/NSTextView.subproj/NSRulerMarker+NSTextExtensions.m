//
//  NSRulerMarker+NSTextExtensions.m
//  AppKit
//
//  Created by Airy ANDRE on 16/11/12.
//
//

#import "NSRulerMarker+NSTextExtensions.h"

// TODO: provide different images for different kinds of markers
@implementation NSRulerMarker (NSTextExtensions)
+ (NSRulerMarker *)leftTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}

+ (NSRulerMarker *)rightTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}


+ (NSRulerMarker *)centerTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}

+ (NSRulerMarker *)decimalTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}

+ (NSRulerMarker *)leftMarginMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}

+ (NSRulerMarker *)rightMarginMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}

+ (NSRulerMarker *)firstIndentMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}

+ (NSRulerMarker *)leftIndentMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}

+ (NSRulerMarker *)rightIndentMarkerWithRulerView:(NSRulerView *)ruler location:(float)location
{
    NSImage *image = [NSImage imageNamed:@"NSRulerMarkerTab"];
    NSPoint imageOrigin = NSMakePoint(0, 0);
    NSRulerMarker *marker = [[[NSRulerMarker alloc] initWithRulerView:ruler markerLocation:location image:image imageOrigin:imageOrigin] autorelease];
    return marker;
}
@end
