/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>

Test -base's cluster of NSString classes. This tests the optimized
implementations of many NSString methods in GSString and its subclasses.
*/

#import "NSString_tests.h"

#import <Foundation/NSString.h>

int main(int argc,char **argv)
{
  TestNSStringClass([NSString class]);
  return 0;
}

