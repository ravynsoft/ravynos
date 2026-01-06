
__attribute__((visibility("hidden")))
void foo() { }

void bar() { }


int main()
{
  foo();
  bar();

  return 0;
}