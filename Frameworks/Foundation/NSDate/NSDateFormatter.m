/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSDateFormatter.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSTimeZone.h>
#include <math.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSKeyedUnarchiver.h>
#import <Foundation/NSCalendar.h>
#import <Foundation/NSNumber.h>

@implementation NSDateFormatter

-initWithDateFormat:(NSString *)format allowNaturalLanguage:(BOOL)flag {
    return [self initWithDateFormat:format allowNaturalLanguage:flag locale:[[NSUserDefaults standardUserDefaults] dictionaryRepresentation]];
}

-initWithDateFormat:(NSString *)format allowNaturalLanguage:(BOOL)flag locale:(NSDictionary *)locale {
    [super init];
    _behavior=NSDateFormatterBehavior10_0;
    _dateFormat10_0 = [format copy];
    _dateFormat = [format copy];
    _allowsNaturalLanguage = flag;
    _locale = [locale retain];
    _tz = [[NSTimeZone defaultTimeZone] retain];

    return self;
}

-init {
   return [self initWithDateFormat:@"" allowNaturalLanguage:NO];
}

-(void)dealloc {
    [_dateFormat10_0 release];
    [_dateFormat release];
    [_locale release];
    [_tz release];

    [super dealloc];
}

-initWithCoder:(NSCoder*)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSDictionary *attributes=[coder decodeObjectForKey:@"NS.attributes"];

    _dateFormat10_0=[[attributes objectForKey:@"dateFormat_10_0"] copy];
    _behavior=[[attributes objectForKey:@"formatterBehavior"] intValue];
    _dateStyle=[[attributes objectForKey:@"dateStyle"] intValue];
    _timeStyle=[[attributes objectForKey:@"timeStyle"] intValue];
    _dateFormat=[[coder decodeObjectForKey:@"NS.format"] retain];
    _allowsNaturalLanguage=[coder decodeBoolForKey:@"NS.natural"];
    _tz = [[coder decodeObjectForKey:@"timeZone"] retain];

   }

   return self;
}

-(NSString *)dateFormat {
    return _dateFormat;
}

-(BOOL)allowsNaturalLanguage {
    return _allowsNaturalLanguage;
}

-(NSDateFormatterBehavior)formatterBehavior {
   return _behavior;
}

-(NSDictionary *)locale {
    return _locale;
}

-(void)setDateFormat:(NSString *)format {
   format=[format copy];
   [_dateFormat10_0 release];
   _dateFormat10_0 = format;

   format=[format copy];
   [_dateFormat release];
   _dateFormat = format;
}

-(void)setTimeZone:(NSTimeZone*)tz {
    NSParameterAssert(tz);
    [_tz autorelease];
    _tz = [tz retain];
}

-(NSTimeZone*)timeZone {
    return _tz;
}

- (NSString *)stringFromDate:(NSDate *)date {
	return NSStringWithDateFormatLocale([date timeIntervalSinceReferenceDate], [self dateFormat], nil, _tz);
}

- (NSDate*)dateFromString:(NSString*)string {
    return NSDateWithStringDateFormatLocale(string, [self dateFormat], nil, _tz);
}

- (NSArray *)shortStandaloneWeekdaySymbols {

	return [[[NSUserDefaults standardUserDefaults] dictionaryRepresentation] objectForKey:
NSShortWeekDayNameArray];
}
- (NSArray *)standaloneWeekdaySymbols {
	return [[[NSUserDefaults standardUserDefaults] dictionaryRepresentation] objectForKey:
NSWeekDayNameArray];
}

-(void)setLenient:(BOOL)value {
}

-(void)setFormatterBehavior:(NSDateFormatterBehavior)value {
   _behavior=value;
}

-(NSString *)stringForObjectValue:(id)object {

   if([object isKindOfClass:[NSDate class]])
    return NSStringWithDateFormatLocale([object timeIntervalSinceReferenceDate], _dateFormat10_0, _locale, _tz);
   if([object isKindOfClass:[NSCalendarDate class]]) 
    return NSStringWithDateFormatLocale([object timeIntervalSinceReferenceDate], _dateFormat10_0, _locale, [object timeZone]);

   return nil;
}

