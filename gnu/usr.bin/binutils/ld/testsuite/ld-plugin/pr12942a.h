extern void link_error ();

inline int test (void)
{
  int exp = -1;
  if ((exp < 2 ? 2U : (unsigned int) exp) != 2)
    link_error ();

  return 0;
}

typedef int (*test_t) (void);
