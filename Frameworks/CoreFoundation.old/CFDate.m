#import <CoreFoundation/CFDate.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSPlatform.h>

const CFTimeInterval kCFAbsoluteTimeIntervalSince1970=978307200.0;

CFAbsoluteTime CFAbsoluteTimeGetCurrent() {
   return NSPlatformTimeIntervalSinceReferenceDate();
}

