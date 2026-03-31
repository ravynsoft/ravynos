// TEST_CRASHES
/*
TEST_RUN_OUTPUT
Associated object is 0x[0-9a-fA-F]+
objc\[\d+\]: objc_setAssociatedObject called on instance \(0x[0-9a-fA-F]+\) of class ForbiddenDuplicate which does not allow associated objects
objc\[\d+\]: HALTED
END
*/

#include "associationForbidden.h"

void test(void)
{
    ShouldSucceed([Normal alloc]);
    Class ForbiddenDuplicate = objc_duplicateClass([Forbidden class],
                                                   "ForbiddenDuplicate", 0);
    ShouldFail([ForbiddenDuplicate alloc]);
}
