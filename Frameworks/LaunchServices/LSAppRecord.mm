/*
 * ravynOS LaunchServices
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

#import <LaunchServices/LSAppRecord.h>
#import <Foundation/NSPlatform.h>
#import <CoreFoundation/CFString.h>
#import "UTTypes.h"
#include <xdgdesktopfile.h>

@implementation LSAppRecord

+appRecordWithURL:(NSURL *)appURL {
    return [[self alloc] initWithURL:appURL];
}

-initWithURL:(NSURL *)appURL {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *appPath = [appURL path];
    NSDictionary *attributes = [fm fileAttributesAtPath:appPath traverseLink:NO];

    _version = 1;
    _URL = [appURL copy];
    _URLSchemes = nil;
    _documentTypes = nil;
    _arguments = nil;
    _lastModified = [[attributes fileModificationDate] timeIntervalSince1970];

    if([attributes fileType] == NSFileTypeDirectory)
        return [self initWithBundle:[NSBundle bundleWithPath:appPath]];
    
    if([[appURL pathExtension] isEqualToString:@"desktop"])
        return [self initWithDesktopFile:appPath];

    if(!_name)
        _name = [appURL lastPathComponent];
    return self;
}

-initWithBundle:(NSBundle *)app {
    NSDictionary *properties = [app localizedInfoDictionary];

    _name = [properties objectForKey:@"CFBundleDisplayName"];
    if(!_name)
        [properties objectForKey:@"CFBundleName"];

    _documentTypes = [properties objectForKey:@"CFBundleDocumentTypes"];
    _URLSchemes = [properties objectForKey:@"CFBundleURLTypes"];
    return self;
}

-initWithDesktopFile:(NSString *)path {
    XdgDesktopFile df;
    if(!df.load([path UTF8String]) || !df.isValid() || df.type() != XdgDesktopFile::ApplicationType)
        return self;
    QString qsname = df.name();
    _name = [NSString stringWithCString:qsname.toLocal8Bit().constData() length:qsname.length()];

    // Find the actual executable and save it as the appURL.
    // Save the remaining command line as the arguments list.
    QString exec = df.value("Exec").toString();
    NSString *execPath = [NSString stringWithCString:exec.toLocal8Bit().constData() length:exec.length()];
    NSMutableArray *components = [execPath componentsSeparatedByString:@" "];

    execPath = [components firstObject];
    [components removeObjectAtIndex:0];
    _arguments = components;

    NSFileManager *fm = [NSFileManager defaultManager];
    if([execPath hasPrefix:@"/"] == NO) {
        // search the PATH to find absolute path of this file
        NSDictionary *env = [[NSPlatform currentPlatform] environment];
	NSArray *paths = [[env objectForKey:@"PATH"] componentsSeparatedByString:@":"];
	for(int i = 0; i < [paths count]; ++i) {
	    NSString *entry = [paths objectAtIndex:i];
	    if([fm fileExistsAtPath:[entry stringByAppendingPathComponent:execPath]]) {
		execPath = [entry stringByAppendingPathComponent:execPath];
		break;
	    }
	}
    }
    _URL = [NSURL fileURLWithPath:execPath];

    // Now we have the executable and args. Determine what this app can
    // accept and store them in _documentTypes. We do this by extracting
    // the MIME types and converting them to UTIs.

    QStringList mimetypes = df.mimeTypes();
    NSMutableArray *documentTypes = [NSMutableArray arrayWithCapacity:5];

    for(QStringList::iterator iter = mimetypes.begin(); iter != mimetypes.end(); iter++) {
	CFStringRef tag = CFStringCreateWithCString(NULL, iter->toUtf8(), kCFStringEncodingUTF8);
	CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassMIMEType, tag, NULL);

	if(uti != NULL) {
	    NSMutableDictionary *aType = [NSMutableDictionary dictionaryWithCapacity:5];
	    [aType setObject:(__bridge_transfer NSString*)tag forKey:@"CFBundleTypeName"];
	    [aType setObject:@"Editor" forKey:@"CFBundleTypeRole"]; // FIXME: should it be Viewer?
	    [aType setObject:@"Alternate" forKey:@"LSHandlerRank"];
	    NSArray *types = [NSArray arrayWithObjects:(__bridge_transfer NSString*)uti,nil];
	    [aType setObject:types forKey:@"LSItemContentTypes"];
	    [documentTypes addObject:aType];
	}
    }

    if([documentTypes count])
    	_documentTypes = documentTypes;
    return self;
}

-initWithCoder:(NSCoder *)coder {
    self = [super init];
    _version = [coder decodeIntForKey:@"version"];
    _name = [coder decodeObjectForKey:@"name"];
    _URL = [coder decodeObjectForKey:@"URL"];
    _URLSchemes = [coder decodeObjectForKey:@"URLSchemes"];
    _documentTypes = [coder decodeObjectForKey:@"documentTypes"];
    _arguments = [coder decodeObjectForKey:@"arguments"];
    _lastModified = [coder decodeDoubleForKey:@"modified"];
    return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
    [coder encodeInt:_version forKey:@"version"];
    [coder encodeObject:_name forKey:@"name"];
    [coder encodeObject:_URL forKey:@"URL"];
    [coder encodeObject:_URLSchemes forKey:@"URLSchemes"];
    [coder encodeObject:_documentTypes forKey:@"documentTypes"];
    [coder encodeObject:_arguments forKey:@"arguments"];
    [coder encodeDouble:_lastModified forKey:@"modified"];
}

-(void)setVersion:(int)version {
    _version = version;
}

-(void)setName:(NSString *)name {
    _name = [name copy];
}

-(void)setURL:(NSURL *)URL {
    _URL = [URL copy];
}

-(void)setURLSchemes:(NSArray *)schemes {
    _URLSchemes = [schemes copy];
}

-(void)setDocumentTypes:(NSArray *)documentTypes {
    _documentTypes = [documentTypes copy];
}

-(void)setArguments:(NSArray *)arguments {
    _arguments = [arguments copy];
}

-(void)setModificationDate:(NSDate *)date {
    _lastModified = [date timeIntervalSince1970];
}

-(int)version {
    return _version;
}

-(NSString *)name {
    return _name;
}

-(NSURL *)URL {
    return _URL;
}

-(NSArray *)URLSchemes {
    return _URLSchemes;
}

-(NSArray *)documentTypes {
    return _documentTypes;
}

-(NSArray *)arguments {
    return _arguments;
}

-(NSDate *)modificationDate {
    return [NSDate dateWithTimeIntervalSince1970:_lastModified];
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@ 0x%08x> %@ %@ %@", [self class], self, _URL, _name, [NSDate dateWithTimeIntervalSince1970:_lastModified]];
}

@end
