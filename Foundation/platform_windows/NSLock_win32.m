/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS
#import <Foundation/NSLock_win32.h>
#import <Foundation/NSDate.h>

@implementation NSLock_win32

-init {
	_lock=CreateSemaphore(NULL, 1, 1, NULL);
   return self;
}

-(void)dealloc {
   CloseHandle(_lock);
   [super dealloc];
}

-(void)lock {
   WaitForSingleObject(_lock, INFINITE);
}

-(void)unlock {
   ReleaseSemaphore(_lock, 1, NULL);
}

-(BOOL)lockBeforeDate:(NSDate*)date
{
	DWORD timeout=[date timeIntervalSinceNow]*1000.0;
	return (WaitForSingleObject(_lock, timeout)==0);
}

-(BOOL)tryLock
{
	return (WaitForSingleObject(_lock, 0)==0);
}
@end
#endif
