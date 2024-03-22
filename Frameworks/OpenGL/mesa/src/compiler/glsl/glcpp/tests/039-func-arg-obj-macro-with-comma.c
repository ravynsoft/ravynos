/* This works. */
#define foo(a) (a)
#define bar two,words
foo(bar)

/* So does this. */
#define foo2(a,b) (a separate b)
#define foo2_wrap(a) foo2(a)
foo2_wrap(bar)

/* But this generates an error. */
#define foo_wrap(a) foo(a)
foo_wrap(bar)

/* Adding parentheses to foo_wrap fixes it. */
#define foo_wrap_parens(a) foo((a))
foo_wrap_parens(bar)

/* As does adding parentheses to bar */
#define bar_parens (two,words)
foo_wrap(bar_parens)
foo_wrap_parens(bar_parens)


