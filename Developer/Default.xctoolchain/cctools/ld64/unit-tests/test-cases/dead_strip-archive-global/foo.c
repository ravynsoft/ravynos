
static int foo_count = 0;
static int bar_count = 0;
static int baz_count = 0;


void foo() { ++foo_count; }

void bar() { ++bar_count; }

void __attribute__((visibility("hidden"))) 
	baz() { ++baz_count; }
