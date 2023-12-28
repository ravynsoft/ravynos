struct A;
struct B
{
  int foo;
  struct A *bar;
};

struct A
{
  long a;
  struct B *foo;
};

static struct A *foo __attribute__((used));
