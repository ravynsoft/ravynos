/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSMeasurementUnit.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSValue.h>

static NSMutableArray *_measurementUnits = nil;

@implementation NSMeasurementUnit

+ (NSMeasurementUnit *)inchesMeasurementUnit
{
    return [self measurementUnitWithName:@"Inches" 
                            abbreviation:@"in" 
                           pointsPerUnit:72.0 
                             stepUpCycle:[NSArray arrayWithObject:[NSNumber numberWithFloat:2.0]]
                           stepDownCycle:[NSArray arrayWithObjects:[NSNumber numberWithFloat:0.5], [NSNumber numberWithFloat:0.25], [NSNumber numberWithFloat:0.125], nil]];
}

+ (NSMeasurementUnit *)centimetersMeasurementUnit
{
    return [self measurementUnitWithName:@"Centimeters" 
                            abbreviation:@"cm" 
                           pointsPerUnit:28.35 
                             stepUpCycle:[NSArray arrayWithObject:[NSNumber numberWithFloat:2.0]]
                           stepDownCycle:[NSArray arrayWithObjects:[NSNumber numberWithFloat:0.5], [NSNumber numberWithFloat:0.2], nil]];
}

+ (NSMeasurementUnit *)pointsMeasurementUnit
{
    return [self measurementUnitWithName:@"Points" 
                            abbreviation:@"pt" 
                           pointsPerUnit:1.0 
                             stepUpCycle:[NSArray arrayWithObject:[NSNumber numberWithFloat:10.0]]
                           stepDownCycle:[NSArray arrayWithObject:[NSNumber numberWithFloat:0.5]]];
}

+ (NSMeasurementUnit *)picasMeasurementUnit
{
    return [self measurementUnitWithName:@"Picas" 
                            abbreviation:@"pc" 
                           pointsPerUnit:12.0 
                             stepUpCycle:[NSArray arrayWithObject:[NSNumber numberWithFloat:10.0]]
                           stepDownCycle:[NSArray arrayWithObject:[NSNumber numberWithFloat:0.5]]];
}

+ (NSArray *)allMeasurementUnits
{
    return _measurementUnits;
}

+ (NSMeasurementUnit *)measurementUnitNamed:(NSString *)name
{
    int i, count =  [_measurementUnits count];
    
    for (i = 0; i < count; ++i)
        if ([[[_measurementUnits objectAtIndex:i] name] isEqualToString:name])
            return [_measurementUnits objectAtIndex:i];
    
    return nil;
}

+ (void)registerUnit:(NSMeasurementUnit *)unit
{
    [_measurementUnits addObject:unit];
}

+ (void)initialize
{
    if (_measurementUnits == nil)
        _measurementUnits = [[NSMutableArray alloc] init];
    
    [self registerUnit:[NSMeasurementUnit inchesMeasurementUnit]];
    [self registerUnit:[NSMeasurementUnit centimetersMeasurementUnit]];
    [self registerUnit:[NSMeasurementUnit pointsMeasurementUnit]];
    [self registerUnit:[NSMeasurementUnit picasMeasurementUnit]];
}

+ (NSMeasurementUnit *)measurementUnitWithName:(NSString *)name abbreviation:(NSString *)abbreviation pointsPerUnit:(float)points stepUpCycle:(NSArray *)upCycle stepDownCycle:(NSArray *)downCycle
{
    return [[[self alloc] initWithName:name abbreviation:abbreviation pointsPerUnit:points stepUpCycle:upCycle stepDownCycle:downCycle] autorelease];
}

- (id)initWithName:(NSString *)name abbreviation:(NSString *)abbreviation pointsPerUnit:(float)points stepUpCycle:(NSArray *)upCycle stepDownCycle:(NSArray *)downCycle;
{
    _name = [name retain];
    _abbreviation = [abbreviation retain];
    _pointsPerUnit = points;
    _stepUpCycle = [upCycle retain];
    _stepDownCycle = [downCycle retain];
    
    return self;
}

- (void)dealloc
{
    [_name release];
    [_abbreviation release];
    [_stepUpCycle release];
    [_stepDownCycle release];
    
    [super dealloc];
}

- (NSString *)name
{
    return _name;
}

- (NSString *)abbreviation
{
    return _abbreviation;
}

- (float)pointsPerUnit
{
    return _pointsPerUnit;
}

- (NSArray *)stepUpCycle
{
    return _stepUpCycle;
}

- (NSArray *)stepDownCycle
{
    return _stepDownCycle;
}

@end
