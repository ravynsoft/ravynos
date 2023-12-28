typedef char foo_t;

/* Conflicting, and indexed.  */
extern foo_t var_1;
extern foo_t *var_666;

int other_func(foo_t *);

int ignore (void) { other_func (&var_1); other_func (var_666); }
