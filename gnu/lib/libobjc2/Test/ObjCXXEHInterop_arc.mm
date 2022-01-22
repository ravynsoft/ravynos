#import "Test.h"
#import "stdio.h"

extern "C" void rethrow(id);


extern "C" void poke_objcxx(void)
{
    @try {
      printf("Raising MyException\n");
      Test *e = [Test new];
      @throw e;
    } @catch (Test *localException) {
      printf("Caught - re-raising\n");
      rethrow(localException);
    }
}