-(NSAttributedString *)attributedStringForObjectValue:(id)object
   withDefaultAttributes:(NSDictionary *)attributes {
    return [[[NSAttributedString allocWithZone:NULL] initWithString:[self stringForObjectValue:object] attributes:attributes] autorelease];
}

-(NSString *)editingStringForObjectValue:(id)object {
    return [self stringForObjectValue:object];
}

-(BOOL)getObjectValue:(id *)object forString:(NSString *)string errorDescription:(NSString **)error {
    *object = NSDateWithStringDateFormatLocale(string, _dateFormat10_0, _locale, _tz);
    if (*object == nil) {
// FIX localization
       if(error!=NULL)
        *error = @"Couldn't convert string to a valid NSCalendarDate object.";
        return NO;
    }

    return YES;
}

-(BOOL)isPartialStringValid:(NSString *)partialString
   newEditingString:(NSString **)newString errorDescription:(NSString **)error {
   NSUnimplementedMethod();
   return NO;
}

@end


@interface NSMutableString(NSDateFormatterExtensions)

-(void)__appendCharacter:(unichar)character;

-(void)__appendLocale:(NSDictionary *)locale key:(NSString *)key
              index:(NSInteger)index;

@end

@implementation NSMutableString(NSDateFormatterExtensions)

// blek, get rid of these
-(void)__appendCharacter:(unichar)character {
    [self appendString:[NSString stringWithCharacters:&character length:1]];
}

-(void)__appendLocale:(NSDictionary *)locale key:(NSString *)key
              index:(NSInteger)index {
    NSArray *array=[locale objectForKey:key];

    if(array!=nil)
        [self appendString:[array objectAtIndex:index]];
}

@end

NSTimeInterval NSMoveIntervalFromTimeZoneToGMT(NSTimeInterval interval,
                                                NSTimeZone *timeZone) {
    int offset = [timeZone secondsFromGMTForDate:[NSDate dateWithTimeIntervalSinceReferenceDate:interval]];
    return interval - (double)offset;
}

NSTimeInterval NSMoveIntervalFromGMTToTimeZone(NSTimeInterval interval, NSTimeZone *timeZone) {
	int offset = [timeZone secondsFromGMTForDate:[NSDate dateWithTimeIntervalSinceReferenceDate:interval]]; 
    return interval + (double)offset;
}

#define NSDaysOfCommonEraOfReferenceDate	730486

// thirty days hath september, april, june, and november.
// all the rest have thirty-one, except February, which is borked.
NSInteger NSNumberOfDaysInMonthOfYear(NSInteger month, NSInteger year) {
    switch (month) {
        case 2:
            if (((year % 4) == 0 && (year % 100) != 0) || (year % 400) == 0)
                return 29;
            else
                return 28;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        default:
            return 31;
    }
}

static inline NSInteger numberOfDaysInCommonEraOfDayMonthAndYear(NSInteger day, NSInteger month, NSInteger year) {
    NSInteger result = 0;

    for (month--; month > 0; month--)
        result += NSNumberOfDaysInMonthOfYear(month, year);

    result += 365 * (year-1);
    result += (year - 1)/4;
    result -= (year - 1)/100;
    result += (year - 1)/400;

    // wtf, i tried this using day as the result variable and it started from zero
    result += day;

    return result;
}

NSTimeInterval NSTimeIntervalWithComponents(NSInteger year, NSInteger month, NSInteger day, NSInteger hour, NSInteger minute, NSInteger second, NSInteger milliseconds) {
    NSInteger daysOfCommonEra;
    NSTimeInterval interval;

    daysOfCommonEra = numberOfDaysInCommonEraOfDayMonthAndYear(day, month, year);
    daysOfCommonEra -= NSDaysOfCommonEraOfReferenceDate;

    interval = (daysOfCommonEra * 86400.0) + (hour * 3600) + (minute * 60) + second;
    
    if (milliseconds) {
        interval += milliseconds/1000.0 + 0.0001;
    }

    return interval;
}

NSInteger NSDayOfCommonEraFromTimeInterval(NSTimeInterval interval) {
    return (interval/86400.0) + NSDaysOfCommonEraOfReferenceDate;
}

