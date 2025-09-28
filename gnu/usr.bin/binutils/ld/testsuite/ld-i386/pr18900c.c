extern void foo (void);

typedef void (*func_p) (void);

func_p p1 = &foo;
