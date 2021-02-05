/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifdef PLATFORM_IS_POSIX
#include <time.h>
#import <Foundation/NSTimeZone_posix.h>
#import <Foundation/NSTimeZone.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSByteOrder.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSByteOrder.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSRaiseException.h>


// structures in tzfiles are big-endian
// for definition of file format see
// http://www.kernel.org/doc/man-pages/online/pages/man5/tzfile.5.html

#define TZ_MAGIC "TZif"

#pragma pack(push)
#pragma pack(1)
struct tzhead {
    char tzh_magic[4];       /* TZ_MAGIC */
    char tzh_version[1];     /* version of the file's format (as of 2005, either an ASCII NUL ('\0')
                                or a '2') */
    char tzh_reserved[15];   /* reserved for future use */
    int32_t tzh_ttisgmtcnt;  /* coded number of trans. time flags */
    int32_t tzh_ttisstdcnt;  /* coded number of trans. time flags */
    int32_t tzh_leapcnt;     /* coded number of leap seconds */
    int32_t tzh_timecnt;     /* coded number of transition times */
    int32_t tzh_typecnt;     /* coded number of local time types */
    int32_t tzh_charcnt;     /* coded number of abbr. chars */
};

struct tzType {
    uint32_t offset;
    uint8_t isDST;
    uint8_t abbrevIndex;
};
#pragma pack(pop)


// private classes
#import <Foundation/NSTimeZoneTransition.h>
#import <Foundation/NSTimeZoneType.h>

@implementation NSTimeZone_posix

NSInteger sortTransitions(id trans1, id trans2, void *context) {
    NSDate  *d1 = [trans1 transitionDate];
    NSDate  *d2 = [trans2 transitionDate];

    return [d1 compare:d2];
}


-initWithName:(NSString *)name data:(NSData *)data
{
    NSMutableArray *transitions, *types;
    NSArray *sortedTransitions;
    const struct tzhead *tzHeader;
    const char *tzData;
    const char *typeIndices;
    //unused
    //int numberOfGMTFlags, numberOfStandardFlags, numberOfAbbreviationCharacters;
    int numberOfTransitionTimes, numberOfLocalTimes;
    int i;

    const struct tzType *tzTypes;
    const char *tzTypesBytes;
    const char *abbreviations;

    if (data == nil) {
        NSString *zonePath = [NSTimeZone_posix _zoneinfoPath];
        zonePath = [zonePath stringByAppendingPathComponent:name];
        data = [NSData dataWithContentsOfFile:zonePath];
    }
    if (data == nil) {
        [self release];
        return nil;
    }

    transitions = [NSMutableArray array];
    sortedTransitions = [NSArray array];
    types = [NSMutableArray array];

    tzHeader = (struct tzhead *)[data bytes];
    tzData = (const char *)tzHeader + sizeof(struct tzhead);

    //unused
    //numberOfGMTFlags = NSSwapBigIntToHost(tzHeader->tzh_ttisgmtcnt);
    //numberOfStandardFlags = NSSwapBigIntToHost(tzHeader->tzh_ttisstdcnt);
    //numberOfAbbreviationCharacters = NSSwapBigIntToHost(tzHeader->tzh_charcnt);
    numberOfTransitionTimes = NSSwapBigIntToHost(tzHeader->tzh_timecnt);
    numberOfLocalTimes = NSSwapBigIntToHost(tzHeader->tzh_typecnt);

    typeIndices = tzData + (numberOfTransitionTimes * 4);
    for (i = 0; i < numberOfTransitionTimes; ++i) {
        NSDate *d1 = [NSDate dateWithTimeIntervalSince1970:NSSwapBigIntToHost(((int *)tzData)[i])];
        [transitions addObject:[NSTimeZoneTransition timeZoneTransitionWithTransitionDate:d1
                typeIndex:typeIndices[i]]];
    }

    //sort date array
    sortedTransitions = [transitions sortedArrayUsingFunction:sortTransitions context:NULL];

    // this is a bit more awkward, but i want to support non-3 character abbreviations theoretically.
    tzTypesBytes = (tzData + (numberOfTransitionTimes * 5));
    abbreviations = tzTypesBytes + numberOfLocalTimes * sizeof(struct tzType);
    for (i = 0; i < numberOfLocalTimes; ++i) {
        tzTypes = (struct tzType *)tzTypesBytes;
        NSString *abb = [NSString stringWithCString:abbreviations + tzTypes->abbrevIndex];
        if (name == nil) {
            name = abb;
        }
        [types addObject:[NSTimeZoneType timeZoneTypeWithSecondsFromGMT:NSSwapBigIntToHost(tzTypes->offset)
                isDaylightSavingTime:tzTypes->isDST
                abbreviation:[NSString stringWithCString:abbreviations+tzTypes->abbrevIndex]]];
        tzTypesBytes += sizeof(struct tzType);
    }

    return [self initWithName:name data:data transitions:sortedTransitions types:types];
}


-initWithName:(NSString *)name data:(NSData *)data transitions:(NSArray *)transitions types:(NSArray *)types {
    _name = [name retain];
    _data = [data retain];
    _timeZoneTransitions = [transitions retain];
    _timeZoneTypes = [types retain];

    return self;
}

-(void)dealloc {
    [_name release];
    [_data release];
    [_timeZoneTransitions release];
    [_timeZoneTypes release];

    [super dealloc];
}

-(NSString *)name {
    return _name;
}

-(NSData *)data {
    return _data;
}


