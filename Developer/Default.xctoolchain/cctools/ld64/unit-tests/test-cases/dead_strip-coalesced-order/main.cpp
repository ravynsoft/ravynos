template<typename T>
struct Foo {

  // Use a large alignment and place foo in a separate
  // section to make sure it's modulo alignment is 0, so that
  // it's consistently preferred over the implementation from foo.cpp.
  __attribute__((section("__TEXT,__foo")))
  T foo(void) {
    asm volatile(".p2align 6");
    return 0;
  }
};

int main() {
  return Foo<int>().foo();
}
