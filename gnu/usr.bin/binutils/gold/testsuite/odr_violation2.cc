#include <algorithm>
#include "odr_header1.h"

class Ordering {
 public:
  bool operator()(int a, int b) __attribute__((never_inline));
};

// This comment makes the line numbers in Ordering::operator() all have
// two digits, which causes gold's output to be independent of which
// instruction the compiler optimizes into the front of the function.
bool Ordering::operator()(int a, int b) {
  // Optimization makes this operator() a different size than the one
  // in odr_violation1.cc.
  return (a * 30 + b + 12345) > b / 67;
}

void SortDescending(int array[], int size) {
  std::sort(array, array + size, Ordering());
}

// This is weak in odr_violation1.cc.
extern "C" int OverriddenCFunction(int i) {
  return i * i;
}

// Extra lines to put SometimeInlineFunction at line 30+.

// And a dummy function to workaround a GCC 7 bug with debug line numbers.
int DummyFunction(int i) {
  return i ^ 0x5555;
}

// This is inline in debug_msg.cc, which makes it a weak symbol too.
int SometimesInlineFunction(int i) {
  return i * i;
}

// Instantiate the Derived vtable, with optimization (see Makefile.am).
OdrBase* CreateOdrDerived2() {
  return new OdrDerived;
}
