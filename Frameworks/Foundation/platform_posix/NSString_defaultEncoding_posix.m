/* Copyright (c) 2009 Glenn Ganz

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX

#import <Foundation/NSString_defaultEncoding.h>
#import <Foundation/NSException.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

NSStringEncoding defaultEncoding()
{
    //don't use objc calls because they call often defaultCStringEncoding

    static int defaultEncoding = -1;

	if(defaultEncoding == -1) {

        char *lang = getenv("LANG");
		if (lang && *lang) {
			lang = strdup(lang); // don't F up the environment!
			const char* firstpart = strtok (lang, ".");
			if (firstpart != NULL) {
				char* secondpart = strtok (NULL, "\0");
				if (secondpart != NULL) {
					int i;

					//make all upper
					for(i = 0; i < strlen(secondpart);i++)
					{
						secondpart[i] = toupper(secondpart[i]);
					}
					if(strcmp(secondpart, "UTF-8") == 0 || strcmp(secondpart, "UTF8") == 0)
					{
						defaultEncoding = NSUTF8StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					}
					else if(strcmp(secondpart, "ISO8859-1") == 0 || strcmp(secondpart, "ISO88591") == 0)
					{
						defaultEncoding = NSISOLatin1StringEncoding;
					}
					else if(strcmp(secondpart, "ISO8859-2") == 0 || strcmp(secondpart, "ISO88592") == 0)
					{
						defaultEncoding = NSISOLatin2StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					}
					else if(strcmp(secondpart, "EUCJP") == 0)
					{
						defaultEncoding = NSJapaneseEUCStringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					}
                    else if(strcmp(secondpart, "CP1250") == 0)
					{
						defaultEncoding = NSWindowsCP1250StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					}
					else if(strcmp(secondpart, "CP1251") == 0)
					{
						defaultEncoding = NSWindowsCP1251StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					}
                    else if(strcmp(secondpart, "CP1252") == 0)
					{
						defaultEncoding = NSWindowsCP1252StringEncoding;
					}
                    else if(strcmp(secondpart, "CP1253") == 0)
					{
						defaultEncoding = NSWindowsCP1253StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					}
                    else if(strcmp(secondpart, "CP1254") == 0)
					{
						defaultEncoding = NSWindowsCP1254StringEncoding;
// FIXME: use until the right encoding is implemented
defaultEncoding = NSISOLatin1StringEncoding;
					}

                    //TODO: add more encodings
				}
			}
			free(lang);
		}

		if(defaultEncoding == -1)
		{
			//set the default to ASCII
			defaultEncoding = NSISOLatin1StringEncoding; // FIXME: should be utf8?
		}
	}

	return defaultEncoding;

}
#endif
