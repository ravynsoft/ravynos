/*
 * Airyx LaunchServices - unified types functions
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/Foundation.h>
#import "UTTypes.h"
#include <sqlite3.h>

extern NSString *LS_DATABASE;

Boolean UTTypeEqual(CFStringRef inUTI1, CFStringRef inUTI2)
{
}

Boolean UTTypeConformsTo(CFStringRef inUTI1, CFStringRef inUTI2)
{
}

// FIXME: `inConformingToUTI` is currently ignored
COREFOUNDATION_EXPORT CFStringRef UTTypeCreatePreferredIdentifierForTag(CFStringRef inTagClass,
	CFStringRef inTag, CFStringRef inConformingToUTI)
{
    sqlite3 *pDB = 0;
    if(sqlite3_open([LS_DATABASE UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return CFSTR(""); // FIXME: log error somewhere
    }

    char *column = "pboards";
    if(CFStringCompare(inTagClass, kUTTagClassFilenameExtension, 0) == NSOrderedSame)
    	column = "extensions";
    else if(CFStringCompare(inTagClass, kUTTagClassMIMEType, 0) == NSOrderedSame)
        column = "mimetypes";
    else if(CFStringCompare(inTagClass, kUTTagClassOSType, 0) == NSOrderedSame)
        column = "ostypes";
    
    NSMutableString *qtemplate = [NSMutableString stringWithCString:"SELECT * FROM types WHERE _column_ LIKE ? || ',%' OR _column_ LIKE '%,' || ? OR _column_ LIKE '%,' || ? || ',%' OR _column_ = ?"];
    [qtemplate replaceOccurrencesOfString:@"_column_" withString:[NSString stringWithCString:column] options:NSLiteralSearch range:NSMakeRange(0,[qtemplate length])];
    const char *query = [qtemplate UTF8String];
    const int length = [qtemplate length];
    sqlite3_stmt *stmt;
    const char *tail;

    if(sqlite3_prepare_v2(pDB, query, length, &stmt, &tail) != SQLITE_OK) {
        sqlite3_close(pDB);
        return CFSTR("");
    }

    const char *tag = CFStringGetCStringPtr(inTag, kCFStringEncodingUTF8);
    int taglen = CFStringGetLength(inTag);

    if(sqlite3_bind_text(stmt, 1, tag, taglen, SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 2, tag, taglen, SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 3, tag, taglen, SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 4, tag, taglen, SQLITE_STATIC) != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(pDB);
        return CFSTR("");
    }

    int rc = sqlite3_step(stmt);
    NSString *uti = nil;
    if(rc == SQLITE_ROW)
    	uti = [NSString stringWithCString:sqlite3_column_text(stmt, 0)];

    sqlite3_finalize(stmt);
    sqlite3_close(pDB);
    return (CFStringRef)uti;
}

CFArrayRef UTTypeCreateAllIdentifiersForTag(CFStringRef inTagClass,
	CFStringRef inUTI, CFStringRef inConformingToUTI)
{
}

CFStringRef UTTypeCopyPreferredTagWithClass(CFStringRef inUTI,
	CFStringRef inTagClass)
{
}

// Not implemented yet
//CFStringRef UTCreateStringForOSType(OSType inOSType);
//OSType UTGetOSTypeFromString(CFStringRef inTag);

//CFDictionaryRef UTTypeCopyDeclaration(CFStringRef inUTI);
//CFURLRef UTTypeCopyDeclaringBundleURL(CFStringRef inUTI);
//CFStringRef UTTypeCopyDescription(CFStringRef inUTI);

