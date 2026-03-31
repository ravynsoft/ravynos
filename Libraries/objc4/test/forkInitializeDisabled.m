/*
TEST_CONFIG OS=macosx MEM=mrc ARCH=x86_64
(confused by ARC which loads Foundation which provokes more +initialize logs)
(also confused by i386 OS_object +load workaround)

TEST_ENV OBJC_PRINT_INITIALIZE_METHODS=YES

TEST_RUN_OUTPUT
objc\[\d+\]: INITIALIZE: disabling \+initialize fork safety enforcement because the app has a __DATA,__objc_fork_ok section
OK: forkInitializeDisabled\.m
END
*/

#include "test.h"

asm(".section __DATA, __objc_fork_ok\n.long 0\n");

int main()
{
    succeed(__FILE__);
}
