__attribute__((visibility("hidden")))
int foo() asm("_foo.llvm.ABCDEFG2");
int foo() { return 0; }

void api2() {
  foo();
}
