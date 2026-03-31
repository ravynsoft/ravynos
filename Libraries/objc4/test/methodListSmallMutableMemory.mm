/*
TEST_CFLAGS -std=c++11
TEST_CRASHES
TEST_RUN_OUTPUT
objc\[\d+\]: CLASS: class 'Foo' 0x[0-9a-fA-F]+ small method list 0x[0-9a-fA-F]+ is not in immutable memory
objc\[\d+\]: HALTED
END
*/

#define MUTABLE_METHOD_LIST 1

#include "methodListSmall.h"

int main() {
    Class fooClass = (__bridge Class)&FooClass;
    [fooClass new];
    fail("Should have crashed");
}
