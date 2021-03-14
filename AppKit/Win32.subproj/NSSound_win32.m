/* Copyright (c) 2008 Julian Mayer

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSSound_win32.h"
#import <Foundation/NSString.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSURL.h>
#import <AppKit/NSRaise.h>
#import <windows.h>

// zero is null number
static unsigned int uniquenum = 1;

@interface NSSound(win32)
@end

@implementation NSSound(win32)

+allocWithZone:(NSZone *)zone {
    Class directShow=NSClassFromString(@"NSSound_DirectShow");
    if(directShow!=Nil)
        return NSAllocateObject(directShow,0,NULL);
    
   return NSAllocateObject([NSSound_win32 class],0,NULL);
}

+(NSArray *)_soundUnfilteredFileTypes {
	//	FIXME: Instead of returned this predetermined set (XPSP2) we should query it something *like* GetProfileString("mci extensions",....);
	  
   return [NSArray arrayWithObjects:@"wav",@"aif",@"aifc",@"aiff",@"asf",@"asx",@"au",@"m1v",@"m3u",@"mp2",@"mp2v",@"mp3",@"mpa",@"mpe",@"mpeg",@"mpg",@"mpv2",@"snd",@"wax",@"wm",@"wma",@"wmv",@"wmx",@"wpl",@"wvx",nil];
}

@end

@implementation NSSound_win32

- (id)initWithContentsOfURL:(NSURL *)url byReference:(BOOL)byReference {
	return [self initWithContentsOfFile:[url path] byReference:byReference];
}

- (id)initWithContentsOfFile:(NSString *)path byReference:(BOOL)byReference {
	if ((self = [super initWithContentsOfFile:path byReference:byReference]))
	{
		_soundFilePath = [path copy];
		_paused = NO;
		_handle = 0;
	}
	return self;
}

-(BOOL)play {
    if (_handle == 0) {
     _handle = uniquenum++;
    }
    else {
/* If the sound has already been played we need to close it before playing again or it wont play. */
        NSString *stopStr = [NSString stringWithFormat:@"close %i", _handle];
        mciSendString([stopStr UTF8String], NULL, 0, 0);
    }

    NSString *loadStr = [NSString stringWithFormat:@"open \"%@\" type %@ alias %i", _soundFilePath, [[_soundFilePath pathExtension] isEqualToString:@"wav"] ? @"waveaudio" : @"MPEGVideo", _handle];
    if (mciSendString([loadStr UTF8String], NULL, 0, 0))
	 	return NO;

	NSString *playStr = [NSString stringWithFormat:@"play %i from 0", _handle];
	if (mciSendString([playStr UTF8String], NULL, 0, 0))
		return NO;

	return YES;
}

-(BOOL)pause {
	if (_paused)
		return NO;
	else
	{
        if(_handle!=0){
 		 NSString *pauseStr = [NSString stringWithFormat:@"pause %i", _handle];
		 mciSendString([pauseStr UTF8String], NULL, 0, 0);
        }
		_paused = YES;
	}
	return YES;
}

-(BOOL)resume {
	if (!_paused)
		return NO;
	else
	{
        if(_handle!=0){
 		 NSString *pauseStr = [NSString stringWithFormat:@"resume %i", _handle];
		 mciSendString([pauseStr UTF8String], NULL, 0, 0);
        }
		_paused = NO;
	}
	return YES;
}

-(BOOL)stop {
    if(_handle!=0){
	 NSString *stopStr = [NSString stringWithFormat:@"stop %i", _handle];
	 mciSendString([stopStr UTF8String], NULL, 0, 0);
	}
    
	return YES;
}

-(void)dealloc {
    if(_handle!=0){
 	 NSString *stopStr = [NSString stringWithFormat:@"close %i", _handle];
	 mciSendString([stopStr UTF8String], NULL, 0, 0);
    }
    [_soundFilePath release];
	[super dealloc];
}

@end
