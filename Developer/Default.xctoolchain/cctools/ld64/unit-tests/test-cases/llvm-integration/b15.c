extern void foo();
void bar() {
  foo();
}

void __attribute__((visibility("hidden"))) f2()
{}

