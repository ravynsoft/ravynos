/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSColorList.h>
#import <AppKit/NSRaise.h>
#import <Foundation/NSPlatform.h>

NSString * const NSColorListDidChangeNotification = @"NSColorListDidChangeNotification";

@implementation NSColorList

static NSMutableDictionary *_namedColorLists = nil;

+ (void)_createDefaultColorLists
{
    _namedColorLists = [[NSMutableDictionary alloc] init];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *library = [[[NSPlatform currentPlatform] libraryDirectory] stringByAppendingPathComponent:@"Colors"];
    NSArray *places = [NSArray arrayWithObjects:@"/System/Library/Colors",@"/Library/Colors",library,nil];
    for(int x=0; x<[places count]; ++x) {
        NSArray *lists = [fm contentsOfDirectoryAtPath:[places objectAtIndex:x] error:NULL];
        for(int y=0; y<[lists count]; ++y) {
            NSString *name = [lists objectAtIndex:y];
            NSString *path = [[places objectAtIndex:x] stringByAppendingPathComponent:name];
            name = [name stringByDeletingPathExtension];
            NSColorList *clr = [[[NSColorList alloc] initWithName:name fromFile:path] autorelease];
            [_namedColorLists setObject:clr forKey:name];
        }
    }
}

+ (NSArray *)availableColorLists
{
    if (_namedColorLists == nil)
        [NSColorList _createDefaultColorLists];
    
    return [_namedColorLists allValues];
}

-init {
    [super init];
    _isEditable = NO;
    return self;
}

-initWithName:(NSString *)name fromFile:(NSString *)path
{
    _name = [name copy];
    _path = [path copy];
    _isEditable = NO;

    if (_path != nil) {
        BOOL isDir;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:_path isDirectory:&isDir];
        if(isDir)
            _path = [_path stringByAppendingPathComponent:[_path lastPathComponent]];
        NSData *data = [NSData dataWithContentsOfFile:_path];
        NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData:data];
        _keys = [[unarchiver decodeObjectForKey:@"NSKeys"] retain];
        _colors = [[unarchiver decodeObjectForKey:@"NSColors"] retain];
    } else {
        _keys = [[NSMutableArray alloc] init];
        _colors = [[NSMutableArray alloc] init];
    }

    return self;
}

-initWithName:(NSString *)name
{
    return [self initWithName:name fromFile:nil];
}

-(void)dealloc
{
    [_keys release];
    [_colors release];
    [_name release];
    [_path release];

    [super dealloc];
}

+ (NSColorList *)colorListNamed:(NSString *)name
{
    if (_namedColorLists == nil)
        [NSColorList _createDefaultColorLists];

    return [_namedColorLists objectForKey:name];
}

-(BOOL)isEditable {
    return _isEditable;
}

-(BOOL)editable {
    return _isEditable;
}

-(NSString *)name { return _name; }

-(NSArray *)allKeys { return _keys; }

-(NSColor *)colorWithKey:(NSString *)soughtKey indexPtr:(unsigned *)index
{
    for(int x=0; x<[_keys count] && x<[_colors count]; ++x) {
        (*index) = x;
        NSString *thisKey = [_keys objectAtIndex:x];
        if ([thisKey isEqualToString:soughtKey]) {
            return [_colors objectAtIndex:x];
        }
    }

    return nil;
}

-(NSColor *)colorWithKey:(NSString *)soughtKey
{
    unsigned index;	// unused
    return [self colorWithKey:soughtKey indexPtr:&index];
}

-(void)setColor:(NSColor *)color forKey:(NSString *)key
{
    unsigned index;

    // if we already have a color with this key, replace it...
    if ([self colorWithKey:key indexPtr:&index])
        [_colors replaceObjectAtIndex:index withObject:color];
    else {
        [_keys addObject:key];		// otherwise the color/key combo are added to the end of the list
        [_colors addObject:color];
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:NSColorListDidChangeNotification object:self];
}

-(void)removeColorWithKey:(NSString *)key
{
    unsigned index;

    if ([self colorWithKey:key indexPtr:&index]) {
        [_colors removeObjectAtIndex:index];
        [_keys removeObjectAtIndex:index];

        [[NSNotificationCenter defaultCenter] postNotificationName:NSColorListDidChangeNotification object:self];
    }
}

-(void)insertColor:(NSColor *)color key:(NSString *)key atIndex:(unsigned)index
{
    [_colors insertObject:color atIndex:index];
    [_keys insertObject:key atIndex:index];

    [[NSNotificationCenter defaultCenter] postNotificationName:NSColorListDidChangeNotification object:self];
}

-(BOOL)writeToFile:(NSString *)path
{
    NSString *p;
    NSFileManager *fm = [NSFileManager defaultManager];

//  "If path is nil, the file is saved as listname.clr in the user’s private colorlists directory."
    if(path == nil) {
        p = [[[NSPlatform currentPlatform] libraryDirectory] stringByAppendingPathComponent:@"Colors"];

        if(![fm fileExistsAtPath:_path])
            [fm createDirectoryAtPath:_path attributes:[NSDictionary new]];

        p = [[p stringByAppendingPathComponent:[self name]] stringByAppendingPathExtension:@"clr"];
    } else {
//  "If path is a directory, the receiver is saved in a file named listname.clr in that directory"
        BOOL isDir;
        BOOL exists = [fm fileExistsAtPath:path isDirectory:&isDir];
        if(isDir)
            p = [[path stringByAppendingPathComponent:[self name]] stringByAppendingPathExtension:@"clr"];
        else
            p = [path copy];
    }

    NSMutableData *data = [NSMutableData new];
    NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData:data];
    [archiver encodeObject:_keys forKey:@"NSKeys"];
    [archiver encodeObject:_colors forKey:@"NSColors"];
    [archiver finishEncoding];
    [data writeToFile:p atomically:YES];
}

-(void)removeFile
{
    [[NSFileManager defaultManager] removeFileAtPath:_path handler:nil];
}

@end

