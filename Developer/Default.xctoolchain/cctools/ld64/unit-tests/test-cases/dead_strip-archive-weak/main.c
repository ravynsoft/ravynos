
extern void loadme();

void good()
{
}

void bad()
{
}

// foo is first found be dead stripping here
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
	return 0;
}


