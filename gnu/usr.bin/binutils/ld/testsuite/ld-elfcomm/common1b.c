static
#if     __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)
  __attribute__((__used__))
#endif  /* __GNUC__ */
  char dummy1 = 'X';
char foo1 [] = "Aligned at odd byte.";
char foo2 [4];
