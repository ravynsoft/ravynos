__attribute__((visibility("hidden")))
int foo() asm("_bar.llvm.1234567");
int foo() { return 0; }

void api3() {
  foo();
}
