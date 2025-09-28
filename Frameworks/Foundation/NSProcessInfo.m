/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDarwinString.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSThread-Private.h>
#import <Foundation/NSPlatform.h>
#ifdef __WINDOWS__
#import <Foundation/NSPlatform_win32.h>
#endif
#import <objc/runtime.h>

@implementation NSProcessInfo

int                 NSProcessInfoArgc=0;
const char * const *NSProcessInfoArgv=NULL;

-(NSInteger)incrementCounter {
   NSInteger result;

   [_counterLock lock];
   _counter++;
   result=_counter;
   [_counterLock unlock];

   return result;
}

+(NSProcessInfo *)processInfo {
   return NSThreadSharedInstance(@"NSProcessInfo");
}

-init {
   _environment=nil;
   _arguments=nil;
   _hostName=nil;
   _processName=nil;
   _counter=0;
   _counterLock=[NSLock new];
   return self;
}

-(NSUInteger)processorCount {
   NSUnimplementedMethod();
   return 0;
}

-(NSUInteger)activeProcessorCount {
   NSUnimplementedMethod();
   return 0;
}

-(uint64_t)physicalMemory {
   NSUnimplementedMethod();
   return 0;
}

-(NSUInteger)operatingSystem {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)operatingSystemName {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)operatingSystemVersionString {
#ifdef __WINDOWS__
	OSVERSIONINFOEX osVersion;
	int systemVersion;
	NSString* versionString;
	NSString* servicePack;

	osVersion.dwOSVersionInfoSize=sizeof(osVersion);
	GetVersionEx((OSVERSIONINFO *)&osVersion);

	// Switches aren't float-friendly, so let's get our major/minor version in
	// some kind of combination that'll be easier for it to handle.  dwMajorVersion
	// can live in the 10s digit, with dwMinorVersion living in the 1s digit.
	// We'll also want to negate in the case we're not an NT_WORKSTATION productType - 
	// this only matters for Vista and up
	systemVersion =  osVersion.dwMajorVersion * 10 + osVersion.dwMinorVersion;
	if(systemVersion >= 60 && osVersion.wProductType != VER_NT_WORKSTATION) {
		systemVersion = -systemVersion;
	}
	
	if (osVersion.szCSDVersion != '\0') {
		servicePack = [NSString stringWithCString:osVersion.szCSDVersion];
	} else {
		servicePack = @"";
	}
		
	switch (systemVersion) {
		case 62:
			return [NSString stringWithFormat: @"Windows 8 %@", servicePack];
			break;
		case -62:
			return [NSString stringWithFormat: @"Windows Server 2012 %@", servicePack];
			break;
		case 61:
			return [NSString stringWithFormat: @"Windows 7 %@", servicePack];
			break;
		case -61:
			return [NSString stringWithFormat: @"Windows Server 2008 R2 %@", servicePack];
			break;
		case 60:
			return [NSString stringWithFormat: @"Windows Vista %@", servicePack];
			break;
		case -60:
			return [NSString stringWithFormat: @"Windows Server 2003 R2 %@", servicePack];
			break;
		case 52:
			return [NSString stringWithFormat: @"Windows XP Professional x64 Edition %@", servicePack];
			break;
		case 51:
			return [NSString stringWithFormat: @"Windows XP %@", servicePack];
			break;
		case 50:
			return [NSString stringWithFormat: @"Windows 2000 %@", servicePack];
			break;
		default:
			return [NSString stringWithFormat: @"%d.%d %d %d", osVersion.dwMajorVersion, osVersion.dwMinorVersion, osVersion.wServicePackMajor, osVersion.wServicePackMinor ];
			break;
	}
#else
    NSUnimplementedMethod();
    return 0;
#endif
}

-(NSString *)hostName {
   if(_hostName==nil){
    _hostName=[[[NSPlatform currentPlatform] hostName] retain];

    if(_hostName==nil)
     _hostName=@"HOSTNAME";
   }

   return _hostName;
}

-(NSString *)processName {
   if(_processName==nil){
    NSArray *arguments=[self arguments];

    if([arguments count]>0)
     _processName=[[[[[self arguments] objectAtIndex:0]
       lastPathComponent] stringByDeletingPathExtension] retain];

    if(_processName==nil){
     _processName=@"";
    }
   }

   return _processName;
}

-(void)setProcessName:(NSString *)name {
   [_processName release];
   _processName=[name copy];
}

-(int)processIdentifier {
   return NSPlatformProcessID();
}

-(NSArray *)arguments {
   if(_arguments==nil){
    _arguments=[[[NSPlatform currentPlatform] arguments] retain];
   }

   return _arguments;
}

-(NSDictionary *)environment {
   if(_environment==nil)
    _environment=[[[NSPlatform currentPlatform] environment] retain];

   return _environment;
}

-(NSString *)globallyUniqueString {
   return NSStringWithFormat(@"%@_%d_%d_%d_%d",[self hostName],
     [self processIdentifier],0,0,[self incrementCounter]);
}

@end

int __NSConstantStringClassReference[24];

FOUNDATION_EXPORT void __NSInitializeProcess(int argc,const char *argv[]) {
   NSProcessInfoArgc=argc;
   NSProcessInfoArgv=argv;
#if !defined(APPLE_RUNTIME_4) && !defined(__RAVYNOS__)
    OBJCInitializeProcess();
#endif
#if defined(__APPLE__) || defined(__RAVYNOS__)
    Class cls = objc_getClass("__builtin_NSString");
    memcpy(&__NSConstantStringClassReference, cls, sizeof(__NSConstantStringClassReference));

#if __LP64__
    extern int __CFConstantStringClassReference[24];
#else
    extern int __CFConstantStringClassReference[12];
#endif

    cls = objc_getClass("NSDarwinString");
    memcpy(&__CFConstantStringClassReference, cls, sizeof(__CFConstantStringClassReference));
    
    // Override the compiler version of the class
    //objc_addClass(&_NSConstantStringClassReference);
#endif

}

