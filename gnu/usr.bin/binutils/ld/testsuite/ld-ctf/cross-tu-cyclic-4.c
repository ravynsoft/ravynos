struct A { struct B *foo; };
struct B { struct B *next; };
static struct A *a __attribute__((__used__));
static struct B *conflicty __attribute__((__used__));
