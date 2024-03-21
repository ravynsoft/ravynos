struct cycle_1 {
  struct A *a;
  struct B *b;
  struct cycle_1 *next;
};

static struct cycle_1 *cycle_1 __attribute__((used));
