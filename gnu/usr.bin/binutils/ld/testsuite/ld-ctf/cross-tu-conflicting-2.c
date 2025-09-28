struct B;
struct A
{
  int a;
  struct B *foo;
};

static struct A *foo __attribute__((used));
