#if     defined(GNUSTEP_BASE_LIBRARY)
#import <Foundation/Foundation.h>
#import <GNUstepBase/GSMime.h>
#import "Testing.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *xml = @"<?xml version=\"1.0\" encoding=\"UTF-8\"?><html></html>";
  NSString *charset = nil;
  PASS_RUNS(charset = [GSMimeDocument charsetForXml: xml], "Can determine charset of xml document.");
  PASS_EQUAL(@"UTF-8", charset, "Charset detected correctly");
  DESTROY(arp);
  return 0;
}
#else
int main(int argc,char **argv)
{
  return 0;
}
#endif
