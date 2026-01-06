

extern void bar1();
extern void bar2();
extern char  bar_array[];

void foo1() {}
void foo2() {}
char foo_array[3] = { 1,2,3 };



void* foostuff[] = { &foo1, &foo2, foo_array, &foo_array[3] };
void* barstuff[] = { &bar1, &bar2, bar_array, &bar_array[3] };
