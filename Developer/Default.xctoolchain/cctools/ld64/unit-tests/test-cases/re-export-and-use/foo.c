extern int bar();
extern int baz();

void* pbar = &bar;

#if USE_BAZ
void* pbaz = &baz;
#endif

int foo()
{
#if USE_BAZ
	baz();
#endif
	return bar();
}
