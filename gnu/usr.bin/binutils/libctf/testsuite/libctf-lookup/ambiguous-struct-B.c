struct A;
struct B { struct A *a; };
struct A { struct B b; int foo; struct B b2; };

static struct A a __attribute__((__used__));
