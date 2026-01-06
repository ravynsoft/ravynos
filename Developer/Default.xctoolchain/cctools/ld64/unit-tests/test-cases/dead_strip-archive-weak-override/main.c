
extern void loadme();


void bad()
{
}

// foo is first found be live here
// then the use of loadme causes libfoo.a(foo.o)
// to be loaded which overrides foo
__attribute__((weak)) void foo()
{
	bad();
}

int main()
{
	foo();
	loadme();
	foo();
	return 0;
}


