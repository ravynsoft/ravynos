#if     defined(GNUSTEP_BASE_LIBRARY)
#import <Foundation/Foundation.h>
#import <GNUstepBase/GSMime.h>
#import "Testing.h"

static int
find(const char *buf, unsigned len, const char *str)
{
  int   l = strlen(str);
  int   max = len - l;
  int   i;

  for (i = 0; i < max; i++)
    {
      if (strncmp(buf + i, str, l) == 0)
        {
          return i;
        }
    }
  return -1;
}

int main(int argc,char **argv)
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSData *data = nil;
  NSString *string = nil;
  GSMimeDocument *doc = [[GSMimeDocument alloc] init];
  NSMutableDictionary *par = [[NSMutableDictionary alloc] init];

  [par setObject: @"my/type" forKey: @"type"];
  [doc setContent: @"Hello\r\n"];
  [doc setHeader: [[GSMimeHeader alloc] initWithName: @"content-type"
  					       value: @"text/plain"
					  parameters: par]];

  [doc setHeader:
    [[GSMimeHeader alloc] initWithName: @"content-transfer-encoding"
                                 value: @"binary"
                            parameters: nil]];
				
  data = [NSData dataWithContentsOfFile: @"mime8.dat"];
  PASS([[doc rawMimeData] isEqual: data], "Can make a simple document");

  string = @"ABCD credit card account − more information about Peach Pay.";
  [doc setHeader:
    [[GSMimeHeader alloc] initWithName: @"subject"
                                 value: string
                            parameters: nil]];
  data = [doc rawMimeData];
  PASS(data != nil, "Can use non-ascii character in subject");
  doc = [GSMimeParser documentFromData: data];
  PASS_EQUAL([[doc headerNamed: @"subject"] value], string,
   "Can restore non-ascii character in subject");

  data = [[GSMimeSerializer smtp7bitSerializer] encodeDocument: doc];
  PASS(data != nil, "Can serialize with non-ascii character in subject");
  doc = [GSMimeParser documentFromData: data];
  PASS_EQUAL([[doc headerNamed: @"subject"] value], string,
   "Can restore non-ascii character in subject form serialized document");

  [doc setHeader:
    [[GSMimeHeader alloc] initWithName: @"subject"
                                 value: @"€"
                            parameters: nil]];
  data = [doc rawMimeData];
  const char *bytes = "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain; type=\"my/type\"\r\n"
    "Subject: =?utf-8?B?4oKs?=\r\n\r\n";
  PASS(find((char*)[data bytes], (unsigned)[data length], "?B?4oKs?=") > 0,
    "encodes utf-8 euro in subject");

  [arp release]; arp = nil;
  return 0;
}
#else
int main(int argc,char **argv)
{
  return 0;
}
#endif
