#import "Test.h"
#import "stdio.h"

#ifdef __unix__
// Declare these inline.  The libsupc++ version of cxxabi.h does not include
// __cxa_eh_globls, even though it's mandated by the ABI.
namespace __cxxabiv1
{
	struct __cxa_exception;
	struct __cxa_eh_globals
	{
		__cxa_exception *caughtExceptions;
		unsigned int uncaughtExceptions;
	};
	extern "C" __cxa_eh_globals *__cxa_get_globals();
}
extern "C" void check_uncaught_count(void)
{
	assert(__cxxabiv1::__cxa_get_globals()->uncaughtExceptions == 0);
}
#else
extern "C" void check_uncaught_count(void) {}
#endif

extern "C" void rethrow(id);


extern "C" void poke_objcxx(void)
{
    @try {
      printf("Raising MyException\n");
      Test *e = [Test new];
      @throw e;
    } @catch (Test *localException) {
      printf("Caught - re-raising\n");
      [localException retain];
      localException = [localException autorelease];;
      rethrow(localException);
    }
}

