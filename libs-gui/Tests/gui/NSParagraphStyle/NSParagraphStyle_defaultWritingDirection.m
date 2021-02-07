/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/NSParagraphStyle.h>

int main(int argc, char **argv)
{
	int ok;

	CREATE_AUTORELEASE_POOL(arp);

	ok = [NSParagraphStyle defaultWritingDirectionForLanguage: @"en"]==NSWritingDirectionLeftToRight
	  && [NSParagraphStyle defaultWritingDirectionForLanguage: @"ar"]==NSWritingDirectionRightToLeft;

	pass(ok,"[NSParagraphStyle defaultWritingDirectionForLanguage:] works");

	DESTROY(arp);
	return 0;
}

