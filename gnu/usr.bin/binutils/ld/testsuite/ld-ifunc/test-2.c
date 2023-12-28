extern int library_func2 (void);
int foo (void) { library_func2 (); return 0; }
__asm__(".type library_func2, %gnu_indirect_function");
