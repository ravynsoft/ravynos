/*
 * Airyx LaunchServices
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

#import <Foundation/NSRaiseException.h>
#import <Foundation/NSPlatform.h>
#import <CoreFoundation/CoreFoundation.h>
#import <LaunchServices/LaunchServices.h>
#import "LSAppRecord.h"
#import "UTTypes.h"

#include <sqlite3.h>
#include <stdio.h>

    
NSString *LS_DATABASE = [[[NSPlatform currentPlatform] libraryDirectory] stringByAppendingString:@"/db/launchservices.db"];

// #include <KIO/ApplicationLauncherJob>
// #include <KIO/OpenUrlJob>
// #include <KService>


// FIXME: these should talk to a privileged service (maybe over DBus) but for now we'll
// just manipulate some files. The service is /System/Library/CoreServices/launchservicesd
// on macOS.
// See https://developer.gnome.org/DBusApplicationLaunching/ and 
// https://techbase.kde.org/Development/Tutorials/D-Bus/Autostart_Services

// FIXME: which error code to return for each case is just a guess

// FIXME: stuff to track per application:
// - DBus activatable boolean

//------------------------------------------------------------------------
//    INTERNAL FUNCTIONS - DON'T USE. SEE BELOW FOR PUBLIC API
//------------------------------------------------------------------------

static BOOL _LSInitializeDatabase()
{
    NSFileManager *fm = [NSFileManager defaultManager];
    if([fm fileExistsAtPath:LS_DATABASE])
        return true;

    [fm createFileAtPath:LS_DATABASE contents:[NSData new] attributes:[NSDictionary new]];

    sqlite3 *pDB = 0;
    if(sqlite3_open([LS_DATABASE UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return false; // FIXME: log error somewhere
    }

    // Create and populate the DB from our resource file
    NSString *sqlPath = [[NSBundle bundleForClass:[LaunchServices class]] pathForResource:@"InitDB" ofType:@"sql"];
    if(sqlPath == nil) {
    	NSLog(@"ERROR: cannot find InitDB.sql to init launchservices.db");
    } else {
        sqlite3_stmt *stmt;
        const char *tail;
        FILE *sql = fopen([sqlPath UTF8String], "r");
	size_t length;
	char *line = fgetln(sql, &length);

	while(length > 0) {
	    if(sqlite3_prepare_v2(pDB, line, length, &stmt, &tail) != SQLITE_OK) {
	    	sqlite3_close(pDB);
		fclose(sql);
		return false;
	    }
	    sqlite3_step(stmt);
	    sqlite3_finalize(stmt);
	    line = fgetln(sql, &length);
	}
    }
    sqlite3_close(pDB);
    return true;
}

static BOOL _LSFindRecordInDatabase(const NSURL *appURL, LSAppRecord **appRecord)
{
    sqlite3 *pDB = 0;
    if(sqlite3_open([LS_DATABASE UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return false; // FIXME: log error somewhere
    }
    
    const char *query = "SELECT * FROM applications WHERE url=? OR basename=?";
    const int length = strlen(query);
    sqlite3_stmt *stmt;
    const char *tail;

    if(sqlite3_prepare_v2(pDB, query, length, &stmt, &tail) != SQLITE_OK) {
        sqlite3_close(pDB);
        return false;
    }

    if(sqlite3_bind_text(stmt, 1, [[appURL absoluteString] UTF8String], [[appURL absoluteString] length], SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 2, [[appURL lastPathComponent] UTF8String], [[appURL lastPathComponent] length], SQLITE_STATIC) != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(pDB);
        return false;
    }

    int rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW) {
        NSData *blob = [[NSData alloc] 
            initWithBytes:sqlite3_column_blob(stmt, 3)
            length:sqlite3_column_bytes(stmt, 3)];
        *appRecord = [NSKeyedUnarchiver unarchiveObjectWithData:blob];
    }

    sqlite3_finalize(stmt);
    sqlite3_close(pDB);
    return (rc == SQLITE_ROW) ? true : false;
}

static OSStatus _LSFindAppsForUTI(NSString *uti, NSMutableArray **outAppURLs)
{
    sqlite3 *pDB = 0;
    if(sqlite3_open([LS_DATABASE UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return kLSServerCommunicationErr; // FIXME: log error somewhere
    }
    
    const char *query = "SELECT application,rank FROM typemap WHERE uti=? ORDER BY rank ASC";
    const int length = strlen(query);
    sqlite3_stmt *stmt;
    const char *tail;

    if(sqlite3_prepare_v2(pDB, query, length, &stmt, &tail) != SQLITE_OK) {
        sqlite3_close(pDB);
        return kLSServerCommunicationErr;
    }

    if(sqlite3_bind_text(stmt, 1, [uti UTF8String], [uti length], SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(pDB);
        return kLSServerCommunicationErr;
    }

    int rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW)
        return kLSApplicationNotFoundErr;

    for(; rc == SQLITE_ROW; rc = sqlite3_step(stmt)) {
        NSString *url = [NSString stringWithCString:(const char *)sqlite3_column_text(stmt, 0)];
        [*outAppURLs addObject:[NSURL URLWithString:url]];
    }

    sqlite3_finalize(stmt);
    sqlite3_close(pDB);
    return 0;
}

static BOOL _LSAddRecordToDatabase(const LSAppRecord *appRecord, BOOL isUpdate) {
    sqlite3 *pDB = 0;
    if(sqlite3_open([LS_DATABASE UTF8String], &pDB) != SQLITE_OK) {
        sqlite3_close(pDB);
        return false; // FIXME: log error somewhere
    }
    
    const char *query;
    if(isUpdate)
        query = "UPDATE applications SET basename=?2, version=?3, apprecord=?4 WHERE url=?1";
    else
        query = "INSERT INTO applications (url,basename,version,apprecord) VALUES (?1,?2,?3,?4)";
    int length = strlen(query);
    sqlite3_stmt *stmt;
    const char *tail;

    if(sqlite3_prepare_v2(pDB, query, length, &stmt, &tail) != SQLITE_OK) {
        sqlite3_close(pDB);
        return false;
    }

    NSData *blob = [NSKeyedArchiver archivedDataWithRootObject:appRecord];
    if(sqlite3_bind_text(stmt, 1, [[[appRecord URL] absoluteString] UTF8String], [[[appRecord URL] absoluteString] length], SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_text(stmt, 2, [[[appRecord URL] lastPathComponent] UTF8String], [[[appRecord URL] lastPathComponent] length], SQLITE_STATIC) != SQLITE_OK
        || sqlite3_bind_int(stmt, 3, [appRecord version]) != SQLITE_OK
        || sqlite3_bind_blob(stmt, 4, [blob bytes], [blob length], SQLITE_STATIC) != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(pDB);
        return false;
    }

    if(sqlite3_step(stmt) != SQLITE_DONE)
        return false;

    sqlite3_finalize(stmt);

    query = "DELETE FROM typemap WHERE application = ?";
    length = strlen(query);

    if(sqlite3_prepare_v2(pDB, query, length, &stmt, &tail) != SQLITE_OK) {
        sqlite3_close(pDB);
        return false;
    }

    if(sqlite3_bind_text(stmt, 1, [[[appRecord URL] absoluteString] UTF8String], [[[appRecord URL] absoluteString] length], SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(pDB);
        return false;
    }

    // we don't care if this one fails because there may not be records for this app
    sqlite3_step(stmt); 
    sqlite3_finalize(stmt);

    query = "INSERT INTO typemap (uti,application,rank) VALUES (?,?,?)";
    length = strlen(query);

    NSEnumerator *docTypes = [[appRecord documentTypes] objectEnumerator];
    while(NSDictionary *docType = [docTypes nextObject]) {
	int rank = kLSRankAlternate;
	NSString *nsrank = [docType objectForKey:@"LSHandlerRank"];
	if([nsrank isEqualToString:@"Default"])
	    rank = kLSRankDefault;
	else if([nsrank isEqualToString:@"Owner"])
	    rank = kLSRankOwner;

	NSMutableArray *things = [docType objectForKey:kLSItemContentTypesKey];
	if([things count] == 0) {
	    // do the old keys exist?
	    NSArray *extensions = [docType objectForKey:kCFBundleTypeExtensionsKey];
	    for(int x = 0; x < [extensions count]; ++x) {
		CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension,
		    (CFStringRef)[extensions objectAtIndex:x], NULL);
		if(uti)
		    [things addObject:(id)uti];
	    }
	    // FIXME: also check for MIME Types key
	}

	for(int x = 0; x < [things count]; ++x) {
	    if(sqlite3_prepare_v2(pDB, query, length, &stmt, &tail) != SQLITE_OK) {
		sqlite3_close(pDB);
		return false;
	    }

	    NSString *uti = [things objectAtIndex:x];	
	
	    if(sqlite3_bind_text(stmt, 1, [uti UTF8String], [uti length], SQLITE_STATIC) != SQLITE_OK
		|| sqlite3_bind_text(stmt, 2, [[[appRecord URL] absoluteString] UTF8String], [[[appRecord URL] absoluteString] length], SQLITE_STATIC) != SQLITE_OK
		|| sqlite3_bind_int(stmt, 3, rank) != SQLITE_OK)
	    {
		sqlite3_finalize(stmt);
		sqlite3_close(pDB);
		return false;
	    }

	    sqlite3_step(stmt);
	    sqlite3_finalize(stmt);
	}
    }

    sqlite3_close(pDB);
    return true;
}

static OSStatus _LSOpenAllWithSpecifiedApp(const LSLaunchURLSpec *inLaunchSpec, CFURLRef _Nullable *outLaunchedURL)
{
    const NSURL *appURL = (NSURL *)inLaunchSpec->appURL;
    if([appURL isFileURL] == NO)
        return kLSDataErr;

    // Launch app and pass all itemURLs to it
    // FIXME: use GURL or odoc events instead of passing on cmd line
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *appPath = [appURL path];
    BOOL isDir;
    BOOL exists = [fm fileExistsAtPath:appPath isDirectory:&isDir];

    if(exists == NO)
        return kLSNoExecutableErr;

    if(isDir == YES) {
        NSBundle *app = [NSBundle bundleWithPath:appPath];
        if([fm isExecutableFileAtPath:[app executablePath]] == NO)
            return kLSNoExecutableErr;

        if(outLaunchedURL != NULL)
            *outLaunchedURL = (CFURLRef)[NSURL fileURLWithPath:[app executablePath]];
        
        NSMutableArray *args = [[app infoDictionary] objectForKey:@"ProgramArguments"];
        [args addObjectsFromArray:(NSArray *)inLaunchSpec->itemURLs];
        [NSTask launchedTaskWithLaunchPath:[app executablePath] arguments:args];
    } else {
        // it's not a bundle so just try to exec the file
        if([fm isExecutableFileAtPath:appPath] == NO)
            return kLSNoExecutableErr;

        if(outLaunchedURL != NULL)
            *outLaunchedURL = (CFURLRef)[NSURL fileURLWithPath:appPath];

        // Check if we have any stored arguments in the database
        LSAppRecord *appRecord;
        NSMutableArray *args = [NSMutableArray alloc];
        if(_LSFindRecordInDatabase(appURL, &appRecord) == YES) {
            [args addObjectsFromArray:[appRecord arguments]];
        }

        // Just open the app unless there are itemURLs
        // Otherwise, launch the app with each one
        if([(NSArray *)inLaunchSpec->itemURLs count] == 0) {
            [args retain];
            [NSTask launchedTaskWithLaunchPath:appPath arguments:args];
            [args release];
            return 0;
        }

	NSEnumerator *items = [(NSArray *)inLaunchSpec->itemURLs objectEnumerator];
        BOOL found = NO;

	while(NSURL *item = [items nextObject]) {
            NSMutableArray *copyargs = args;
            for(int i=0; i<[copyargs count]; ++i) {
                if([[copyargs objectAtIndex:i] caseInsensitiveCompare:@"%U"] == NSOrderedSame) {
                    [copyargs replaceObjectAtIndex:i withObject:[item absoluteString]];
                    found = YES;
                }
                if([[copyargs objectAtIndex:i] caseInsensitiveCompare:@"%F"] == NSOrderedSame) {
                    [copyargs replaceObjectAtIndex:i withObject:[item path]];
                    found = YES;
                }
            }

            if(found == NO)
                [copyargs addObject:[item path]];
            [copyargs retain];
            [NSTask launchedTaskWithLaunchPath:appPath arguments:copyargs];
            [copyargs release];
        }
        [args release];
    }
    return 0;
}

static BOOL _acceptsThing(NSArray *things, NSString *aThing)
{
    NSEnumerator *thingEnumerator = [things objectEnumerator];
    NSString *currentThing;
    while(currentThing = [thingEnumerator nextObject]) {
        if([currentThing isEqualToString:@"*"] || [currentThing isEqualToString:aThing]) {
            return YES;
        }
    }
    return NO;
}

static BOOL _acceptsRole(NSString *role, LSRolesMask rolesMask)
{
    if([role isEqualToString:@"None"]) return NO;
    if(rolesMask == kLSRolesAll) return YES;
    if([role isEqualToString:@"Shell"] && (rolesMask & kLSRolesShell)) return YES;
    if([role isEqualToString:@"Viewer"] && (rolesMask & kLSRolesViewer)) return YES;
    if([role isEqualToString:@"Editor"] && (rolesMask & kLSRolesEditor)) return YES;
    return NO;
}

BOOL LSIsNSBundle(NSURL *url)
{
    if([[url scheme] isEqualToString:@"file"] &&
       [[url pathExtension] isEqualToString:@"app"])
       return YES;
    return NO;
}

BOOL LSIsAppDir(NSURL *url)
{
    if([[url scheme] isEqualToString:@"file"] &&
       [[url pathExtension] caseInsensitiveCompare:@"appdir"] == NSOrderedSame) {
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *appRunPath = [[url URLByAppendingPathComponent:@"AppRun"] path];
	if([fm fileExistsAtPath:appRunPath] && [fm isExecutableFileAtPath:appRunPath])
	    return YES;
    }
    return NO;
}

//------------------------------------------------------------------------
//    PUBLIC API
//------------------------------------------------------------------------

OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef _Nullable *outLaunchedURL)
{
    LSLaunchURLSpec spec;
    spec.appURL = 0;
    spec.asyncRefCon = 0;
    spec.itemURLs = CFArrayCreate(NULL, (const void **)&inURL, 1, NULL);
    spec.launchFlags = kLSLaunchDefaults;
    OSStatus rc = LSOpenFromURLSpec(&spec, outLaunchedURL);
    CFRelease(spec.itemURLs);
    return rc;
}

OSStatus LSOpenFromURLSpec(const LSLaunchURLSpec *inLaunchSpec, CFURLRef _Nullable *outLaunchedURL)
{
    _LSInitializeDatabase();

    if(inLaunchSpec->appURL) {
        // We are launching this specific application which must be a file URL
        return _LSOpenAllWithSpecifiedApp(inLaunchSpec, outLaunchedURL);
    }

    // We are opening one or more files or URLs with their preferred apps
    // If any of the items refer to application bundles, they will be launched
    // NOTE: Plain executable files are NOT considered "applications" here
    // because file permissions are not reliable. e.g. Samba shares often have
    // execute permission on non-executable files.

    NSEnumerator *items = [(id)(inLaunchSpec->itemURLs) objectEnumerator];
    NSURL *item;

    while(item = [items nextObject]) {
        if(LSIsNSBundle(item)) {
            LSLaunchURLSpec spec;
	    spec.appURL = (CFURLRef)item;
	    spec.itemURLs = NULL;
	    spec.launchFlags = kLSLaunchDefaults;
	    _LSOpenAllWithSpecifiedApp(&spec, NULL);
        } else if(LSIsAppDir(item)) {
            LSLaunchURLSpec spec;
	    spec.appURL = (CFURLRef)([item URLByAppendingPathComponent:@"AppRun"]);
	    spec.itemURLs = NULL;
	    spec.launchFlags = kLSLaunchDefaults;
	    _LSOpenAllWithSpecifiedApp(&spec, NULL);
        } else {
            NSMutableArray *appCandidates = [NSMutableArray arrayWithCapacity:6];
	    NSString *uti = (NSString *)UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension,
	    	(CFStringRef)[item pathExtension], NULL); 
            if(_LSFindAppsForUTI(uti, &appCandidates) == 0) {
                LSLaunchURLSpec spec;
                spec.appURL = (CFURLRef)[[appCandidates firstObject] copy];
                spec.itemURLs = (CFArrayRef)[NSArray arrayWithObject:item];
                spec.launchFlags = kLSLaunchDefaults;
                _LSOpenAllWithSpecifiedApp(&spec, NULL);
            }
        }    
    }

    return 0;
}

OSStatus LSRegisterURL(CFURLRef inURL, Boolean inUpdate)
{
    // Per Apple, inURL must be a file URL that refers to an app bundle
    // or executable. This version will also accept a .desktop file as
    // a special case.
    _LSInitializeDatabase();

    NSURL *appURL = (NSURL *)inURL;
    if([appURL isFileURL] == NO)
        return kLSDataErr;

    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *appPath = [appURL path];
    BOOL isDir;
    BOOL exists = [fm fileExistsAtPath:appPath isDirectory:&isDir];
    BOOL isDesktopFile = [appPath hasSuffix:@".desktop"];

    if(exists == NO)
        return kLSNoExecutableErr;

    if(isDir == YES) {
        NSBundle *app = [NSBundle bundleWithPath:appPath];
        if([fm isExecutableFileAtPath:[app executablePath]] == NO)
            return kLSNoExecutableErr;
    } else {
        if( isDesktopFile == NO && [fm isExecutableFileAtPath:appPath] == NO)
            return kLSNoExecutableErr;
    }

    NSDictionary *attributes = [fm fileAttributesAtPath:appPath traverseLink:NO];

    // Does this app exist in the database already?
    LSAppRecord *appRecord = [[LSAppRecord alloc] initWithURL:appURL];
    LSAppRecord *appRecordInDB = [LSAppRecord new];

    // Use the parsed appRecord to handle .desktop files: the Exec=
    // is not the same as the appURL. Sigh.
    BOOL inDatabase = _LSFindRecordInDatabase([appRecord URL], &appRecordInDB);

    if(inDatabase == YES && inUpdate == NO && [appRecord modificationDate] == [attributes fileModificationDate])
        return 0; // Date hasn't changed and "force update" not specified

    // Either record did not exist, file has been modified, or an update is forced
    BOOL rc = _LSAddRecordToDatabase(appRecord, inDatabase);
    return (rc == true) ? 0 : kLSServerCommunicationErr;
}

OSStatus LSCanURLAcceptURL(CFURLRef inItemURL, CFURLRef inTargetURL, LSRolesMask inRoleMask, LSAcceptanceFlags inFlags, Boolean *outAcceptsItem)
{
    *outAcceptsItem = NO;
    _LSInitializeDatabase();

    NSURL *appURL = (NSURL *)inTargetURL;
    if([appURL isFileURL] == NO)
        return kLSDataErr;

    LSAppRecord *appRecord = [LSAppRecord new];
    if(_LSFindRecordInDatabase(appURL, &appRecord) == NO)
        return kLSApplicationNotFoundErr;

    if([[(NSURL *)inItemURL scheme] isEqualToString:@"file"]) {
        NSString *ext = [(NSURL *)inItemURL pathExtension];
        CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension,
		(CFStringRef)ext, NULL);

        NSEnumerator *docTypes = [[appRecord documentTypes] objectEnumerator];
        NSDictionary *docType;

        while(docType = [docTypes nextObject]) {
            NSArray *things = [docType objectForKey:kLSItemContentTypesKey];
            if([things count]) {
                if(_acceptsThing(things, (NSString*)uti) && _acceptsRole([docType objectForKey:kCFBundleTypeRoleKey], inRoleMask)) {
                    *outAcceptsItem = YES;
                    break;
                }
            } else {
                things = [docType objectForKey:kCFBundleTypeExtensionsKey];
                if(_acceptsThing(things, ext) && _acceptsRole([docType objectForKey:kCFBundleTypeRoleKey], inRoleMask)) {
                    *outAcceptsItem = YES;
                    break;
                }
            }
        }
    } else {
        NSString *itemScheme = [(NSURL *)inItemURL scheme];
        NSEnumerator *appSchemes = [[appRecord URLSchemes] objectEnumerator];
        NSDictionary *appScheme;

        while(appScheme = [appSchemes nextObject]) {
            if(_acceptsThing([appScheme objectForKey:kCFBundleURLSchemesKey], itemScheme) && _acceptsRole([appScheme objectForKey:kCFBundleTypeRoleKey], inRoleMask)) {
                *outAcceptsItem = YES;
                break;
            }
        }
    }

    return 0;
}

@implementation LaunchServices
@end

