#import "Test.h"

#import "minRep1.h"

#import "stdio.h"

@implementation MinRep1

- (void)poke
{
  printf("Poking from minRep1\n");
  poke_objcxx();
}

@end
