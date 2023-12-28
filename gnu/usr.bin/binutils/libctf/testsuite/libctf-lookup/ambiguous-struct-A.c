struct A;
struct B { struct A *a; };
struct A { struct B b; long foo; long bar; struct B b2; };

typedef struct A a_array[50];
a_array *foo __attribute__((__used__));

static struct A a __attribute ((__used__));
