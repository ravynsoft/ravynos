/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSView.h>

@class NSString, NSMutableArray, NSScrollView, NSRulerMarker, NSMeasurementUnit;

typedef enum {
    NSHorizontalRuler,
    NSVerticalRuler
} NSRulerOrientation;

@interface NSRulerView : NSView {
    NSScrollView *_scrollView;
    NSView *_clientView;
    NSView *_accessoryView;
    NSMutableArray *_markers;
    NSMeasurementUnit *_measurementUnit;
    NSMutableArray *_rulerlineLocations;

    float _originOffset;
    float _ruleThickness;
    float _thicknessForMarkers;
    float _thicknessForAccessoryView;

    NSRulerOrientation _orientation;
}

+ (void)registerUnitWithName:(NSString *)name abbreviation:(NSString *)abbreviation unitToPointsConversionFactor:(float)conversionFactor stepUpCycle:(NSArray *)stepUpCycle stepDownCycle:(NSArray *)stepDownCycle;

- initWithScrollView:(NSScrollView *)scrollView orientation:(NSRulerOrientation)orientation;

- (NSScrollView *)scrollView;
- (NSView *)clientView;
- (NSView *)accessoryView;
- (NSArray *)markers;
- (NSString *)measurementUnits;
- (NSRulerOrientation)orientation;
- (float)ruleThickness;
- (float)reservedThicknessForMarkers;
- (float)reservedThicknessForAccessoryView;
- (float)originOffset;
- (float)baselineLocation;
- (float)requiredThickness;

- (void)setScrollView:(NSScrollView *)scrollView;
- (void)setClientView:(NSView *)view;
- (void)setAccessoryView:(NSView *)view;
- (void)setMarkers:(NSArray *)markers;
- (void)addMarker:(NSRulerMarker *)marker;
- (void)removeMarker:(NSRulerMarker *)marker;
- (void)setMeasurementUnits:(NSString *)unitName;
- (void)setOrientation:(NSRulerOrientation)orientation;
- (void)setRuleThickness:(float)value;
- (void)setReservedThicknessForMarkers:(float)value;
- (void)setReservedThicknessForAccessoryView:(float)value;
- (void)setOriginOffset:(float)value;

- (BOOL)trackMarker:(NSRulerMarker *)marker withMouseEvent:(NSEvent *)event;

- (void)moveRulerlineFromLocation:(float)fromLocation toLocation:(float)toLocation;

- (void)invalidateHashMarks;

- (void)drawHashMarksAndLabelsInRect:(NSRect)rect;
- (void)drawMarkersInRect:(NSRect)rect;

@end

@interface NSView (NSRulerMarker_clientView)

- (void)rulerView:(NSRulerView *)rulerView willSetClientView:(NSView *)clientView;

- (void)rulerView:(NSRulerView *)rulerView handleMouseDown:(NSEvent *)event;

- (BOOL)rulerView:(NSRulerView *)rulerView shouldAddMarker:(NSRulerMarker *)marker;
- (float)rulerView:(NSRulerView *)rulerView willAddMarker:(NSRulerMarker *)marker atLocation:(float)location;
- (void)rulerView:(NSRulerView *)rulerView didAddMarker:(NSRulerMarker *)marker;

- (BOOL)rulerView:(NSRulerView *)rulerView shouldRemoveMarker:(NSRulerMarker *)marker;
- (void)rulerView:(NSRulerView *)rulerView didRemoveMarker:(NSRulerMarker *)marker;

- (BOOL)rulerView:(NSRulerView *)rulerView shouldMoveMarker:(NSRulerMarker *)marker;
- (float)rulerView:(NSRulerView *)rulerView willMoveMarker:(NSRulerMarker *)marker toLocation:(float)location;
- (void)rulerView:(NSRulerView *)rulerView didMoveMarker:(NSRulerMarker *)marker;

@end
