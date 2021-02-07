#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  char		c[4];
  unsigned	i;
  NSString	*s;
  NSString	*e;

  c[1] = 0;
  
  c[0] = 0;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%00"], "character 0 is escaped");
  [s release];
  c[0] = 1;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%01"], "character 1 is escaped");
  [s release];
  c[0] = 2;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%02"], "character 2 is escaped");
  [s release];
  c[0] = 3;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%03"], "character 3 is escaped");
  [s release];
  c[0] = 4;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%04"], "character 4 is escaped");
  [s release];
  c[0] = 5;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%05"], "character 5 is escaped");
  [s release];
  c[0] = 6;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%06"], "character 6 is escaped");
  [s release];
  c[0] = 7;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%07"], "character 7 is escaped");
  [s release];
  c[0] = 8;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%08"], "character 8 is escaped");
  [s release];
  c[0] = 9;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%09"], "character 9 is escaped");
  [s release];
  c[0] = 10;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%0A"], "character 10 is escaped");
  [s release];
  c[0] = 11;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%0B"], "character 11 is escaped");
  [s release];
  c[0] = 12;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%0C"], "character 12 is escaped");
  [s release];
  c[0] = 13;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%0D"], "character 13 is escaped");
  [s release];
  c[0] = 14;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%0E"], "character 14 is escaped");
  [s release];
  c[0] = 15;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%0F"], "character 15 is escaped");
  [s release];
  c[0] = 16;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%10"], "character 16 is escaped");
  [s release];
  c[0] = 17;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%11"], "character 17 is escaped");
  [s release];
  c[0] = 18;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%12"], "character 18 is escaped");
  [s release];
  c[0] = 19;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%13"], "character 19 is escaped");
  [s release];
  c[0] = 20;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%14"], "character 20 is escaped");
  [s release];
  c[0] = 21;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%15"], "character 21 is escaped");
  [s release];
  c[0] = 22;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%16"], "character 22 is escaped");
  [s release];
  c[0] = 23;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%17"], "character 23 is escaped");
  [s release];
  c[0] = 24;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%18"], "character 24 is escaped");
  [s release];
  c[0] = 25;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%19"], "character 25 is escaped");
  [s release];
  c[0] = 26;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%1A"], "character 26 is escaped");
  [s release];
  c[0] = 27;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%1B"], "character 27 is escaped");
  [s release];
  c[0] = 28;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%1C"], "character 28 is escaped");
  [s release];
  c[0] = 29;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%1D"], "character 29 is escaped");
  [s release];
  c[0] = 30;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%1E"], "character 30 is escaped");
  [s release];
  c[0] = 31;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%1F"], "character 31 is escaped");
  [s release];
  c[0] = 32;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%20"], "character 32 is escaped");
  [s release];
  c[0] = 33;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 33 is not escaped");
  [s release];
  c[0] = 34;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%22"], "character 34 is escaped");
  [s release];
  c[0] = 35;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%23"], "character 35 is escaped");
  [s release];
  c[0] = 36;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 36 is not escaped");
  [s release];
  c[0] = 37;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%25"], "character 37 is escaped");
  [s release];
  c[0] = 38;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 38 is not escaped");
  [s release];
  c[0] = 39;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 39 is not escaped");
  [s release];
  c[0] = 40;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 40 is not escaped");
  [s release];
  c[0] = 41;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 41 is not escaped");
  [s release];
  c[0] = 42;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 42 is not escaped");
  [s release];
  c[0] = 43;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 43 is not escaped");
  [s release];
  c[0] = 44;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 44 is not escaped");
  [s release];
  c[0] = 45;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 45 is not escaped");
  [s release];
  c[0] = 46;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 46 is not escaped");
  [s release];
  c[0] = 47;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 47 is not escaped");
  [s release];
  c[0] = 48;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 48 is not escaped");
  [s release];
  c[0] = 49;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 49 is not escaped");
  [s release];
  c[0] = 50;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 50 is not escaped");
  [s release];
  c[0] = 51;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 51 is not escaped");
  [s release];
  c[0] = 52;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 52 is not escaped");
  [s release];
  c[0] = 53;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 53 is not escaped");
  [s release];
  c[0] = 54;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 54 is not escaped");
  [s release];
  c[0] = 55;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 55 is not escaped");
  [s release];
  c[0] = 56;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 56 is not escaped");
  [s release];
  c[0] = 57;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 57 is not escaped");
  [s release];
  c[0] = 58;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 58 is not escaped");
  [s release];
  c[0] = 59;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 59 is not escaped");
  [s release];
  c[0] = 60;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%3C"], "character 60 is escaped");
  [s release];
  c[0] = 61;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 61 is not escaped");
  [s release];
  c[0] = 62;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%3E"], "character 62 is escaped");
  [s release];
  c[0] = 63;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 63 is not escaped");
  [s release];
  c[0] = 64;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 64 is not escaped");
  [s release];
  c[0] = 65;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 65 is not escaped");
  [s release];
  c[0] = 66;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 66 is not escaped");
  [s release];
  c[0] = 67;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 67 is not escaped");
  [s release];
  c[0] = 68;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 68 is not escaped");
  [s release];
  c[0] = 69;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 69 is not escaped");
  [s release];
  c[0] = 70;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 70 is not escaped");
  [s release];
  c[0] = 71;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 71 is not escaped");
  [s release];
  c[0] = 72;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 72 is not escaped");
  [s release];
  c[0] = 73;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 73 is not escaped");
  [s release];
  c[0] = 74;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 74 is not escaped");
  [s release];
  c[0] = 75;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 75 is not escaped");
  [s release];
  c[0] = 76;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 76 is not escaped");
  [s release];
  c[0] = 77;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 77 is not escaped");
  [s release];
  c[0] = 78;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 78 is not escaped");
  [s release];
  c[0] = 79;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 79 is not escaped");
  [s release];
  c[0] = 80;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 80 is not escaped");
  [s release];
  c[0] = 81;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 81 is not escaped");
  [s release];
  c[0] = 82;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 82 is not escaped");
  [s release];
  c[0] = 83;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 83 is not escaped");
  [s release];
  c[0] = 84;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 84 is not escaped");
  [s release];
  c[0] = 85;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 85 is not escaped");
  [s release];
  c[0] = 86;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 86 is not escaped");
  [s release];
  c[0] = 87;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 87 is not escaped");
  [s release];
  c[0] = 88;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 88 is not escaped");
  [s release];
  c[0] = 89;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 89 is not escaped");
  [s release];
  c[0] = 90;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 90 is not escaped");
  [s release];
  c[0] = 91;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%5B"], "character 91 is escaped");
  [s release];
  c[0] = 92;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%5C"], "character 92 is escaped");
  [s release];
  c[0] = 93;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%5D"], "character 93 is escaped");
  [s release];
  c[0] = 94;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%5E"], "character 94 is escaped");
  [s release];
  c[0] = 95;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 95 is not escaped");
  [s release];
  c[0] = 96;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%60"], "character 96 is escaped");
  [s release];
  c[0] = 97;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 97 is not escaped");
  [s release];
  c[0] = 98;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 98 is not escaped");
  [s release];
  c[0] = 99;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 99 is not escaped");
  [s release];
  c[0] = 100;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 100 is not escaped");
  [s release];
  c[0] = 101;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 101 is not escaped");
  [s release];
  c[0] = 102;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 102 is not escaped");
  [s release];
  c[0] = 103;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 103 is not escaped");
  [s release];
  c[0] = 104;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 104 is not escaped");
  [s release];
  c[0] = 105;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 105 is not escaped");
  [s release];
  c[0] = 106;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 106 is not escaped");
  [s release];
  c[0] = 107;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 107 is not escaped");
  [s release];
  c[0] = 108;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 108 is not escaped");
  [s release];
  c[0] = 109;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 109 is not escaped");
  [s release];
  c[0] = 110;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 110 is not escaped");
  [s release];
  c[0] = 111;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 111 is not escaped");
  [s release];
  c[0] = 112;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 112 is not escaped");
  [s release];
  c[0] = 113;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 113 is not escaped");
  [s release];
  c[0] = 114;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 114 is not escaped");
  [s release];
  c[0] = 115;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 115 is not escaped");
  [s release];
  c[0] = 116;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 116 is not escaped");
  [s release];
  c[0] = 117;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 117 is not escaped");
  [s release];
  c[0] = 118;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 118 is not escaped");
  [s release];
  c[0] = 119;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 119 is not escaped");
  [s release];
  c[0] = 120;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 120 is not escaped");
  [s release];
  c[0] = 121;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 121 is not escaped");
  [s release];
  c[0] = 122;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 122 is not escaped");
  [s release];
  c[0] = 123;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%7B"], "character 123 is escaped");
  [s release];
  c[0] = 124;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%7C"], "character 124 is escaped");
  [s release];
  c[0] = 125;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%7D"], "character 125 is escaped");
  [s release];
  c[0] = 126;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: s], "character 126 is not escaped");
  [s release];
  c[0] = 127;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%7F"], "character 127 is escaped");
  [s release];
  c[0] = 128;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%80"], "character 128 is escaped");
  [s release];
  c[0] = 129;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%81"], "character 129 is escaped");
  [s release];
  c[0] = 130;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%82"], "character 130 is escaped");
  [s release];
  c[0] = 131;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%83"], "character 131 is escaped");
  [s release];
  c[0] = 132;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%84"], "character 132 is escaped");
  [s release];
  c[0] = 133;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%85"], "character 133 is escaped");
  [s release];
  c[0] = 134;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%86"], "character 134 is escaped");
  [s release];
  c[0] = 135;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%87"], "character 135 is escaped");
  [s release];
  c[0] = 136;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%88"], "character 136 is escaped");
  [s release];
  c[0] = 137;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%89"], "character 137 is escaped");
  [s release];
  c[0] = 138;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%8A"], "character 138 is escaped");
  [s release];
  c[0] = 139;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%8B"], "character 139 is escaped");
  [s release];
  c[0] = 140;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%8C"], "character 140 is escaped");
  [s release];
  c[0] = 141;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%8D"], "character 141 is escaped");
  [s release];
  c[0] = 142;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%8E"], "character 142 is escaped");
  [s release];
  c[0] = 143;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%8F"], "character 143 is escaped");
  [s release];
  c[0] = 144;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%90"], "character 144 is escaped");
  [s release];
  c[0] = 145;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%91"], "character 145 is escaped");
  [s release];
  c[0] = 146;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%92"], "character 146 is escaped");
  [s release];
  c[0] = 147;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%93"], "character 147 is escaped");
  [s release];
  c[0] = 148;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%94"], "character 148 is escaped");
  [s release];
  c[0] = 149;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%95"], "character 149 is escaped");
  [s release];
  c[0] = 150;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%96"], "character 150 is escaped");
  [s release];
  c[0] = 151;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%97"], "character 151 is escaped");
  [s release];
  c[0] = 152;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%98"], "character 152 is escaped");
  [s release];
  c[0] = 153;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%99"], "character 153 is escaped");
  [s release];
  c[0] = 154;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%9A"], "character 154 is escaped");
  [s release];
  c[0] = 155;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%9B"], "character 155 is escaped");
  [s release];
  c[0] = 156;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%9C"], "character 156 is escaped");
  [s release];
  c[0] = 157;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%9D"], "character 157 is escaped");
  [s release];
  c[0] = 158;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%9E"], "character 158 is escaped");
  [s release];
  c[0] = 159;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%9F"], "character 159 is escaped");
  [s release];
  c[0] = 160;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A0"], "character 160 is escaped");
  [s release];
  c[0] = 161;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A1"], "character 161 is escaped");
  [s release];
  c[0] = 162;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A2"], "character 162 is escaped");
  [s release];
  c[0] = 163;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A3"], "character 163 is escaped");
  [s release];
  c[0] = 164;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A4"], "character 164 is escaped");
  [s release];
  c[0] = 165;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A5"], "character 165 is escaped");
  [s release];
  c[0] = 166;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A6"], "character 166 is escaped");
  [s release];
  c[0] = 167;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A7"], "character 167 is escaped");
  [s release];
  c[0] = 168;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A8"], "character 168 is escaped");
  [s release];
  c[0] = 169;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%A9"], "character 169 is escaped");
  [s release];
  c[0] = 170;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%AA"], "character 170 is escaped");
  [s release];
  c[0] = 171;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%AB"], "character 171 is escaped");
  [s release];
  c[0] = 172;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%AC"], "character 172 is escaped");
  [s release];
  c[0] = 173;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%AD"], "character 173 is escaped");
  [s release];
  c[0] = 174;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%AE"], "character 174 is escaped");
  [s release];
  c[0] = 175;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%AF"], "character 175 is escaped");
  [s release];
  c[0] = 176;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B0"], "character 176 is escaped");
  [s release];
  c[0] = 177;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B1"], "character 177 is escaped");
  [s release];
  c[0] = 178;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B2"], "character 178 is escaped");
  [s release];
  c[0] = 179;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B3"], "character 179 is escaped");
  [s release];
  c[0] = 180;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B4"], "character 180 is escaped");
  [s release];
  c[0] = 181;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B5"], "character 181 is escaped");
  [s release];
  c[0] = 182;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B6"], "character 182 is escaped");
  [s release];
  c[0] = 183;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B7"], "character 183 is escaped");
  [s release];
  c[0] = 184;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B8"], "character 184 is escaped");
  [s release];
  c[0] = 185;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%B9"], "character 185 is escaped");
  [s release];
  c[0] = 186;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%BA"], "character 186 is escaped");
  [s release];
  c[0] = 187;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%BB"], "character 187 is escaped");
  [s release];
  c[0] = 188;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%BC"], "character 188 is escaped");
  [s release];
  c[0] = 189;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%BD"], "character 189 is escaped");
  [s release];
  c[0] = 190;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%BE"], "character 190 is escaped");
  [s release];
  c[0] = 191;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%BF"], "character 191 is escaped");
  [s release];
  c[0] = 192;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C0"], "character 192 is escaped");
  [s release];
  c[0] = 193;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C1"], "character 193 is escaped");
  [s release];
  c[0] = 194;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C2"], "character 194 is escaped");
  [s release];
  c[0] = 195;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C3"], "character 195 is escaped");
  [s release];
  c[0] = 196;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C4"], "character 196 is escaped");
  [s release];
  c[0] = 197;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C5"], "character 197 is escaped");
  [s release];
  c[0] = 198;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C6"], "character 198 is escaped");
  [s release];
  c[0] = 199;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C7"], "character 199 is escaped");
  [s release];
  c[0] = 200;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C8"], "character 200 is escaped");
  [s release];
  c[0] = 201;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%C9"], "character 201 is escaped");
  [s release];
  c[0] = 202;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%CA"], "character 202 is escaped");
  [s release];
  c[0] = 203;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%CB"], "character 203 is escaped");
  [s release];
  c[0] = 204;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%CC"], "character 204 is escaped");
  [s release];
  c[0] = 205;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%CD"], "character 205 is escaped");
  [s release];
  c[0] = 206;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%CE"], "character 206 is escaped");
  [s release];
  c[0] = 207;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%CF"], "character 207 is escaped");
  [s release];
  c[0] = 208;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D0"], "character 208 is escaped");
  [s release];
  c[0] = 209;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D1"], "character 209 is escaped");
  [s release];
  c[0] = 210;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D2"], "character 210 is escaped");
  [s release];
  c[0] = 211;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D3"], "character 211 is escaped");
  [s release];
  c[0] = 212;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D4"], "character 212 is escaped");
  [s release];
  c[0] = 213;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D5"], "character 213 is escaped");
  [s release];
  c[0] = 214;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D6"], "character 214 is escaped");
  [s release];
  c[0] = 215;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D7"], "character 215 is escaped");
  [s release];
  c[0] = 216;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D8"], "character 216 is escaped");
  [s release];
  c[0] = 217;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%D9"], "character 217 is escaped");
  [s release];
  c[0] = 218;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%DA"], "character 218 is escaped");
  [s release];
  c[0] = 219;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%DB"], "character 219 is escaped");
  [s release];
  c[0] = 220;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%DC"], "character 220 is escaped");
  [s release];
  c[0] = 221;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%DD"], "character 221 is escaped");
  [s release];
  c[0] = 222;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%DE"], "character 222 is escaped");
  [s release];
  c[0] = 223;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%DF"], "character 223 is escaped");
  [s release];
  c[0] = 224;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E0"], "character 224 is escaped");
  [s release];
  c[0] = 225;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E1"], "character 225 is escaped");
  [s release];
  c[0] = 226;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E2"], "character 226 is escaped");
  [s release];
  c[0] = 227;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E3"], "character 227 is escaped");
  [s release];
  c[0] = 228;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E4"], "character 228 is escaped");
  [s release];
  c[0] = 229;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E5"], "character 229 is escaped");
  [s release];
  c[0] = 230;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E6"], "character 230 is escaped");
  [s release];
  c[0] = 231;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E7"], "character 231 is escaped");
  [s release];
  c[0] = 232;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E8"], "character 232 is escaped");
  [s release];
  c[0] = 233;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%E9"], "character 233 is escaped");
  [s release];
  c[0] = 234;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%EA"], "character 234 is escaped");
  [s release];
  c[0] = 235;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%EB"], "character 235 is escaped");
  [s release];
  c[0] = 236;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%EC"], "character 236 is escaped");
  [s release];
  c[0] = 237;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%ED"], "character 237 is escaped");
  [s release];
  c[0] = 238;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%EE"], "character 238 is escaped");
  [s release];
  c[0] = 239;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%EF"], "character 239 is escaped");
  [s release];
  c[0] = 240;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F0"], "character 240 is escaped");
  [s release];
  c[0] = 241;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F1"], "character 241 is escaped");
  [s release];
  c[0] = 242;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F2"], "character 242 is escaped");
  [s release];
  c[0] = 243;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F3"], "character 243 is escaped");
  [s release];
  c[0] = 244;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F4"], "character 244 is escaped");
  [s release];
  c[0] = 245;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F5"], "character 245 is escaped");
  [s release];
  c[0] = 246;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F6"], "character 246 is escaped");
  [s release];
  c[0] = 247;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F7"], "character 247 is escaped");
  [s release];
  c[0] = 248;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F8"], "character 248 is escaped");
  [s release];
  c[0] = 249;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%F9"], "character 249 is escaped");
  [s release];
  c[0] = 250;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%FA"], "character 250 is escaped");
  [s release];
  c[0] = 251;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%FB"], "character 251 is escaped");
  [s release];
  c[0] = 252;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%FC"], "character 252 is escaped");
  [s release];
  c[0] = 253;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%FD"], "character 253 is escaped");
  [s release];
  c[0] = 254;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%FE"], "character 254 is escaped");
  [s release];
  c[0] = 255;
  s = [[NSString alloc]
    initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];
  e = [s stringByAddingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
  PASS([e isEqual: @"%FF"], "character 255 is escaped");
  [s release];

  for (i = 0; i < 256; i++)
    {
      NSString	*escaped;
      NSString	*unescaped;
      NSString	*reference;

      c[0] = i;
      reference = [[NSString alloc] initWithBytes: c length: 1 encoding: NSISOLatin1StringEncoding];

      sprintf(c, "%%%02x", i);
      escaped = [[NSString alloc] initWithBytes: c length: 3 encoding: NSASCIIStringEncoding];
      unescaped = [escaped stringByReplacingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
      [escaped release];
      PASS_EQUAL(unescaped, reference, "unescapes '%s'", c);

      sprintf(c, "%%%02X", i);
      escaped = [[NSString alloc] initWithBytes: c length: 3 encoding: NSASCIIStringEncoding];
      unescaped = [escaped stringByReplacingPercentEscapesUsingEncoding: NSISOLatin1StringEncoding];
      [escaped release];
      PASS_EQUAL(unescaped, reference, "unescapes '%s'", c);

      [reference release];
    }

  [arp release]; arp = nil;
  return 0;
}
