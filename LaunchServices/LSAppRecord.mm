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

#import <LaunchServices/LSAppRecord.h>

@implementation LSAppRecord

+appRecordWithURL:(NSURL *)appURL
{
    return [[self alloc] initWithURL:appURL];
}

-initWithURL:(NSURL *)appURL
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *appPath = [appURL path];
    NSDictionary *attributes = [fm fileAttributesAtPath:appPath traverseLink:NO];

    NSArray *schemes = @[
        @{ @"CFBundleTypeRole":@"Editor", @"CFBundleURLSchemes":@[@"https",@"ftp"] }
    ];

    NSArray *docTypes = @[
        @{ @"CFBundleTypeRole":@"Viewer", @"CFBundleTypeExtensions":@[@"jpg",@"JPG",@"jpeg"] },
        @{ @"CFBundleTypeRole":@"Viewer", @"CFBundleTypeExtensions":@[@"html",@"HTML"] }
    ];

    _version = 1;
    _name = @"App Name";
    _URL = [appURL copy];
    _URLSchemes = schemes;
    _documentTypes = docTypes;
    _arguments = nil;
    _lastModified = [[attributes fileModificationDate] timeIntervalSince1970];
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