NSInteger NSYearFromTimeInterval(NSTimeInterval interval) {
    NSInteger days = NSDayOfCommonEraFromTimeInterval(interval);
    NSInteger year = days/366;

    while (days >= numberOfDaysInCommonEraOfDayMonthAndYear(1, 1, year+1))
        year++;

    return year;
}

NSInteger NSDayOfYearFromTimeInterval(NSTimeInterval interval){ // 1-366
    NSInteger year = NSYearFromTimeInterval(interval);
    NSInteger result;

    result = NSDayOfCommonEraFromTimeInterval(interval) - numberOfDaysInCommonEraOfDayMonthAndYear(1, 1, year) + 1;
    if(result == 0) { result = 366; };

    return result;
}

NSInteger NSMonthFromTimeInterval(NSTimeInterval interval){ // 1-12
    NSInteger year = NSYearFromTimeInterval(interval);
    NSInteger days = NSDayOfCommonEraFromTimeInterval(interval);
    NSInteger month = 1;

    while (days > numberOfDaysInCommonEraOfDayMonthAndYear(NSNumberOfDaysInMonthOfYear(month, year), month, year))
        month++;

    return month;
}

NSInteger NSDayOfMonthFromTimeInterval(NSTimeInterval interval){ // 1-31
    NSInteger dayOfCommonEra = NSDayOfCommonEraFromTimeInterval(interval);
    NSInteger year = NSYearFromTimeInterval(interval);
    NSInteger month = NSMonthFromTimeInterval(interval);

    dayOfCommonEra -= numberOfDaysInCommonEraOfDayMonthAndYear(1, month, year) - 1;

    return dayOfCommonEra;
}

NSInteger NSWeekdayFromTimeInterval(NSTimeInterval interval){ // 1-7
    NSInteger weekday = NSDayOfCommonEraFromTimeInterval(interval);

    weekday = weekday % 7;
    if (weekday < 0)
        weekday += 7;

    return weekday;
}

NSInteger NS24HourFromTimeInterval(NSTimeInterval interval){ // 0-23
    NSTimeInterval hour = NSDayOfCommonEraFromTimeInterval(interval);

    hour -= NSDaysOfCommonEraOfReferenceDate;
    hour *= 86400.0;
    hour -= interval;
    hour /= 3600;
    hour = fabs(hour);

    if (hour == 24)
        hour = 0;

    return hour;
}

NSInteger NS12HourFromTimeInterval(NSTimeInterval interval){ // 1-12
    NSInteger hour = NS24HourFromTimeInterval(interval)%12;
    if (hour == 0)
        hour = 12;
    return hour;
}

NSInteger NSAMPMFromTimeInterval(NSTimeInterval interval){ // 0-1
    NSInteger hour=NS24HourFromTimeInterval(interval);

    return (hour < 11) ? 0 : 1;
}

NSInteger NSMinuteFromTimeInterval(NSTimeInterval interval){ // 0-59
    NSTimeInterval startOfHour = NSTimeIntervalWithComponents(NSYearFromTimeInterval(interval),
                                                              NSMonthFromTimeInterval(interval),
                                                              NSDayOfMonthFromTimeInterval(interval),
                                                              NS24HourFromTimeInterval(interval), 0, 0, 0);

    return (NSInteger)(interval - startOfHour)/60;
}

NSInteger NSSecondFromTimeInterval(NSTimeInterval interval){ // 0-59
    NSInteger seconds = fmod(floor(interval),60);
    if (seconds < 0)
        seconds = (60 + seconds);

    return seconds;
}

NSInteger NSMillisecondsFromTimeInterval(NSTimeInterval interval){ // 0-999
    NSInteger milli = fmod(interval*1000,1000);
    if (milli < 0) {
        milli = (1000 + milli);
    }
    return milli;
}

