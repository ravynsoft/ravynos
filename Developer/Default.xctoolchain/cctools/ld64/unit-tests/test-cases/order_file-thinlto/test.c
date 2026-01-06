__attribute__((visibility("hidden")))
int foo() asm("_foo.llvm.ABCDEFG1");
int foo() { return 0; }

void api() {
  foo();
}
