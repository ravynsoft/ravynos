/* Copyright (c) 2006-2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSSound.h>
#import <Foundation/NSString.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSMutableDictionary.h>
#import <AppKit/NSRaise.h>

static NSMutableDictionary* sSounds = nil;

@implementation NSSound

+(NSArray *)soundUnfilteredFileTypes {
// _soundUnfilteredFileTypes is implemented in a category in the platform 
   if([self respondsToSelector:@selector(_soundUnfilteredFileTypes)])
    return [self performSelector:@selector(_soundUnfilteredFileTypes)];
   
   return nil;
}

+(NSSound *)soundNamed:(NSString *)name {
	
	if (sSounds == nil) {
		sSounds = [[NSMutableDictionary dictionaryWithCapacity: 10] retain];
	}
	
	NSSound* sound = [sSounds objectForKey: name];
	if (sound == nil) {
		@synchronized(sSounds) {
			// Look for the sound in the bundle
			// FIXME: We really have to search in other places too, like the docs say

			NSArray *types = [NSSound soundUnfilteredFileTypes];
			NSString *type;
			NSEnumerator *enumerator = [types objectEnumerator];
			while ((sound == nil) && (type = [enumerator nextObject]))
			{
				if ([[NSBundle mainBundle] pathForResource:name ofType:type]) {
					sound =  [[[NSSound alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:name ofType:type] byReference:NO] autorelease];
					// Store it for quick access if asked again
					[sSounds setObject: sound forKey: name];
				}
			}	
		}
	}
	return sound;
}
-(id)initWithContentsOfURL:(NSURL *)url byReference:(BOOL)byReference {
    self = [super init];
    return self;
}

- (id)initWithContentsOfFile:(NSString *)path byReference:(BOOL)byReference {
    self = [super init];
   return self;
}

-(BOOL)setName:(NSString *)name {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)play {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)pause {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)resume {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)stop {
   NSUnimplementedMethod();
   return NO;
}

@end

@implementation NSBundle(NSSound)

-(NSString *)pathForSoundResource:(NSString *)name {
   NSArray *types=[NSSound soundUnfilteredFileTypes];
   int      i,count=[types count];

   for(i=0;i<count;i++){
    NSString *type=[types objectAtIndex:i];
    NSString *path=[self pathForResource:name ofType:type];

    if(path!=nil)
     return path;
   }

   return nil;
}

@end

