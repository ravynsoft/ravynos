/* Copyright (c) 2009 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifdef WINDOWS
#import <CoreFoundation/CFString.h>

#import <Foundation/NSString_defaultEncoding.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRaiseException.h>
#include <windows.h>

NSStringEncoding defaultEncoding()
{
    //don't use objc calls because they call often defaultCStringEncoding
    UINT codepage = GetACP();

    CFStringEncoding encoding = CFStringConvertWindowsCodepageToEncoding(codepage);
    if (encoding != kCFStringEncodingInvalidId) {
        return CFStringConvertEncodingToNSStringEncoding(encoding);
    }
	switch(codepage)
	{
		case 1250:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
            return NSWindowsCP1250StringEncoding;
			
		case 1251:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
            return NSWindowsCP1251StringEncoding;
            
		case 1252:
            return NSWindowsCP1252StringEncoding;	
            
		case 1253:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
            return NSWindowsCP1253StringEncoding;
            
		case 1254:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
            return NSWindowsCP1254StringEncoding;		
            
		case 50220:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
            return NSISO2022JPStringEncoding;
            
		case 10000:
			return NSMacOSRomanStringEncoding;
            
		case 12000:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
			return NSUTF32LittleEndianStringEncoding;
            
		case 12001:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
			return NSUTF32BigEndianStringEncoding;
            
		case 20127:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
			return NSASCIIStringEncoding;
            
		case 20932:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
			return NSJapaneseEUCStringEncoding;
            
		case 65001:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
			return NSUTF8StringEncoding;
            
		case 28591:
			return NSISOLatin1StringEncoding;
            
		case 28592:
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
			return NSISOLatin2StringEncoding;
			
		default: {
            static BOOL codePageErrorLogged = NO;
            if (codePageErrorLogged == NO) {
                codePageErrorLogged = YES;
                NSCLog("Unknown codepage=%d",codepage);
            }
        }
// FIXME: use until the right encoding is implemented
            return NSWindowsCP1252StringEncoding;
	}
    
}

#endif

