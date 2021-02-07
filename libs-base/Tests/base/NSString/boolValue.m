/*
copyright 2008 David Ayers <ayers@fsfe.org>

Tests that boolValue of certain strings return the correct value.
*/

#import "Testing.h"

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main(int argc, char **argv)
{
  NSString *constantStringY[]={
    @"y",@"Y",@"t",@"T",@"1",@"9",
    @"-y",@"-Y",@"-t",@"-T",@"-1",@"-9",
    @"Yes",@"YES",@"yes"
  };
  NSString *constantStringN[]={
    @"n",@"N",@"f",@"F",@"0",@"00",
    @"-n",@"-N",@"-f",@"-F",@"-0",@"-00",
    @"No",@"NO",@"no",
    @"0.0",@"0,0",
    @"0.1",@"0,1"
  };
  NSString *normalString;
  NSMutableString *mutableString;
  unsigned i;

  [NSAutoreleasePool new];
  for (i=0;i<(sizeof(constantStringY)/sizeof(constantStringY[0]));i++)
    {
      PASS([constantStringY[i] boolValue] == YES, "constant:%s == YES", [constantStringY[i] lossyCString]);
      PASS([constantStringN[i] boolValue] == NO,  "constant:%s == NO",  [constantStringN[i] lossyCString]);

      normalString = [NSString stringWithString:constantStringY[i]];
      PASS([normalString boolValue] == YES, "normal:%s == YES", [normalString lossyCString]);
      normalString = [NSString stringWithString:constantStringN[i]];
      PASS([normalString boolValue] == NO,  "normal:%s == NO",  [normalString lossyCString]);

      mutableString = (id)[NSMutableString stringWithString:constantStringY[i]];
      PASS([mutableString boolValue] == YES, "mutable:%s == YES", [mutableString lossyCString]);
      mutableString = (id)[NSMutableString stringWithString:constantStringN[i]];
      PASS([mutableString boolValue] == NO,  "mutable:%s == NO",  [mutableString lossyCString]);
    }

  return 0;
}
