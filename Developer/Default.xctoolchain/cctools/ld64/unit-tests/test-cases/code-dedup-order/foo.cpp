#include <stdio.h>

struct Foo {

  int sum;

  Foo(int a, int b) { sum = a + b; }

  template<typename T>
  T getSum() { return sum; }
};

template<typename T>
T foo(T a, T b) {
  return Foo(a, b).getSum<T>();
}

int bar(int a, int b);
int baz(int a, int b);

int main() {
  return foo(1, 1) + bar(1, 1) + baz(1, 1);
}
