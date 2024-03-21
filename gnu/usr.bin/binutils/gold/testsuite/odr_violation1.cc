#include <algorithm>
#include "odr_header1.h"

class Ordering {
 public:
  bool operator()(int a, int b) {
    return a < b;
  }
};

void SortAscending(int array[], int size) {
  std::sort(array, array + size, Ordering());
}

extern "C" int OverriddenCFunction(int i) __attribute__ ((weak));
extern "C" int OverriddenCFunction(int i) {
  return i;
}

// Instantiate the Derived vtable, without optimization.
OdrBase* CreateOdrDerived1() {
  return new OdrDerived;
}
