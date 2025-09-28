//
//  NSRulerMarker+NSTextExtensions.h
//  AppKit
//
//  Created by Airy ANDRE on 16/11/12.
//
//

#import <AppKit/AppKit.h>

// Private methods to help text markers creation
@interface NSRulerMarker (NSTextExtensions)
+ (NSRulerMarker *)leftMarginMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)rightMarginMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)firstIndentMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)leftIndentMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)rightIndentMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)leftTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)rightTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)centerTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
+ (NSRulerMarker *)decimalTabMarkerWithRulerView:(NSRulerView *)ruler location:(float)location;
@end
