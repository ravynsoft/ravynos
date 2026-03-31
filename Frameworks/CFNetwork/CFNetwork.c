/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* Repository for initialization routine and any useful utilities that need to be added */

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFStreamPriv.h>
#include <CFNetwork/CFNetworkPriv.h>
#include "CFNetworkInternal.h"
#include "CFHTTPInternal.h"

#include <sys/stat.h>
#include <string.h>  // For strcat
#include <stdlib.h>  // for getenv
#if defined(__MACH__)
#include <mach-o/dyld.h>
#include <SystemConfiguration/SystemConfiguration.h>
#endif

#include <sys/types.h>
#if defined(__WIN32__)
#include <winsock2.h>
#include <ws2tcpip.h>	// for ipv6
#include <wininet.h>	// for InternetTimeToSystemTime
                        // WinHTTP has the same function, but it has more OS/SP constraints
#include <objbase.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif


#if defined(__MACH__)

/* extern*/ void*
__CFNetworkLoadFramework(const char* const framework_path) {

	// OS 10.3 change to NSAddImage options here:
	// a) Use NSADDIMAGE_OPTION_WITH_SEARCHING to support setting common DYLD_ environment variables
	// including DYLD_IMAGE_SUFFIX and DYLD_LIBRARY_PATH.
	// b) Use NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME to fix a nasty problem where two copies of
	// a given framework are loaded into the same address space (See bug # 3060641).
    return ((void*)NSAddImage(framework_path, NSADDIMAGE_OPTION_WITH_SEARCHING | NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME));
}

#endif

#if defined(__WIN32__)
typedef WSAAPI int (*getnameinfo_funcPtr)(const struct sockaddr*, socklen_t, char*, DWORD, char*, DWORD, int);
WINBOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved);
#else
typedef int (*getnameinfo_funcPtr)(const struct sockaddr *sa, socklen_t salen, char *node, socklen_t nodelen, char *service, socklen_t servicelen, int flags);
#endif
    

/* extern */ UInt8*
_CFStringGetOrCreateCString(CFAllocatorRef allocator, CFStringRef string, UInt8* buffer, CFIndex* bufferLength, CFStringEncoding encoding) {
	
	CFIndex extra = 0;
	
	if (!bufferLength)
		bufferLength = &extra;
                
        assert(string);
	
	if (buffer && *bufferLength && CFStringGetCString(string, (char*)buffer, *bufferLength, encoding))
		*bufferLength = strlen((const char*)buffer);
	
	else {
		
		UInt8* saved = buffer;
        CFRange range = CFRangeMake(0, CFStringGetLength(string));
		
		CFStringGetBytes(string, range, encoding, 0, FALSE, NULL, 0, bufferLength);
		
        buffer = (UInt8*)CFAllocatorAllocate(allocator, *bufferLength + 1, 0);
		
		if (buffer) {
			
			*bufferLength = CFStringGetBytes(string, range, encoding, 0, FALSE, buffer, *bufferLength, NULL);
			
			buffer[*bufferLength] = '\0';
		}
		else {
			*bufferLength = 0;
			buffer = saved;
			if (buffer)
				buffer[*bufferLength] = '\0';
		}
    }
	
	return buffer;
}


/* extern */ CFStringRef
_CFNetworkCFStringCreateWithCFDataAddress(CFAllocatorRef alloc, CFDataRef addr) {
	
    getnameinfo_funcPtr getnameinfo_func = NULL;
	struct sockaddr* sa = (struct sockaddr*)CFDataGetBytePtr(addr);
	CFIndex salen = CFDataGetLength(addr);
	
#if defined(__WIN32__)
    // getnameinfo doesn't exist on Win2K, so we must look it up dynamically
    static CFSpinLock_t lock = 0;
    static Boolean beenHere = FALSE;
    static getnameinfo_funcPtr getnameinfo_funcCache = NULL;
    
    __CFSpinLock(&lock);
    if (!beenHere) {
        HMODULE module;
        beenHere = TRUE;
        module = GetModuleHandle("ws2_32");
        if (module != NULL) {
            getnameinfo_funcCache = (getnameinfo_funcPtr)GetProcAddress(module, "getnameinfo");
        }
    }
    __CFSpinUnlock(&lock);
    getnameinfo_func = getnameinfo_funcCache;
#else
    getnameinfo_func = &getnameinfo;
#endif
	
    CFStringRef name = NULL;
    if (getnameinfo_func) {
        char buffer[NI_MAXHOST];
        if ((getnameinfo_func)(sa, salen, buffer, sizeof(buffer), NULL, 0, NI_NUMERICHOST) == 0) {
            name = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingASCII);
        }
    } else {
        if (sa->sa_family == AF_INET) {
            char *cStr = inet_ntoa(((struct sockaddr_in*)sa)->sin_addr);
            if (cStr)
                name = CFStringCreateWithCString(NULL, cStr, kCFStringEncodingASCII);
        }
    }
    return name;
}


