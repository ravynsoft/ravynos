#define REL(n) \
  static int data##n; \
  void *p##n = &data##n;

REL(1)
REL(2)
REL(3)
REL(4)
REL(5)
REL(6)
REL(7)
