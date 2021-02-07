/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>

Test that -base's cluster of NSMutableString classes are valid NSString
classes.
*/

#import "../NSString/NSString_tests.h"

#import <Foundation/NSString.h>

int main(int argc,char **argv)
{
	TestNSStringClass([NSMutableString class]);
	return 0;
}