+ (NSTimeZone *)systemTimeZone
{
    NSTimeZone *systemTimeZone = nil;
    NSString *timeZoneName;

    if ([[NSFileManager defaultManager] fileExistsAtPath:@"/etc/localtime"] == YES) {
        NSError *error;
        NSString *path = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath:@"/etc/localtime" error:&error];

        if (path != nil) {
            //localtime is a symlink
            timeZoneName = [path stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@/", [NSTimeZone_posix _zoneinfoPath]] withString:@""];
            systemTimeZone = [self timeZoneWithName:timeZoneName];
        } else {
            //localtime is a file
            systemTimeZone = [[[NSTimeZone alloc] initWithName:nil data:[NSData dataWithContentsOfFile:@"/etc/localtime"]] autorelease];
        }
    }

    if (systemTimeZone == nil) {
        //try to use TZ environment variable
        const char *envTimeZoneName = getenv("TZ");

        if (envTimeZoneName != NULL) {
            systemTimeZone = [self timeZoneWithName:[NSString stringWithCString:envTimeZoneName]];
        }
    }

    if (systemTimeZone == nil) {
        NSString *abbreviation;

        tzset();
        abbreviation = [NSString stringWithCString:tzname[0]];

        systemTimeZone = [self timeZoneWithAbbreviation:abbreviation];

#ifdef LINUX
        if (systemTimeZone == nil) {
            //check if the error is because of a missing entry in NSTimeZoneAbbreviations.plist (only for logging)
            if ([[self abbreviationDictionary] objectForKey:abbreviation] == nil) {
                NSCLog("Abbreviation [%s] not found in NSTimeZoneAbbreviations.plist -> using absolute timezone (no daylight saving)", [abbreviation cString]);
            } else {
                NSCLog("TimeZone [%s] not instantiable -> using absolute timezone (no daylight saving)", [[[self abbreviationDictionary] objectForKey:abbreviation] cString]);
            }

            systemTimeZone = [NSTimeZone timeZoneForSecondsFromGMT:timezone];
        }
#endif
    }

    return systemTimeZone;
}


-(NSTimeZoneType *)timeZoneTypeForDate:(NSDate *)date {
    if ([_timeZoneTransitions count] == 0 ||
        [date compare:[[_timeZoneTransitions objectAtIndex:0] transitionDate]] == NSOrderedAscending) {

        NSEnumerator *timeZoneTypeEnumerator = [_timeZoneTypes objectEnumerator];
        NSTimeZoneType *type;

        while ((type = [timeZoneTypeEnumerator nextObject])!=nil) {
            if (![type isDaylightSavingTime])
                return type;
        }

        return [_timeZoneTypes objectAtIndex:0];
    }
    else {
        NSEnumerator *timeZoneTransitionEnumerator = [_timeZoneTransitions objectEnumerator];
        NSTimeZoneTransition *transition, *previousTransition = nil;

        while ((transition = [timeZoneTransitionEnumerator nextObject])!=nil) {
            if ([date compare:[transition transitionDate]] == NSOrderedDescending) {
                previousTransition = transition;
            }
            else
                return [_timeZoneTypes objectAtIndex:[previousTransition typeIndex]];
        }

        return [_timeZoneTypes lastObject];
    }
    //don't use date description in exception text, because of recursion
    [NSException raise:NSInternalInconsistencyException
                format:@"%@ could not determine seconds from GMT for timeInterval %d since reference date", self, [date timeIntervalSinceReferenceDate]];
    return nil;
}

-(NSInteger)secondsFromGMTForDate:(NSDate *)date {
    return [[self timeZoneTypeForDate:date] secondsFromGMT];
}

-(NSString *)abbreviationForDate:(NSDate *)date {
    return [[self timeZoneTypeForDate:date] abbreviation];
}

-(BOOL)isDaylightSavingTimeForDate:(NSDate *)date {
    return [[self timeZoneTypeForDate:date] isDaylightSavingTime];
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] name: %@ secondsFromGMT: %d isDaylightSavingTime: %@ abbreviation: %@>",
        [self class], self,
        [self name], [self secondsFromGMT], [self isDaylightSavingTime] ? @"YES" : @"NO", [self abbreviation]];
}

-copyWithZone:(NSZone *)zone {
    return [self retain];
}

-(void)encodeWithCoder:(NSCoder *)coder {
    [coder encodeObject:_name];
    [coder encodeObject:_data];
    [coder encodeObject:_timeZoneTransitions];
    [coder encodeObject:_timeZoneTypes];
}

-initWithCoder:(NSCoder *)coder {
    _name = [[coder decodeObject] retain];
    _data = [[coder decodeObject] retain];
    _timeZoneTransitions = [[coder decodeObject] retain];
    _timeZoneTypes = [[coder decodeObject] retain];

    return self;
}

+(NSString*)_zoneinfoPath
{
    static NSString *zoneinfoPath = nil;
    if(zoneinfoPath == nil) {
        BOOL            isDir;
        NSFileManager   *fileManager = [NSFileManager defaultManager];

        //we can create some subclasses for all os or a method on NSPlatform instead of this if else cascade
        if ([fileManager fileExistsAtPath:@"/usr/share/zoneinfo" isDirectory:&isDir] && isDir) { // os x & linux
            return @"/usr/share/zoneinfo";
        }
        else if ([fileManager fileExistsAtPath:@"/usr/share/lib/zoneinfo" isDirectory:&isDir] && isDir) { // solaris
            return @"/usr/share/lib/zoneinfo";
        }
        else if ([fileManager fileExistsAtPath:@"/usr/lib/zoneinfo" isDirectory:&isDir] && isDir) { // older linux
            return @"/usr/lib/zoneinfo";
        }
        else {
            [NSException raise:NSInternalInconsistencyException
                        format:@"could not find zoneinfo directory"];
            // compiler does not know if NSException+raise:… throws
            return nil;
        }
    }
    else {
        return zoneinfoPath;
    }
}

@end
#endif

