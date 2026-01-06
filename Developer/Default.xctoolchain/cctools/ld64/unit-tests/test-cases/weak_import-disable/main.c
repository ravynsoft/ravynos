
extern void foo()
#if MAKE_FOO_WEAK_IMPORT
__attribute__((weak_import))
#endif
;


int main()
{
	foo();

    return 0;
}

