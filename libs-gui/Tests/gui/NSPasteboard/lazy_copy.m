/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#include "Testing.h"

#include <Foundation/NSArray.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>
#include <AppKit/NSPasteboard.h>

static NSString *theString=@"QUUX!!1!!\"$!";

@interface Foo : NSObject
@end

@implementation Foo
+(void) pasteboard: (NSPasteboard *)pb
	provideDataForType: (NSString *)type
{
//	printf("pasteboard: %@ provideDataForType: %@\n",pb,type);
	[pb setString: theString
		forType: NSStringPboardType];
}
@end

int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);

	NSPasteboard *pb=[NSPasteboard pasteboardWithName: @"lazy copy test"];
	NSString *s;

	[pb declareTypes: [NSArray arrayWithObject: NSStringPboardType]
		owner: [Foo self]];
	DESTROY(arp);

	arp=[NSAutoreleasePool new];
	pb=[NSPasteboard pasteboardWithName: @"lazy copy test"];
	s=[pb stringForType: NSStringPboardType];

	testHopeful = YES;
	pass([s isEqual: theString], "NSPasteboard handles lazy setting of data");
	testHopeful = NO;

	DESTROY(arp);

	return 0;
}


