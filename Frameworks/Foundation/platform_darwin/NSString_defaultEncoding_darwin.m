/* Copyright (c) 2009 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


#import <Foundation/NSString_defaultEncoding.h>
#import <Foundation/NSException.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

NSStringEncoding defaultEncoding()
{
    //don't use objc calls because they call often defaultCStringEncoding 

    static int defaultEncoding = -1;
    
	if (defaultEncoding == -1) {
        static struct passwd    *pwent = NULL;
        char                    filename[255] = {0};
        FILE                    *fhandle;
        if (pwent == NULL) {
            pwent = getpwuid(getuid());
        }
        strcpy(filename, pwent->pw_dir);
        strcat(filename, "/.CFUserTextEncoding");

		fhandle = fopen(filename, "r");
		if(fhandle != NULL)
        {
            int enc;
            fscanf (fhandle,"%X",&enc);
            fclose(fhandle);
            
			switch(enc) {
				case 0:
					defaultEncoding = NSMacOSRomanStringEncoding;
					break;
				case 0x0500:
					defaultEncoding = NSWindowsCP1252StringEncoding;
					break;
				case 0x0201:
					defaultEncoding = NSISOLatin1StringEncoding;
					break;
				case 0x0202:
					defaultEncoding = NSISOLatin2StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					break;
				case 0x0B01:
					defaultEncoding = NSNEXTSTEPStringEncoding;
					break;
				case 0x0600:
					defaultEncoding = NSASCIIStringEncoding;
					break;
				case 0x0100:
					defaultEncoding = NSUnicodeStringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					break;
				case 0x08000100:
					defaultEncoding = NSUTF8StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					break;	
				case 0x0BFF:
					defaultEncoding = NSNonLossyASCIIStringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					break;	
				case 0x0A01:	
					defaultEncoding = NSShiftJISStringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					break;
				case 0x0920:
					defaultEncoding = NSJapaneseEUCStringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					break;					
				default:
					defaultEncoding = NSMacOSRomanStringEncoding;					
                   break;
            }
		
        }
        if(defaultEncoding == -1) {
			defaultEncoding = NSMacOSRomanStringEncoding;
		}
	}
	
	return defaultEncoding;		    
}