NSString *NSStringWithDateFormatLocale(NSTimeInterval interval,NSString *format,NSDictionary *locale,NSTimeZone *timeZone) {
    NSUInteger         pos,fmtLength=[format length];
    unichar          fmtBuffer[fmtLength],unicode;
    NSMutableString *result=[NSMutableString stringWithCapacity:fmtLength];

    unichar fillChar='0';
    BOOL    suppressZero=NO;

    enum {
        STATE_SCANNING,
        STATE_PERCENT,
        STATE_CONVERSION
    } state=STATE_SCANNING;

#if defined(__RAVYNOS__) // our clock is UTC as is good and proper
    interval=NSMoveIntervalFromGMTToTimeZone(interval,timeZone);
#else
    interval=NSMoveIntervalFromTimeZoneToGMT(interval,timeZone);
#endif
    if (locale == nil)
        locale = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];

    [format getCharacters:fmtBuffer];

    for(pos=0;pos<fmtLength;pos++){
        unicode=fmtBuffer[pos];

        switch(state){

            case STATE_SCANNING:
                if(unicode!='%')
                    [result __appendCharacter:unicode];
                else{
                    fillChar='0';
                    suppressZero=NO;
                    state=STATE_PERCENT;
                }
                    break;

            case STATE_PERCENT:
                switch(unicode){

                    case '.': suppressZero=YES; break;
                    case '_': fillChar=' '; break;
                    case ' ': fillChar=' '; break;

                    default:
                        pos--;
                        state=STATE_CONVERSION;
                        break;
                }
                break;

            case STATE_CONVERSION:
                switch(unicode){

                    case '%':
                        [result __appendCharacter:'%'];
                        break;

                    case 'a':
                        [result __appendLocale:locale key:NSShortWeekDayNameArray
                                       index:NSWeekdayFromTimeInterval(interval)];
                        break;

                    case 'A':
                        [result __appendLocale:locale key:NSWeekDayNameArray
                                       index:NSWeekdayFromTimeInterval(interval)];
                        break;

                    case 'b':
                        [result __appendLocale:locale key:NSShortMonthNameArray
                                       index:NSMonthFromTimeInterval(interval)-1];
                        break;

                    case 'B':
                        [result __appendLocale:locale key:NSMonthNameArray
                                       index:NSMonthFromTimeInterval(interval)-1];
                        break;

                    case 'c':
                        [result appendFormat:@"%@", NSStringWithDateFormatLocale(
                            NSMoveIntervalFromTimeZoneToGMT(interval,timeZone),
                            [locale objectForKey:NSTimeDateFormatString],locale,timeZone)];
                        break;

                    case 'd':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%2d":@"%02d");

                        [result appendFormat:fmt,NSDayOfMonthFromTimeInterval(interval)];
                    }
                        break;

                    case 'D':
                        [result appendFormat:@"%@", NSStringWithDateFormatLocale(
                            NSMoveIntervalFromTimeZoneToGMT(interval,timeZone),
                            @"%m/%d/%y",locale,timeZone)];
                        break;
                    case 'e':{
                        id fmt=@"%d";
                        [result appendFormat:fmt,NSDayOfMonthFromTimeInterval(interval)];
                    }
                        break;

                   case 'F':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%3d":@"%03d");

                        [result appendFormat:fmt,NSMillisecondsFromTimeInterval(interval)];
                    }
                        break;

                    case 'H':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%2d":@"%02d");

                        [result appendFormat:fmt,NS24HourFromTimeInterval(interval)];
                    }
                        break;

                    case 'I':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%2d":@"%02d");

                        [result appendFormat:fmt,NS12HourFromTimeInterval(interval)];
                    }
                        break;

                    case 'j':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%3d":@"%03d");

                        [result appendFormat:fmt,NSDayOfYearFromTimeInterval(interval)];
                    }
                        break;

                    case 'm':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%2d":@"%02d");

                        [result appendFormat:fmt,NSMonthFromTimeInterval(interval)];
                    }
                        break;

                    case 'M':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%2d":@"%02d");

                        [result appendFormat:fmt,NSMinuteFromTimeInterval(interval)];
                    }
                        break;

                    case 'p':
                        [result __appendLocale:locale key:NSAMPMDesignation
                                       index:NSAMPMFromTimeInterval(interval)];
                        break;
                    case 'r':
                        [result appendFormat:@"%@", NSStringWithDateFormatLocale(
                            NSMoveIntervalFromTimeZoneToGMT(interval,timeZone),
                            @"%I:%M:%S %p",locale,timeZone)];
                        break;
                    case 'R':
                        [result appendFormat:@"%@", NSStringWithDateFormatLocale(
                            NSMoveIntervalFromTimeZoneToGMT(interval,timeZone)
                            ,@"%H:%M",locale,timeZone)];
                        break;

                    case 'S':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%2d":@"%02d");

                        [result appendFormat:fmt,NSSecondFromTimeInterval(interval)];
                    }
                        break;
                    case 'T':
                        [result appendFormat:@"%@", NSStringWithDateFormatLocale(
                            NSMoveIntervalFromTimeZoneToGMT(interval,timeZone),
                            @"%H:%M:%S",locale,timeZone)];
                        break;

                    case 'w':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%1d":@"%01d");

                        [result appendFormat:fmt,NSWeekdayFromTimeInterval(interval)];
                    }
                        break;

                    case 'x':
                        [result appendFormat:@"%@", NSStringWithDateFormatLocale(
                            NSMoveIntervalFromTimeZoneToGMT(interval,timeZone),
                            [locale objectForKey:NSDateFormatString],locale,timeZone)];
                        break;

                    case 'X':
                        [result appendFormat:@"%@", NSStringWithDateFormatLocale(
                            NSMoveIntervalFromTimeZoneToGMT(interval,timeZone),
                            [locale objectForKey:NSTimeFormatString],locale,timeZone)];
                        break;

                    case 'y':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%2d":@"%02d");

                        [result appendFormat:fmt,NSYearFromTimeInterval(interval)%100];
                    }
                        break;

                    case 'Y':{
                        id fmt=(suppressZero)?@"%d":((fillChar==' ')?@"%4d":@"%04d");

                        [result appendFormat:fmt,NSYearFromTimeInterval(interval)];
                    }
                        break;

                    case 'Z':
                        [result appendString:[timeZone name]];
                        break;

                    case 'z': {
                        NSInteger  secondsFromGMT = [timeZone secondsFromGMTForDate:[NSDate dateWithTimeIntervalSinceReferenceDate:interval]];
                        [result appendString:[[[NSString allocWithZone:NULL] initWithFormat:@"%@%02d%02d",
                                                                          (secondsFromGMT >= 0 ? @"+" : @""),
                                                                          secondsFromGMT/3600,
                                                                          (secondsFromGMT % 3600)/60] autorelease]];
                    }
                        break;
                }
                state=STATE_SCANNING;
                break;
        }

    }

    return result;
}

