// TEST_CRASHES
/* 
TEST_RUN_OUTPUT
objc\[\d+\]: Memory corruption in class list\.
objc\[\d+\]: HALTED
OR
old abi
OK: badSuperclass\.m
END
*/

#define CACHE_FLUSH
#include "badSuperclass.m"
