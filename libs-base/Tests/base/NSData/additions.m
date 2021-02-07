#import <Foundation/Foundation.h>

#import "Testing.h"

#import <GNUstepBase/NSData+GNUstepBase.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSData        *data;
  NSData        *ref;
  NSUInteger    last;
  NSUInteger    length;

  ref = [NSData dataWithContentsOfFile: @"Lorum"];
  PASS(NO == [ref isGzipped], "Reference data is not gzipped");

  data = [ref gzipped: 0];
  PASS(YES == [data isGzipped], "We can gzip with compression 0");

  length = [data length];
  PASS(length > [ref length], "Compression 0 does not compress");
  PASS_EQUAL([data gunzipped], ref, "gunzipped 0 matches reference");

  data = [ref gzipped: 1];
  length = [data length];
  PASS(length < [ref length], "Compression 1 does compress");
  last = length;
  PASS_EQUAL([data gunzipped], ref, "gunzipped 1 matches reference");

  data = [ref gzipped: 3];
  length = [data length];
  PASS(length < last, "Compression 3 is smaller than 1");
  last = length;
  PASS_EQUAL([data gunzipped], ref, "gunzipped 3 matches reference");

  data = [ref gzipped: 5];
  length = [data length];
  PASS(length < last, "Compression 5 is smaller than 3");
  last = length;
  PASS_EQUAL([data gunzipped], ref, "gunzipped 5 matches reference");

  data = [ref gzipped: 9];
  length = [data length];
  PASS(length < last, "Compression 9 is smaller than 5");
  last = length;
  PASS_EQUAL([data gunzipped], ref, "gunzipped 9 matches reference");

  data = [NSData data];
  PASS(NO == [data isGzipped], "An empty data is not gzipped");

  data = [data gzipped: -1];
  PASS(YES == [data isGzipped], "An empty can be gzipped");

  data = [data gzipped: -1];
  PASS(YES == [data isGzipped], "An empty can be gzipped twice");

  data = [data gunzipped];
  PASS(YES == [data isGzipped], "Partial gunzip is still gzipped");

  data = [data gunzipped];
  PASS(NO == [data isGzipped], "Complete gunzip is not gzipped");

  PASS(0 == [data length], "Complete gunzip is empty");

  [arp release]; arp = nil;
  return 0;
}
