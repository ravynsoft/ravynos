/*
 * Airyx LaunchServices - unified types functions
 *
 * Copyright (C) 2021-2022 Zoe Knox <zoe@pixin.net>
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
#import "LaunchServices.h"
#import "UTTypes.h"
#import "UTTypes-private.h"
#include <sqlite3.h>

Boolean UTTypeEqual(CFStringRef inUTI1, CFStringRef inUTI2)
{
    // Dynamic Type. Equal if all tags in UTI1 exist in UTI2
    if(CFStringHasPrefix(inUTI1, CFSTR("dyn."))) {
	CFArrayRef allTags1 = CFArrayCreateMutable(NULL, 10, NULL);
	CFArrayRef allTags2 = CFArrayCreateMutable(NULL, 10, NULL);

	CFArrayRef tags = UTTypeCopyAllTagsWithClass(inUTI1, kUTTagClassFilenameExtension);
	if(tags != (CFArrayRef)0) {
	    CFArrayAppendArray(allTags1, tags, CFRangeMake(0, CFArrayGetCount(tags)));
	    CFRelease(tags);
	}
	tags = UTTypeCopyAllTagsWithClass(inUTI1, kUTTagClassMIMEType);
	if(tags != (CFArrayRef)0) {
	    CFArrayAppendArray(allTags1, tags, CFRangeMake(0, CFArrayGetCount(tags)));
	    CFRelease(tags);
	}

	tags = UTTypeCopyAllTagsWithClass(inUTI2, kUTTagClassFilenameExtension);
	if(tags != (CFArrayRef)0) {
	    CFArrayAppendArray(allTags2, tags, CFRangeMake(0, CFArrayGetCount(tags)));
	    CFRelease(tags);
	}
	tags = UTTypeCopyAllTagsWithClass(inUTI2, kUTTagClassMIMEType);
	if(tags != (CFArrayRef)0) {
	    CFArrayAppendArray(allTags2, tags, CFRangeMake(0, CFArrayGetCount(tags)));
	    CFRelease(tags);
	}

	if(CFArrayGetCount(allTags1) > CFArrayGetCount(allTags2)) {
	    CFRelease(allTags1);
	    CFRelease(allTags2);
	    return false; // more types in dynamic UTI means it is not equal!
	}

	for(int x = 0; x < CFArrayGetCount(allTags1); ++x) {
	    CFStringRef dyntag = (CFStringRef)CFArrayGetValueAtIndex(allTags1, 0);
	    if(!CFArrayContainsValue(allTags2, CFRangeMake(0, CFArrayGetCount(allTags2)), dyntag)) {
		CFRelease(allTags1);
		CFRelease(allTags2);
		return false; // dynamic UTI contains tag not in UTI2
	    }
	}
	return true;
    }

    // Defined Type. Equal if strings are identical.
    return (CFStringCompare(inUTI1, inUTI2, 0) == NSOrderedSame);
}

CFArrayRef UTTypeCopyConformsTo(CFStringRef inUTI)
{
    sqlite3 *pDB = 0;
    if(sqlite3_open([[LaunchServices database] UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return (CFArrayRef)0; // FIXME: log error somewhere
    }

    const char *query = "SELECT conforms FROM types WHERE uti = ?";
    sqlite3_stmt *stmt;
    const char *tail;
    if(sqlite3_prepare_v2(pDB, query, strlen(query), &stmt, &tail) != SQLITE_OK) {
        sqlite3_close(pDB);
        return (CFArrayRef)0;
    }

    const char *uti = CFStringGetCStringPtr(inUTI, kCFStringEncodingUTF8);
    int len = CFStringGetLength(inUTI);

    if(sqlite3_bind_text(stmt, 1, uti, len, SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(pDB);
        return (CFArrayRef)0;
    }

    int rc = sqlite3_step(stmt);
    CFArrayRef result = CFArrayCreateMutable(NULL, 5, NULL);
    while(rc == SQLITE_ROW) {
        CFArrayRef conforms = (__bridge_retained CFArrayRef)[[NSString
	    stringWithCString:(const char *)sqlite3_column_text(stmt, 0)]
	    componentsSeparatedByString:@","];
	CFArrayAppendArray(result, conforms, CFRangeMake(0, CFArrayGetCount(conforms)));
	rc = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(pDB);
    CFRetain(result);
    return result;
}

Boolean UTTypeConformsTo(CFStringRef inUTI1, CFStringRef inUTI2)
{
    CFArrayRef conformsUTI1 = UTTypeCopyConformsTo(inUTI1);

    if(conformsUTI1 == (CFArrayRef)0)
	return false;

    for(int x = 0; x < CFArrayGetCount(conformsUTI1); ++x) {
	CFStringRef uti = (CFStringRef)CFArrayGetValueAtIndex(conformsUTI1, x);
	if(CFStringCompare(uti, inUTI2, 0) == NSOrderedSame) {
	    CFRelease(conformsUTI1);
	    return true;
	}
    }

    // if UTI2 is not a direct ancestor of UTI1, we need to traverse the ancestors
    // of UTI1 until we either find UTI2 or exhaust the list
    CFArrayRef transits = CFArrayCreateMutable(NULL, 10, NULL);
    CFArrayAppendArray(transits, conformsUTI1, CFRangeMake(0, CFArrayGetCount(conformsUTI1)));
    CFRelease(conformsUTI1);

    int elements = CFArrayGetCount(transits);
    for(int x = 0; x < elements; ++x) {
	CFStringRef xtype = (CFStringRef)CFArrayGetValueAtIndex(transits, x);
	conformsUTI1 = UTTypeCopyConformsTo(xtype);
	if(conformsUTI1 != (CFArrayRef)0) {
	    CFArrayAppendArray(transits, conformsUTI1, CFRangeMake(0, CFArrayGetCount(conformsUTI1)));
	    CFRelease(conformsUTI1);
	    elements = CFArrayGetCount(transits);
	}
	if(UTTypeEqual(xtype, inUTI2)) {
	    CFRelease(transits);
	    return true;
	}
    }

    return false;
}

CFStringRef UTTypeCreatePreferredIdentifierForTag(CFStringRef inTagClass,
	CFStringRef inTag, CFStringRef inConformingToUTI)
{
    CFArrayRef result = UTTypeCreateAllIdentifiersForTag(inTagClass, inTag, inConformingToUTI);
    if(result == (CFArrayRef)0 || CFArrayGetCount(result) == 0) {
	// FIXME: create a dynamic type
	return (CFStringRef)0;
    }

    if(inConformingToUTI == NULL) {
	CFStringRef uti = CFStringCreateCopy(NULL, (CFStringRef)CFArrayGetValueAtIndex(result, 0));
	CFRelease(result);
	return uti;
    }

    for(int x = 0; x < CFArrayGetCount(result); ++x) {
	CFStringRef uti = CFStringCreateCopy(NULL, (CFStringRef)CFArrayGetValueAtIndex(result, x));
	if(UTTypeConformsTo(uti, inConformingToUTI)) {
	    CFRelease(result);
	    return uti;
	}
	CFRelease(uti);
    }

    CFRelease(result);
    return (CFStringRef)0; // FIXME: create a dynamic type
}

CFArrayRef UTTypeCreateAllIdentifiersForTag(CFStringRef inTagClass,
	CFStringRef inTag, CFStringRef inConformingToUTI)
{
    sqlite3 *pDB = 0;
    if(sqlite3_open([[LaunchServices database] UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return (CFArrayRef)0; // FIXME: log error somewhere
    }

    const char *column = "pboards";
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
        return (CFArrayRef)0;
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
        return (CFArrayRef)0;
    }

    int rc = sqlite3_step(stmt);
    NSString *uti = nil;
    CFArrayRef result = CFArrayCreateMutable(NULL, 5, NULL);
    while(rc == SQLITE_ROW) {
    	uti = [NSString stringWithCString:(const char *)sqlite3_column_text(stmt, 0)];
        CFArrayAppendValue(result, (__bridge_retained void *)uti);
	rc = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(pDB);
    CFRetain(result);
    return result;
}

CFArrayRef UTTypeCopyAllTagsWithClass(CFStringRef inUTI,
        CFStringRef inTagClass)
{
    sqlite3 *pDB = 0;
    if(sqlite3_open([[LaunchServices database] UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return (CFArrayRef)0; // FIXME: log error somewhere
    }

    const char *column = "pboards";
    if(CFStringCompare(inTagClass, kUTTagClassFilenameExtension, 0) == NSOrderedSame)
    	column = "extensions";
    else if(CFStringCompare(inTagClass, kUTTagClassMIMEType, 0) == NSOrderedSame)
        column = "mimetypes";
    else if(CFStringCompare(inTagClass, kUTTagClassOSType, 0) == NSOrderedSame)
        column = "ostypes";
    
    NSMutableString *qtemplate = [NSMutableString stringWithCString:"SELECT _column_ FROM types WHERE uti LIKE ? || ',%' OR uti LIKE '%,' || ? OR uti LIKE '%,' || ? || ',%' OR uti = ?"];
    [qtemplate replaceOccurrencesOfString:@"_column_" withString:[NSString stringWithCString:column] options:NSLiteralSearch range:NSMakeRange(0,[qtemplate length])];
    const char *query = [qtemplate UTF8String];
    const int length = [qtemplate length];
    sqlite3_stmt *stmt;
    const char *tail;

    if(sqlite3_prepare_v2(pDB, query, length, &stmt, &tail) != SQLITE_OK) {
        sqlite3_close(pDB);
        return (CFArrayRef)0;
    }

    const char *uti = CFStringGetCStringPtr(inUTI, kCFStringEncodingUTF8);
    int utilen = CFStringGetLength(inUTI);

    if(sqlite3_bind_text(stmt, 1, uti, utilen, SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 2, uti, utilen, SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 3, uti, utilen, SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 4, uti, utilen, SQLITE_STATIC) != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(pDB);
        return (CFArrayRef)0;
    }

    int rc = sqlite3_step(stmt);
    CFArrayRef result = (CFArrayRef)0;
    if(rc == SQLITE_ROW) {
    	NSString *tagString = [NSString stringWithCString:(const char *)sqlite3_column_text(stmt, 0)];
	NSArray *tags = [tagString componentsSeparatedByString:@","];
	result = (__bridge_retained CFArrayRef)tags;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(pDB);
    return result;

}

CFStringRef UTTypeCopyPreferredTagWithClass(CFStringRef inUTI,
	CFStringRef inTagClass)
{
    CFArrayRef tags = UTTypeCopyAllTagsWithClass(inUTI, inTagClass);
    CFStringRef result = (CFStringRef)0;
    if(tags != (CFArrayRef)0) {
	result = (CFStringRef)CFArrayGetValueAtIndex(tags, 0);
	CFRetain(result);
	CFRelease(tags);
    }
    return result;
}

// Not implemented yet
//CFStringRef UTCreateStringForOSType(OSType inOSType);
//OSType UTGetOSTypeFromString(CFStringRef inTag);

//CFDictionaryRef UTTypeCopyDeclaration(CFStringRef inUTI);
//CFURLRef UTTypeCopyDeclaringBundleURL(CFStringRef inUTI);
//CFStringRef UTTypeCopyDescription(CFStringRef inUTI);

