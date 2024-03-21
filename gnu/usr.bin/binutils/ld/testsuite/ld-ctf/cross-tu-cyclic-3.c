struct A { struct B *foo; };
static struct A *a __attribute__((__used__));
static struct A *conflicty __attribute__((__used__));
