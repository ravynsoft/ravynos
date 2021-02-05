#ifdef WINDOWS
/* Copyright (c) 2009-2010 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSTimeZone_win32.h>
#import <Foundation/NSPlatform_win32.h>
#import <Foundation/NSString_win32.h>

#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSDateFormatter.h>
#import <Foundation/NSKeyedUnarchiver.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#pragma pack(1)

typedef struct _REG_TZI_FORMAT
{
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

#pragma pack()


//   TODO:
//   Dynamic DST (see SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\*\\Dynamic DST in registry)


//this function (without suffix '_priv') should be in kernel32, but in mingw its not :-(
WINBASEAPI BOOL WINAPI TzSpecificLocalTimeToSystemTime_priv(LPTIME_ZONE_INFORMATION lpTimeZoneInformation, LPSYSTEMTIME lpLocalTime, LPSYSTEMTIME lpUniversalTime)
{
	DWORD dwTzID = 0;
	FILETIME ft = {0};
	LONG lBias = 0;
	TIME_ZONE_INFORMATION tzinfo = {0};
	ULARGE_INTEGER uliGreg = {{0}};
    
	if (lpTimeZoneInformation != NULL) {
		tzinfo = *lpTimeZoneInformation;
	}else {
		dwTzID = GetTimeZoneInformation(&tzinfo);
		if ((dwTzID==TIME_ZONE_ID_UNKNOWN) || (dwTzID==TIME_ZONE_ID_INVALID)) {
			return FALSE;
		}else if (dwTzID == TIME_ZONE_ID_DAYLIGHT) {
			lBias = tzinfo.DaylightBias + tzinfo.Bias;
		}else if (dwTzID == TIME_ZONE_ID_STANDARD) {
			lBias = tzinfo.StandardBias + tzinfo.Bias;
		}
	}
    
	if (!SystemTimeToFileTime(lpLocalTime, &ft) ) {
		return FALSE;
	}
    
	uliGreg.HighPart = ft.dwHighDateTime;
	uliGreg.LowPart = ft.dwLowDateTime;
    
	uliGreg.QuadPart += (ULONGLONG)lBias * 600000000;
    
	ft.dwHighDateTime = uliGreg.HighPart;
	ft.dwLowDateTime = uliGreg.LowPart;
    
	if(!FileTimeToSystemTime(&ft, lpUniversalTime)) {
		return FALSE;
	}
    
	return TRUE;
}

@implementation NSTimeZone_win32

-initWithCoder:(NSCoder *)coder {
    NSString *name=[coder decodeObjectForKey:@"NS.name"];
    
    return [self initWithName:name data:nil];
}

-initWithName:(NSString *)name data:(NSData *)data {
    
    NSString *registryName = nil;
    
    if(data == nil) {
        data = [NSTimeZone_win32 _getDataWithWindowsName:name registryName:&registryName];
    }
    
    if(data == nil) {
        NSDictionary    *abbDict = [NSTimeZone abbreviationDictionary];
        NSDictionary    *windowsZones = [NSTimeZone_win32 _windowsZones];
        
        for (id key in windowsZones) {
            NSArray *values = [abbDict objectForKey:key];
            
            if ([values indexOfObject:name] != NSNotFound) {
                data = [NSTimeZone_win32 _getDataWithWindowsName:key registryName:&registryName];
                break;

            }
        }
        
        if (data == nil) {
            [self release];
            return nil;
        }
    }
    
    _abbreviation = [[NSTimeZone_win32 _registryNameToAbbreviation:registryName] retain];
    _data = [data retain];
    _name = [name retain];
    
    return self;
    
}

-(void)dealloc {
    [_abbreviation release];
    [_data release];
    [_name release];
    
    [super dealloc];
}

-(void)encodeWithCoder:(NSCoder *)coder {
    NSInvalidAbstractInvocation();
}

-copyWithZone:(NSZone *)zone {
   NSTimeZone_win32 *result=NSCopyObject(self,0,zone);
   
   result->_data=[_data copy];
   result->_name=[_name copy];
   result->_abbreviation=[_abbreviation copy];

   return result;
}

+(NSTimeZone *)systemTimeZone {
    TIME_ZONE_INFORMATION   timeZoneInformation;
    NSString                *timeZoneName;

    GetTimeZoneInformation(&timeZoneInformation);
    timeZoneName = NSStringFromNullTerminatedUnicode(timeZoneInformation.StandardName);

    return [self timeZoneWithName:timeZoneName];
}

-(NSInteger)secondsFromGMTForDate:(NSDate *)date {
    SYSTEMTIME utc;
    SYSTEMTIME local = [NSTimeZone_win32 _dateToSystemTime:date];
    TIME_ZONE_INFORMATION t=[self _timeZoneInformation];
    LPTIME_ZONE_INFORMATION tzip= &t;
    NSDate *utcDate, *localDate;
    
    TzSpecificLocalTimeToSystemTime_priv(tzip, &local, &utc);
    SystemTimeToTzSpecificLocalTime(tzip, &utc ,&local );
    
    
    utcDate = [NSTimeZone_win32 _systemtimeToDate:utc];
    localDate = [NSTimeZone_win32 _systemtimeToDate:local];
    
    return [localDate timeIntervalSinceReferenceDate] - [utcDate timeIntervalSinceReferenceDate];
}

-(NSString *)abbreviationForDate:(NSDate *)date {
    return _abbreviation;
}

-(BOOL)isDaylightSavingTimeForDate:(NSDate *)date {
    NSUnimplementedMethod();
    return NO;
}

-(NSTimeInterval)daylightSavingTimeOffsetForDate:(NSDate *)date {
    NSUnimplementedMethod();
    return 0;
}

-(NSDate *)nextDaylightSavingTimeTransitionAfterDate:(NSDate *)date {
    NSUnimplementedMethod();
    return 0;
}

-(NSString *)localizedName:(NSTimeZoneNameStyle)style locale:(NSLocale *)locale {
    NSUnimplementedMethod();
    return 0;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] name: %@ (%@)>", [self class], self, [self name], [self abbreviation]];
}

-(TIME_ZONE_INFORMATION)_timeZoneInformation {
    TIME_ZONE_INFORMATION timezoneInformation;
    REG_TZI_FORMAT  *tzi = (REG_TZI_FORMAT *)[self->_data bytes];
    
    timezoneInformation.Bias          = tzi->Bias;
    timezoneInformation.StandardDate  = tzi->StandardDate;
    timezoneInformation.StandardBias  = tzi->StandardBias;
    timezoneInformation.DaylightDate  = tzi->DaylightDate;
    timezoneInformation.DaylightBias  = tzi->DaylightBias;
    
    return timezoneInformation;
}

+(NSData *)_windowsDataFromRegistry:(NSString *)registryname stdName:(NSString **)stdName daylightName:(NSString **)daylightName {
    HKEY hTimeZoneKey;
    DWORD retCode;
    TCHAR  valueName[MAX_VALUE_NAME]; 
    
    NSString * regPath = [NSString stringWithFormat:@"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\%@", registryname];
    
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                     TEXT([regPath cString]),
                     0,
                     KEY_READ,
                     &hTimeZoneKey) == ERROR_SUCCESS) {
        
        @try {
            DWORD    subKeys;
            DWORD    maxSubKey;
            DWORD    cValues;
            
            RegQueryInfoKey(
                            hTimeZoneKey,
                            NULL,
                            NULL,
                            NULL,
                            &subKeys,
                            &maxSubKey,
                            NULL,
                            &cValues, 
                            NULL,
                            NULL,
                            NULL,
                            NULL);
            
            if (cValues) {
                char stdname[200] = {0};
                char daylightname[200] = {0};
                BYTE tzdata[44] = {0};
                int j;
                BOOL dataset = NO;
                BOOL nameset = NO;
                BOOL daylightnameset = NO;
                
                for (j=0, retCode=ERROR_SUCCESS; j<cValues; j++) { 
                    BYTE lpData[200];
                    DWORD dataSize = 200;
                    DWORD valueLength = MAX_VALUE_NAME; 
                    valueName[0] = '\0';
                    
                    
                    retCode = RegEnumValue(hTimeZoneKey, j, 
                                           valueName, 
                                           &valueLength, 
                                           NULL, 
                                           NULL,
                                           lpData,
                                           &dataSize);
                    if (retCode == ERROR_SUCCESS )  { 
                        if (strcmp(valueName,"Std") == 0) {
                            strcpy(stdname, lpData);
                            nameset = YES;
                        }
                        else if (strcmp(valueName,"Dlt") == 0) {
                            strcpy(daylightname, lpData);
                            daylightnameset = YES;
                        }
                        else if (strcmp(valueName,"TZI") == 0) {
                            if (dataSize == 44) {
                                memcpy(tzdata, lpData, dataSize);
                                dataset = YES;
                            }
                        }
                        
                        if (dataset  == YES && nameset == YES && daylightnameset == YES) {
                            
                            if (stdName != NULL) {
                                *stdName = [NSString stringWithCString:stdname];
                            }
                            if (daylightName != NULL) {
                                *daylightName = [NSString stringWithCString:daylightname];
                            }
                            RegCloseKey(hTimeZoneKey);
                            
                            return [NSData dataWithBytes:tzdata length:44];
                        }
                    }
                }
                
            }   
            
            RegCloseKey(hTimeZoneKey);
        }
        @catch(NSException  *exception) {
            RegCloseKey(hTimeZoneKey);
            //[exception raise];
            return nil;
        }
    }
    
    return nil;
}

+ (NSData *)_getDataWithWindowsName:(NSString *)name registryName:(NSString **)registryName {
    HKEY hTimeZonesKey;
    
    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                     TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"),
                     0,
                     KEY_READ,
                     &hTimeZonesKey) == ERROR_SUCCESS) {
        @try {
            TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
            DWORD    cbName;                   // size of name string 
            DWORD    cSubKeys=0;               // number of subkeys 
            
            DWORD i; 
            
            TCHAR  achValue[MAX_VALUE_NAME]; 
            DWORD cchValue = MAX_VALUE_NAME; 
            
            RegQueryInfoKey(
                            hTimeZonesKey,
                            NULL,
                            NULL,
                            NULL,
                            &cSubKeys,
                            NULL,
                            NULL, 
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
            
            if (cSubKeys) {
                for (i=0; i<cSubKeys; i++) { 
                    
                    NSData *data;
                    cbName = MAX_KEY_LENGTH;
                    HKEY hTimeZoneKey;
                    if ( ERROR_SUCCESS == RegEnumKeyEx(hTimeZonesKey, i,
                                                       achKey, 
                                                       &cbName, 
                                                       NULL, 
                                                       NULL, 
                                                       NULL, 
                                                       NULL)) {
                        
                        
                        NSString *daylightName;
                        NSString *normname;
                        
                        if ((data = [NSTimeZone_win32 _windowsDataFromRegistry:[NSString stringWithCString:achKey] stdName:&normname daylightName:&daylightName]) != nil) {
                            if([name isEqualToString:[NSString stringWithCString:achKey]] ||[name isEqualToString:normname]) {
                                if (registryName != NULL) {
                                    *registryName = [NSString stringWithCString:achKey];

                                }
                                return data;
                            }
                        }
                    }
                    
                }
            }             
            RegCloseKey(hTimeZonesKey);
        }
        @catch(NSException  *exception) {
            RegCloseKey(hTimeZonesKey);
            //[exception raise];
            return nil;
        }
    }
    else {
        //[NSException raise:NSInvalidArgumentException format:@"No time zone found in registry"];
        return nil;
    }
    
    return nil;
}

+(NSDate *)_systemtimeToDate:(SYSTEMTIME)systemtime {
    FILETIME filetime;
    
    SystemTimeToFileTime(&systemtime, &filetime);
    
    return [NSDate dateWithTimeIntervalSinceReferenceDate:Win32TimeIntervalFromFileTime(filetime)];
}

+(SYSTEMTIME)_dateToSystemTime:(NSDate *)date {
    SYSTEMTIME systemtime;
    NSTimeInterval interval = [date timeIntervalSinceReferenceDate];
    
    systemtime.wYear = NSYearFromTimeInterval(interval);
    systemtime.wMonth = NSMonthFromTimeInterval(interval);
    systemtime.wDayOfWeek = NSWeekdayFromTimeInterval(interval)-1;
    systemtime.wDay = NSDayOfMonthFromTimeInterval(interval);
    systemtime.wHour = NS24HourFromTimeInterval(interval);
    systemtime.wMinute = NSMinuteFromTimeInterval(interval);
    systemtime.wSecond = NSSecondFromTimeInterval(interval);
    systemtime.wMilliseconds = NSMillisecondsFromTimeInterval(interval);
    
    return systemtime;
}

+(NSDictionary *)_windowsZones {
    static NSDictionary *windowsZonesDictionary = nil;
    
    if (windowsZonesDictionary == nil) {
        NSString *pathToPlist = [[NSBundle bundleForClass:self] pathForResource:@"NSTimeZoneWindowsZones"
                                                                         ofType:@"plist"];
        windowsZonesDictionary = [[NSDictionary allocWithZone:NULL] initWithContentsOfFile:pathToPlist];
    }
    
    return windowsZonesDictionary;
}

+(NSString *)_registryNameToAbbreviation:(NSString *)registryName {
    
    NSArray         *components;
    NSMutableString *result;

    if([registryName isEqualToString:@"UTC"]) {
        return registryName;
    }
    
    components= [registryName componentsSeparatedByString:@" "];
    result = [NSMutableString string];
    
    for (NSString *element in components) {
        [result appendString:[element substringToIndex:1]];
    }
    return result;
}

@end
#endif
