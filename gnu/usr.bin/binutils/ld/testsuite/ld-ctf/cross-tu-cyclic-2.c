struct B;
struct A
{
  long a;
  struct B *foo;
  struct C *bar;
};

struct C
{
  struct B *foo;
  int b;
};

static struct C *foo __attribute__((used));
static struct A *bar __attribute__((used));
