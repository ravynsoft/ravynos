#import "Test.h"

#import "stdio.h"

void poke_objcxx(void);

void rethrow(id x)
{
	@throw x;
}

int main(void)
{
  @try {
    printf("Poking from minRepM\n");
    poke_objcxx();
    printf("Poked from minRepM\n");
  } @catch (Test *localException) {
    printf("In NS_HANDLER block, %p\n", localException);
  }
}

