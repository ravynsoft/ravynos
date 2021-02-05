/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifdef PLATFORM_IS_POSIX
#import <Foundation/NSPersistantDomain_posix.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSPropertyListReader.h>
#import <Foundation/NSPropertyListWriter_vintage.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSException.h>
#import <Foundation/NSArray.h>

@implementation NSPersistantDomain_posix

-initWithName:(NSString *)name {
    NSDictionary *serializedDictionary;
    
    _path = [NSHomeDirectory() stringByAppendingPathComponent:@"Library"];
    _path = [_path stringByAppendingPathComponent:@"NSUserDefaults"];
    _path = [_path stringByAppendingPathComponent:name];
    _path = [[_path stringByAppendingPathExtension:@"plist"] retain];
    
    serializedDictionary = [NSPropertyListReader dictionaryWithContentsOfFile:_path];
    if (serializedDictionary != nil)
        _mutableDomain = [serializedDictionary mutableCopy];
    else
        _mutableDomain = [[NSMutableDictionary alloc] init];

    return self;
}

-(void)dealloc {
    [_mutableDomain release];
    [_path release];
    
    [super dealloc];
}

+(NSPersistantDomain_posix *)persistantDomainWithName:(NSString *)name {
    return [[[self allocWithZone:NULL] initWithName:name] autorelease];
}

-(NSArray *)allKeys {
    return [_mutableDomain allKeys];
}

-(NSEnumerator *)keyEnumerator {
    return [[self allKeys] objectEnumerator];
}

-objectForKey:(NSString *)key {
    return [_mutableDomain objectForKey:key];
}

-(void)setObject:object forKey:(NSString *)key {
    [_mutableDomain setObject:object forKey:key];
}

-(void)removeObjectForKey:(NSString *)key {
    [_mutableDomain removeObjectForKey:key];
}

-(void)createUserDefaultsDirectoryIfNeeded {
   BOOL      isDirectory;
   NSArray  *components=[_path pathComponents];
   NSInteger       i,count=[components count];
   NSString *check=@"";
   
   for(i=0;i<count;i++){
   // leave the error checking up to -synchronize
    check=[check stringByAppendingPathComponent:[components objectAtIndex:i]];
    if(i>0 && ![[NSFileManager defaultManager] fileExistsAtPath:check isDirectory:&isDirectory]){
     [[NSFileManager defaultManager] createDirectoryAtPath:check attributes:nil];
    }
   }
}

-(void)synchronize {
   [self createUserDefaultsDirectoryIfNeeded];
    if ([NSPropertyListWriter_vintage writePropertyList:_mutableDomain toFile:_path atomically:YES] == NO)
        [NSException raise:NSInternalInconsistencyException
                    format:@"Cannot synchronize NSUserDefaults to %@", _path];
}

@end
#endif
