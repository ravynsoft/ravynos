void cxx_throw();

__attribute((visibility("hidden")))
int eh_trampoline()
{
	struct X { ~X() {} } x;
	cxx_throw();
	return 0;
}
