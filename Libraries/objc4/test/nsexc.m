/* 
need exception-safe ARC for exception deallocation tests 
TEST_CFLAGS -fobjc-arc-exceptions -framework Foundation
*/

#define USE_FOUNDATION 1
#include "exc.m"
