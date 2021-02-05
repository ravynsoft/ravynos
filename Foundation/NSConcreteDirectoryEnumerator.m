/* Copyright (c) 2007 Dirk Theisen

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSConcreteDirectoryEnumerator.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSArray.h>

@implementation NSConcreteDirectoryEnumerator

- (void) skipDescendents
{
	skipDescendents = YES;
}

- (void) dealloc
{
	[startPath release];
	[list release];
	[lastFileAttributes release];
	[fm release];
	[super dealloc];
}

- (NSString*) description
{
	return [NSString stringWithFormat: @"%@, to enumerate: %@", [super description], list];
}


- (id)initWithPath: (NSString*)aPath
{
    if ((self = [super init])) {
        startPath = [aPath copy];
        fm = [[NSFileManager defaultManager] retain];
        list = [[fm directoryContentsAtPath: aPath] mutableCopy];
        lastFilePath = @"";
    }
    return self;
}


- (void) setLastFilePath: (NSString*) aPath
	/*" Sets the lastFilePath (relative) to aPath and its file attributes. "*/
{
	NSString* fullPath  = [startPath stringByAppendingPathComponent: aPath];
	NSDictionary* attrs = [fm fileAttributesAtPath: fullPath
									  traverseLink: NO];

	//NSLog(@"Found '%@' to have attributes %@", fullPath, attrs);
	[lastFilePath release];
	lastFilePath = [aPath retain];

	[lastFileAttributes release];
	lastFileAttributes = [attrs retain];
}

- (id) nextObject
{
	NSString* result = nil;
	if ([[lastFileAttributes fileType] isEqualToString: NSFileTypeDirectory]) {
		// last enumerated file was a directory
		if (! skipDescendents) {
			// Add all files in the directory to the list,
			// after making them relative to the lastFilePath:
			NSString* lastFilePathAbs = [startPath stringByAppendingPathComponent: lastFilePath];
			NSArray* dirContent = [fm directoryContentsAtPath: lastFilePathAbs];

			if ([dirContent count]) {
				NSEnumerator* dirContentEnumerator = [dirContent reverseObjectEnumerator];
				NSString* filename;

				//NSLog(@"Found dir content of '%@' to be %@", lastFilePathAbs, dirContent);

				while ((filename = [dirContentEnumerator nextObject])) {
					NSString* filePath = [lastFilePath stringByAppendingPathComponent: filename];
					[list insertObject: filePath atIndex: 0];
				}
			}
		}
	}

	if ([list count] > 0) {
		result = [[[list objectAtIndex: 0] retain] autorelease];
		[list removeObjectAtIndex: 0];
	}

	if (result) [self setLastFilePath: result];
	skipDescendents = NO;

	//NSLog(@"Enumerating %@", result);

	return result;
}




- (NSDictionary*) directoryAttributes
{
	return [fm fileAttributesAtPath: startPath traverseLink: NO];
}

- (NSDictionary*) fileAttributes
{
	return lastFileAttributes;
}


- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id *)stackbuf count:(NSUInteger)len
{
	++state->state;
	state->itemsPtr = stackbuf;
	state->mutationsPtr = (unsigned long *)self;

	id next = [self nextObject];
	if (nil == next) return 0;

	stackbuf[0] = next;
	return 1;
}

@end