#if 0
/*
	-=-=- RFC-1123 -=-=-

	5.2.14  RFC-822 Date and Time Specification: RFC-822 Section 5

	The syntax for the date is hereby changed to:

		date = 1*2DIGIT month 2*4DIGIT

	All mail software SHOULD use 4-digit years in dates, to ease
	the transition to the next century.

	There is a strong trend towards the use of numeric timezone
	indicators, and implementations SHOULD use numeric timezones
	instead of timezone names.  However, all implementations MUST
	accept either notation.  If timezone names are used, they MUST
	be exactly as defined in RFC-822.

	The military time zones are specified incorrectly in RFC-822:
	they count the wrong way from UT (the signs are reversed).  As
	a result, military time zones in RFC-822 headers carry no
	information.

	Finally, note that there is a typo in the definition of "zone"
	in the syntax summary of appendix D; the correct definition
	occurs in Section 3 of RFC-822.

 
	-=-=- RFC-822 -=-=-

	5.  DATE AND TIME SPECIFICATION

	5.1.  SYNTAX

	date-time   =  [ day "," ] date time        ; dd mm yy
												;  hh:mm:ss zzz

	day         =  "Mon"  / "Tue" /  "Wed"  / "Thu"
				/  "Fri"  / "Sat" /  "Sun"

	date        =  1*2DIGIT month 2DIGIT        ; day month year
												;  e.g. 20 Jun 82

	month       =  "Jan"  /  "Feb" /  "Mar"  /  "Apr"
				/  "May"  /  "Jun" /  "Jul"  /  "Aug"
				/  "Sep"  /  "Oct" /  "Nov"  /  "Dec"

	time        =  hour zone                    ; ANSI and Military

	hour        =  2DIGIT ":" 2DIGIT [":" 2DIGIT]
												; 00:00:00 - 23:59:59

	zone		=  "UT"  / "GMT"				; Universal Time
												; North American : UT
				/  "EST" / "EDT"                ;  Eastern:  - 5/ - 4
				/  "CST" / "CDT"                ;  Central:  - 6/ - 5
				/  "MST" / "MDT"                ;  Mountain: - 7/ - 6
				/  "PST" / "PDT"                ;  Pacific:  - 8/ - 7
				/  1ALPHA                       ; Military: Z = UT;
												;  A:-1; (J not used)
												;  M:-12; N:+1; Y:+12
				/ ( ("+" / "-") 4DIGIT )        ; Local differential
												;  hours+min. (HHMM)

	5.2.  SEMANTICS

	If included, day-of-week must be the day implied by the date
	specification.

	Time zone may be indicated in several ways.  "UT" is Univer-
	sal  Time  (formerly called "Greenwich Mean Time"); "GMT" is per-
	mitted as a reference to Universal Time.  The  military  standard
	uses  a  single  character for each zone.  "Z" is Universal Time.
	"A" indicates one hour earlier, and "M" indicates 12  hours  ear-
	lier;  "N"  is  one  hour  later, and "Y" is 12 hours later.  The
	letter "J" is not used.  The other remaining two forms are  taken
	from ANSI standard X3.51-1975.  One allows explicit indication of
	the amount of offset from UT; the other uses  common  3-character
	strings for indicating time zones in North America.

 
	-=-=- RFC-2616 -=-=-

	3.3.1 Full Date

	HTTP applications have historically allowed three different formats
	for the representation of date/time stamps:

		Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
		Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
		Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

	The first format is preferred as an Internet standard and represents
	a fixed-length subset of that defined by RFC 1123 [8] (an update to
	RFC 822 [9]). The second format is in common use, but is based on the
	obsolete RFC 850 [12] date format and lacks a four-digit year.
	HTTP/1.1 clients and servers that parse the date value MUST accept
	all three formats (for compatibility with HTTP/1.0), though they MUST
	only generate the RFC 1123 format for representing HTTP-date values
	in header fields. See section 19.3 for further information.

		Note: Recipients of date values are encouraged to be robust in
		accepting date values that may have been sent by non-HTTP
		applications, as is sometimes the case when retrieving or posting
		messages via proxies/gateways to SMTP or NNTP.

	All HTTP date/time stamps MUST be represented in Greenwich Mean Time
	(GMT), without exception. For the purposes of HTTP, GMT is exactly
	equal to UTC (Coordinated Universal Time). This is indicated in the
	first two formats by the inclusion of "GMT" as the three-letter
	abbreviation for time zone, and MUST be assumed when reading the
	asctime format. HTTP-date is case sensitive and MUST NOT include
	additional LWS beyond that specifically included as SP in the
	grammar.

		HTTP-date		= rfc1123-date | rfc850-date | asctime-date
		rfc1123-date	= wkday "," SP date1 SP time SP "GMT"
		rfc850-date		= weekday "," SP date2 SP time SP "GMT"
		asctime-date	= wkday SP date3 SP time SP 4DIGIT
		date1			= 2DIGIT SP month SP 4DIGIT
							; day month year (e.g., 02 Jun 1982)
		date2			= 2DIGIT "-" month "-" 2DIGIT
							; day-month-year (e.g., 02-Jun-82)
		date3			= month SP ( 2DIGIT | ( SP 1DIGIT ))
							; month day (e.g., Jun  2)
		time			= 2DIGIT ":" 2DIGIT ":" 2DIGIT
							; 00:00:00 - 23:59:59
		wkday			= "Mon" | "Tue" | "Wed"
						| "Thu" | "Fri" | "Sat" | "Sun"
		weekday			= "Monday" | "Tuesday" | "Wednesday"
						| "Thursday" | "Friday" | "Saturday" | "Sunday"
		month			= "Jan" | "Feb" | "Mar" | "Apr"
						| "May" | "Jun" | "Jul" | "Aug"
						| "Sep" | "Oct" | "Nov" | "Dec"

		Note: HTTP requirements for the date/time stamp format apply only
		to their usage within the protocol stream. Clients and servers are
		not required to use these formats for user presentation, request
		logging, etc.
*/
#endif

