#include "foo_impl.h"

int foo_dummy() {
  return foo_impl<int>();
}

int foo();

int main() {
  return foo();
}