NSString *NSReadStringInString(NSString *aString, NSCharacterSet *characterSet, NSUInteger position, NSUInteger maxLength, NSUInteger *endPosition)
{
    NSMutableString *resultString = [NSMutableString stringWithCapacity:maxLength];

    for(NSUInteger i = 0; i < maxLength; i++) {
        if(position+i >= [aString length]) {
            break;
        }

        unichar c = [aString characterAtIndex:position+i];
        if([characterSet characterIsMember:c] == NO) {
            break;
        }
        [resultString appendFormat:@"%c", c];
    }
    *endPosition = position + [resultString length];
    return resultString;
}

NSInteger NSReadIntegerInString(NSString *aString, NSCharacterSet *characterSet, NSUInteger position, NSUInteger maxLength, NSUInteger *endPosition)
{
    NSString    *str;
    unichar     firstChar = [aString characterAtIndex:position];
    BOOL        negate = NO;
    NSInteger   i;

    if (firstChar == '+') {
        position++;
    }
    else  if (firstChar == '-') {
        position++;
        negate = YES;
    }

    str = NSReadStringInString(aString, characterSet, position, maxLength, endPosition);
    i = [str integerValue];

    if (negate == YES) {
        i *= -1;
    }

    return i;
}


// might as well use the same code since they're the exact same formatting specifiers
// ok. we need at minimum the year. everything else is optional.
// weekday information is useless.
NSDate *NSDateWithStringDateFormatLocale(NSString *string, NSString *format, NSDictionary *locale, NSTimeZone *timeZone) {
    NSScanner       *scanner = [NSScanner scannerWithString:string];
    NSUInteger         currentPosition,fmtLength=[format length];
    unichar          fmtBuffer[fmtLength],unicode;
    NSInteger		     years = NSNotFound, months = NSNotFound, days = NSNotFound, hours = NSNotFound, minutes = NSNotFound, seconds = NSNotFound, milliseconds = NSNotFound;
    NSInteger		     AMPMMultiplier = 0;
    NSTimeInterval   adjustment = 0;
    NSArray	    *monthNames, *shortMonthNames, *AMPMDesignations;
    NSTimeInterval   timeInterval;
    NSDate  *date;
    
    [scanner setCharactersToBeSkipped:nil];

    enum {
        STATE_SCANNING,
        STATE_PERCENT,
        STATE_CONVERSION
    } state = STATE_SCANNING;

    if ([string length] == 0) {
        return nil;
    }

    if (locale == nil) {
        locale = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
    }

    monthNames = [locale objectForKey:NSMonthNameArray];
    shortMonthNames = [locale objectForKey:NSShortMonthNameArray];
    AMPMDesignations = [locale objectForKey:NSAMPMDesignation];

    // although we don't use the weekday arrays for anything, the spec
    // says to check them anyway.
    if ([monthNames count] > 12 || [shortMonthNames count] > 12 ||
            [[locale objectForKey:NSShortWeekDayNameArray] count] > 7 ||
            [[locale objectForKey:NSWeekDayNameArray] count] > 7) {
        return nil;
    }

    [format getCharacters:fmtBuffer];

    for (currentPosition = 0; currentPosition < fmtLength; currentPosition++) {
        unicode = fmtBuffer[currentPosition];

        switch (state) {
            case STATE_SCANNING:
                if (unicode == '%') {
                    state = STATE_PERCENT;
                } else {
                    currentPosition++;
                }
                break;

            case STATE_PERCENT:
                switch (unicode) {
                    case '.':
                    case ' ':
                    default:
                        currentPosition--;
                        state = STATE_CONVERSION;
                        break;
                }
                break;

            case STATE_CONVERSION:
                switch (unicode) {
                    case '%':
                        currentPosition++;
                        break;

                        // can't really do anything with the day of the week, but we have to skip it.
                    case 'a':
                        NSReadStringInString(string, [NSCharacterSet letterCharacterSet], currentPosition, 255, &currentPosition);
                        break;
                    case 'A':
                        NSReadStringInString(string, [NSCharacterSet letterCharacterSet], currentPosition, 255, &currentPosition);
                        break;

                        // month or its abbreviation. look it up in the arrays..
                    case 'b': {
                        NSString *temp;
                        NSEnumerator *enumerator = [shortMonthNames objectEnumerator];
                        NSString *shortMonthName;
                        months = NSNotFound;
                        int month = 1;

                        temp = NSReadStringInString(string, [NSCharacterSet letterCharacterSet], currentPosition, 255, &currentPosition);

                        while ((shortMonthName = [enumerator nextObject]) != nil) {
                            if ([shortMonthName caseInsensitiveCompare:temp] == NSOrderedSame) {
                                months = month;
                                break;
                            } else {
                                month++;
                            }
                        }

                        //month not found
                        if (months == NSNotFound) {
                            return nil;
                        }
                        break;
                    }

                    case 'B': {
                        NSString *temp;
                        NSEnumerator *enumerator = [monthNames objectEnumerator];
                        NSString *monthName;
                        months = NSNotFound;
                        int month = 1;

                        temp = NSReadStringInString(string, [NSCharacterSet letterCharacterSet], currentPosition, 255, &currentPosition);

                        while ((monthName = [enumerator nextObject]) != nil) {
                            if ([monthName caseInsensitiveCompare:temp] == NSOrderedSame) {
                                months = month;
                                break;
                            } else {
                                month++;
                            }
                        }

                        //month not found
                        if (months == NSNotFound) {
                            return nil;
                        }
                        break;
                    }

                    case 'c':
                        return NSDateWithStringDateFormatLocale(string, [locale objectForKey:NSTimeDateFormatString], locale, timeZone);

                    case 'd':
                        days = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 2, &currentPosition);
                        break;

                    case 'F':
                        milliseconds = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 3, &currentPosition);
                        break;

                    case 'H':
                        hours = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 2, &currentPosition);
                        break;

                    case 'I':
                        hours = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 2, &currentPosition);

                        AMPMMultiplier = 1;
                        break;

                    // grr
                    case 'j': {
                        NSInteger numberOfDays = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 3, &currentPosition);

                        adjustment += numberOfDays * 86400.0;
                        break;
                    }

                    case 'm':
                        months = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 2, &currentPosition);

                        break;

                    case 'M':
                        minutes = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 2, &currentPosition);
                        break;

                    case 'p': {
                        NSString *temp;

                        temp = NSReadStringInString(string, [NSCharacterSet letterCharacterSet], currentPosition, 255, &currentPosition);

                        AMPMMultiplier = [AMPMDesignations indexOfObject:temp];
                        if (AMPMMultiplier == NSNotFound) {
                            return nil;
                        }
                        AMPMMultiplier++; // e.g. 0 = 1, 1 = 2...
                        break;
                    }

                    case 'S':
                        seconds = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 2, &currentPosition);
                        break;

                    // again, weekdays are useless
                    case 'w': {
                        NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 1, &currentPosition);
                        break;
                    }

                    case 'x':
                        return NSDateWithStringDateFormatLocale(string,[locale objectForKey:NSDateFormatString],locale, timeZone);

                    case 'X':
                        return NSDateWithStringDateFormatLocale(string,[locale objectForKey:NSTimeFormatString],locale, timeZone);

                    case 'y':
                        years = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 2, &currentPosition);

