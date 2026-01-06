static int bar() { return 0; }

template<typename T>
struct Foo {

  T foo(void) {
    return bar();
  }
};

__attribute__((constructor))
int other() {
  return Foo<int>().foo();
}
