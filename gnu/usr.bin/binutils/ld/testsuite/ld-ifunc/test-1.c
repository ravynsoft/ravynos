extern int library_func2 (void);
int (*fn) (void) = library_func2;
int main (void) { fn (); return 0; }
