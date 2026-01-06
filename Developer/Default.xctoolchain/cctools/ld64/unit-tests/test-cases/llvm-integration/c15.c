extern void foo();
int main() {
  foo();
  return 0;
}

void __attribute__((visibility("hidden"))) f2()
{
}
