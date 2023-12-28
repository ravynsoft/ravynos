extern int foo(void);
typedef int (*func_p) (void);
func_p foo_ptr = foo;
