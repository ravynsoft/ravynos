/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/NSPrintInfo.h>

int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);

	/* Should run without causing any exceptions. */
	[NSPrintInfo sharedPrintInfo];

	DESTROY(arp);
	return 0;
}