/* Arrays of asctime-date day and month strs, rfc1123-date day and month strs, and rfc850-date day and month strs. */
static const char* kDayStrs[] = {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday",
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

static const char* kMonthStrs[] = {
	"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December",
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/* NOTE that these are ordered this way on purpose. */
static const char* kUSTimeZones[] = {"PST", "PDT", "MST", "MDT", "CST", "CDT", "EST", "EDT"};


/* extern */ const UInt8*
_CFGregorianDateCreateWithBytes(CFAllocatorRef alloc, const UInt8* bytes, CFIndex length, CFGregorianDate* date, CFTimeZoneRef* tz) {

	UInt8 buffer[256];					/* Any dates longer than this are not understood. */

	length = (length == 256) ? 255 : length;
	memmove(buffer, bytes, length);
	buffer[length] = '\0';				/* Guarantees every compare will fail if trying to index off the end. */
	
	memset(date, 0, sizeof(date[0]));
	if (tz) *tz = NULL;
	
	do {
		int i;
		CFIndex scan = 0;
		UInt8 c = buffer[scan];
			
		/* Skip leading whitespace */
		while (isspace(c))
			c = buffer[++scan];
		
		/* Check to see if there is a weekday up front. */
		if (!isdigit(c)) {
			
			for (i = 0; i < (sizeof(kDayStrs) / sizeof(kDayStrs[0])); i++) {
				if (!memcmp(kDayStrs[i], &buffer[scan], strlen(kDayStrs[i])))
					break;
			}

			if (i >=(sizeof(kDayStrs) / sizeof(kDayStrs[0])))
				break;
			
			scan += strlen(kDayStrs[i]);
			c = buffer[scan];
			
			while (isspace(c) || c == ',')
				c = buffer[++scan];
		}
		
		/* check for asctime where month comes first */
		if (!isdigit(c)) {
			
			for (i = 0; i < (sizeof(kMonthStrs) / sizeof(kMonthStrs[0])); i++) {
				if (!memcmp(kMonthStrs[i], &buffer[scan], strlen(kMonthStrs[i])))
					break;
			}
			
			if (i >= (sizeof(kMonthStrs) / sizeof(kMonthStrs[0])))
				break;
			
			date->month = (i % 12) + 1;
			
			scan += strlen(kMonthStrs[i]);
			c = buffer[scan];
			
			while (isspace(c))
				c = buffer[++scan];
			
			if (!isdigit(c))
				break;
		}
		
		/* Read the day of month */
		for (i = 0; isdigit(c) && (i < 2); i++) {
			date->day *= 10;
			date->day += c - '0';
			c = buffer[++scan];
		}		
		
		while (isspace(c) || c == '-')
			c = buffer[++scan];
		
		/* Not asctime so now comes the month. */
		if (date->month == 0) {
			
			if (isdigit(c)) {
				for (i = 0; isdigit(c) && (i < 2); i++) {
					date->month *= 10;
					date->month += c - '0';
					c = buffer[++scan];
				}		
			}
			else {
				for (i = 0; i < (sizeof(kMonthStrs) / sizeof(kMonthStrs[0])); i++) {
					if (!memcmp(kMonthStrs[i], &buffer[scan], strlen(kMonthStrs[i])))
						break;
				}
				
				if (i >= (sizeof(kMonthStrs) / sizeof(kMonthStrs[0])))
					break;
				
				date->month = (i % 12) + 1;
				
				scan += strlen(kMonthStrs[i]);
				c = buffer[scan];
			}
			
			while (isspace(c) || c == '-')
				c = buffer[++scan];
			
			/* Read the year */
			for (i = 0; isdigit(c) && (i < 4); i++) {
				date->year *= 10;
				date->year += c - '0';
				c = buffer[++scan];
			}
			
			while (isspace(c))
				c = buffer[++scan];
		}
		
		/* Read the hours */
		for (i = 0; isdigit(c) && (i < 2); i++) {
			date->hour *= 10;
			date->hour += c - '0';
			c = buffer[++scan];
		}		
		
		if (c != ':')
			break;
		c = buffer[++scan];
		
		/* Read the minutes */
		for (i = 0; isdigit(c) && (i < 2); i++) {
			date->minute *= 10;
			date->minute += c - '0';
			c = buffer[++scan];
		}		
		
		if (c == ':') {
			
			c = buffer[++scan];
			
			/* Read the seconds */
			for (i = 0; isdigit(c) && (i < 2); i++) {
				date->second *= 10;
				date->second += c - '0';
				c = buffer[++scan];
			}		
			c = buffer[++scan];
		}
		
		/* If haven't read the year yet, now is the time. */
		if (date->year == 0) {
			
			while (isspace(c))
				c = buffer[++scan];
			
			/* Read the year */
			for (i = 0; isdigit(c) && (i < 4); i++) {
				date->year *= 10;
				date->year += c - '0';
				c = buffer[++scan];
			}
		}
		
		if (date->year && date->year < 100) {
			
			if (date->year < 70)
				date->year += 2000;		/* My CC is still using 2-digit years! */
			else
				date->year += 1900;		/* Bad 2 byte clients */
		}
		
		while (isspace(c))
			c = buffer[++scan];

		if (c && tz) {
			
			/* If it has absolute offset, read the hours and minutes. */
			if ((c == '+') || (c == '-')) {
				
				char sign = c;
				CFTimeInterval minutes = 0, offset = 0;
				
				c = buffer[++scan];
				
				/* Read the hours */
				for (i = 0; isdigit(c) && (i < 2); i++) {
					offset *= 10;
					offset += c - '0';
					c = buffer[++scan];
				}
				
				/* Read the minutes */
				for (i = 0; isdigit(c) && (i < 2); i++) {
					minutes *= 10;
					minutes += c - '0';
					c = buffer[++scan];
				}
				
				offset *= 60;
				offset += minutes;

				if (sign == '-') offset *= -60;
				else offset *= 60;
				
				*tz = CFTimeZoneCreateWithTimeIntervalFromGMT(alloc, offset);
			}
			
			/* If it's not GMT/UT time, need to parse the alpha offset. */
			else if (!strncmp((const char*)(&buffer[scan]), "UT", 2)) {
				*tz = CFTimeZoneCreateWithTimeIntervalFromGMT(alloc, 0);
				scan += 2;
			}
				
			else if (!strncmp((const char*)(&buffer[scan]), "GMT", 3)) {
				*tz = CFTimeZoneCreateWithTimeIntervalFromGMT(alloc, 0);
				scan += 3;
			}
			
			else if (isalpha(c)) {
				
				UInt8 next = buffer[scan + 1];
				
				/* Check for military time. */
				if ((c != 'J') && (!next || isspace(next) || (next == '*'))) {
					
					if (c == 'Z')
						*tz = CFTimeZoneCreateWithTimeIntervalFromGMT(alloc, 0);
					
					else {

						CFTimeInterval offset = (c < 'N') ? (c - 'A' + 1) : ('M' - c);
					
						offset *= 60;
						
						if (next == '*') {
							scan++;
							offset = (offset < 0) ? offset - 30 : offset + 30;
						}
						
						offset *= 60;
						
						*tz = CFTimeZoneCreateWithTimeIntervalFromGMT(alloc, 0);
					}
				}
					
				else {
					
					for (i = 0; i < (sizeof(kUSTimeZones) / sizeof(kUSTimeZones[0])); i++) {
						
						if (!memcmp(kUSTimeZones[i], &buffer[scan], strlen(kUSTimeZones[i]))) {
							
							*tz = CFTimeZoneCreateWithTimeIntervalFromGMT(alloc, (-8 + (i >> 2) + (i & 0x1)) * 3600);
							
							scan += strlen(kUSTimeZones[i]);
							
							break;
						}
					}
				}
			}				
		}
		
		if (!CFGregorianDateIsValid(*date, kCFGregorianAllUnits))
			break;
		
		return bytes + scan;
			
	} while (1);
	
	memset(date, 0, sizeof(date[0]));
	if (tz) {
		if (*tz) CFRelease(*tz);
		*tz = NULL;
	}
	
	return bytes;
}

/* extern */ CFIndex
_CFGregorianDateCreateWithString(CFAllocatorRef alloc, CFStringRef str, CFGregorianDate* date, CFTimeZoneRef* tz) {
	
	UInt8 buffer[256];					/* Any dates longer than this are not understood. */
	CFIndex length = CFStringGetLength(str);
	CFIndex result = 0;
	
	CFStringGetBytes(str, CFRangeMake(0, length), kCFStringEncodingASCII, 0, FALSE, buffer, sizeof(buffer), &length);
	
	if (length)
		result = _CFGregorianDateCreateWithBytes(alloc, buffer, length, date, tz) - buffer;
	
	else {
		memset(date, 0, sizeof(date[0]));
		if (tz) *tz = NULL;
	}
	
	return result;
}


/* extern */ CFStringRef
_CFStringCreateRFC1123DateStringWithGregorianDate(CFAllocatorRef alloc, CFGregorianDate* date, CFTimeZoneRef tz) {
	
	CFStringRef result = NULL;
	int hour = 0;
	int minute = 0;
	
	if (tz) {
		CFTimeInterval offset = CFTimeZoneGetSecondsFromGMT(tz, 0.0);
		hour = offset / 3600;
		minute = abs(offset - (hour * 3600));
	}
	
	if (CFGregorianDateIsValid(*date, kCFGregorianAllUnits)) {
		
		result = CFStringCreateWithFormat(alloc,
										  NULL,
										  CFSTR("%02d %s %04ld %02d:%02d:%02d %+03d%02d"),
										  date->day,
										  kMonthStrs[date->month + 11],		/* Offset to the short names */
										  date->year,
										  date->hour,
										  date->minute,
										  (int)date->second,
										  hour,
										  minute);
	}
	
	return result;
}


/* extern */ CFStringRef
_CFStringCreateRFC2616DateStringWithGregorianDate(CFAllocatorRef alloc, CFGregorianDate* date, CFTimeZoneRef tz) {
	
	CFStringRef result = NULL;	
	
	if (CFGregorianDateIsValid(*date, kCFGregorianAllUnits)) {

		CFAbsoluteTime t = CFGregorianDateGetAbsoluteTime(*date, tz);
		SInt32 day = CFAbsoluteTimeGetDayOfWeek(t, NULL);
		
		result = CFStringCreateWithFormat(alloc,
										  NULL,
										  CFSTR("%s, %02d %s %04ld %02d:%02d:%02d GMT"),
										  kDayStrs[6 + day],
										  date->day,
										  kMonthStrs[date->month + 11],		/* Offset to the short names */
										  date->year,
										  date->hour,
										  date->minute,
										  (int)date->second);
	}
	
	return result;
}


#if defined(__WIN32__)

extern void _CFFTPCleanup(void);			/* exported from FTPStream.c */

WINBOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved) {
    if (dwReason == DLL_PROCESS_DETACH) {
        _CFHTTPMessageCleanup();
        _CFHTTPStreamCleanup();
        _CFFTPCleanup();
    }
    return TRUE;
}

#endif // __WIN32__
