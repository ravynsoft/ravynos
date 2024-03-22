/* Macro using defined with a hard-coded identifier (no parentheses) */
#define is_foo_defined defined /*...*/ foo
#undef foo
#if is_foo_defined
failure
#else
success
#endif
#define foo
#if is_foo_defined
success
#else
failure
#endif

/* Macro using defined with a hard-coded identifier within parentheses */
#define is_foo_defined_parens defined /*...*/ ( /*...*/ foo /*...*/ ) //
#define foo
#if is_foo_defined_parens
success
#else
failure
#endif
#undef foo
#if is_foo_defined_parens
failure
#else
success
#endif

/* Macro using defined with an argument identifier (no parentheses) */
#define is_defined(arg) defined /*...*/ arg
#define foo bar
#undef bar
#if is_defined(foo)
failure
#else
success
#endif
#define bar bar
#if is_defined(foo)
success
#else
failure
#endif

/* Macro using defined with an argument identifier within parentheses */
#define is_defined_parens(arg) defined /*...*/ ( /*...*/ arg /*...*/ ) //
#define foo bar
#define bar bar
#if is_defined_parens(foo)
success
#else
failure
#endif
#undef bar
#if is_defined_parens(foo)
failure
#else
success
#endif

/* Multiple levels of macro resulting in defined */
#define X defined A && Y
#define Y defined B && Z
#define Z defined C
#define A
#define B
#define C
#if X
success
#else
failure
#endif
#undef A
#if X
failure
#else
success
#endif
#define A
#undef B
#if X
failure
#else
success
#endif
#define B
#undef C
#if X
failure
#else
success
#endif
