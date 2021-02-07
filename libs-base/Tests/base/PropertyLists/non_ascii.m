/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

#import "Testing.h"

int main(int argc, char **argv)
{
	id plist;
	NSAutoreleasePool   *arp = [NSAutoreleasePool new];

	plist=[[NSString stringWithContentsOfFile: @"non_ascii_utf8.plist"] propertyList];
	PASS(plist!=nil, "utf8 plist works");

	plist=[[NSString stringWithContentsOfFile: @"non_ascii_utf16.plist"] propertyList];
	PASS(plist!=nil, "utf16 plist works");

	plist=[[NSString stringWithContentsOfFile: @"non_ascii_utf8.strings"] propertyListFromStringsFileFormat];
	PASS(plist!=nil, "utf8 strings file works");

	plist=[[NSString stringWithContentsOfFile: @"non_ascii_utf16.strings"] propertyListFromStringsFileFormat];
	PASS(plist!=nil, "utf16 strings file works");

	[arp release]; arp = nil;

	return 0;
}

