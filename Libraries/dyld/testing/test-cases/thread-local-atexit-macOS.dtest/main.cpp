// BUILD_ONLY: MacOSX

// BUILD:  $CXX main.cpp -std=c++11 -o $BUILD_DIR/thread-local-atexit-macOS.exe

// RUN:  ./thread-local-atexit-macOS.exe

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "test_support.h"

// We create an A and a B.
// While destroying B we create a C
// Given that tlv_finalize has "destroy in reverse order of construction", we
// must then immediately destroy C before we destroy A to maintain that invariant

enum State {
    None,
    ConstructedA,
    ConstructedB,
    ConstructedC,
    DestroyingB,
    DestroyedA,
    DestroyedB,
    DestroyedC,
};

struct A {
    A();
    ~A();
};

struct B {
    B();
    ~B();
};

struct C {
    C();
    ~C();
};

State state;

A::A() {
    if ( state != None ) {
        FAIL("should be in the 'None' state");
    }
    state = ConstructedA;
}

B::B() {
    if ( state != ConstructedA ) {
        FAIL("should be in the 'ConstructedA' state");
    }
    state = ConstructedB;
}

C::C() {
    // We construct C during B's destructor
    if ( state != DestroyingB ) {
        FAIL("should be in the 'DestroyingB' state");
    }
    state = ConstructedC;
}

// We destroy B first
B::~B() {
    if ( state != ConstructedB ) {
        FAIL("should be in the 'ConstructedB' state");
    }
    state = DestroyingB;
    static thread_local C c;
    if ( state != ConstructedC ) {
        FAIL("should be in the 'ConstructedC' state");
    }
    state = DestroyedB;
}

// Then we destroy C
C::~C() {
    if ( state != DestroyedB ) {
        FAIL("should be in the 'DestroyedB' state");
    }
    state = DestroyedC;
}

// And finally destroy A
A::~A() {
    if ( state != DestroyedC ) {
        FAIL("should be in the 'DestroyedC' state");
    }
    state = DestroyedA;
    PASS("[Success");
}

static void work()
{
    thread_local A a = {};
    thread_local B b = {};
}

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    work();
    return 0;
}