// FIX QUESTIONABLE
// 1900 or 2000??, YB does 2000, for some? all?
                        years += 2000;
                        break;

                    case 'Y':
                        years = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 4, &currentPosition);
                        /*if (![scanner scanInteger:&years])
                            return nil;*/
                        break;

                    case 'Z': {
                        NSString *temp;

                        temp = NSReadStringInString(string, [NSCharacterSet letterCharacterSet], currentPosition, 255, &currentPosition);

                        timeZone = [NSTimeZone timeZoneWithName:temp];
                        break;
                    }

                    case 'z': {
                        NSInteger hoursMinutes, tzHours, tzMinutes;
                        hoursMinutes = NSReadIntegerInString(string, [NSCharacterSet decimalDigitCharacterSet], currentPosition, 4, &currentPosition);

                        tzHours = hoursMinutes / 100;
                        tzMinutes = hoursMinutes % 100;
                        timeZone = [NSTimeZone timeZoneForSecondsFromGMT:(tzHours * 3600) + (tzMinutes * 60)];
                        break;
                    }
                }
                state = STATE_SCANNING;
                break;
        }
    }

    // now that we've got whatever information we can get from the string,
    // try to make an NSCalendarDate of it.
    if (AMPMMultiplier != 0 && hours != NSNotFound) {
        hours *= AMPMMultiplier;
    }

    // maybe we've been given the number of days in the year but not the month/day
    if (months == NSNotFound && days == NSNotFound) {
        months = 1;
        days = 1;
    }

    // if no year, then this year
    if (years == NSNotFound) {
        years = [[NSCalendarDate date] yearOfCommonEra];
    }

    if (hours == NSNotFound) {
        hours = 0;
    }
    if (minutes == NSNotFound) {
        minutes = 0;
    }
    if (seconds == NSNotFound) {
        seconds = 0;
    }
    if (milliseconds == NSNotFound) {
        milliseconds = 0;
    }

    if (timeZone == nil) {
        timeZone = [NSTimeZone defaultTimeZone];
    }

    timeInterval = NSTimeIntervalWithComponents(years, months, days, hours, minutes, seconds, milliseconds);
    timeInterval += adjustment;

    timeInterval = timeInterval - [timeZone secondsFromGMTForDate:[NSDate dateWithTimeIntervalSinceReferenceDate:timeInterval]];

    date = [[[NSDate allocWithZone:NULL] initWithTimeIntervalSinceReferenceDate:timeInterval] autorelease];
    return date;
}
