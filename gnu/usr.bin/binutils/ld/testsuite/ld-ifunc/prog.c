extern int printf (const char *, ...);

extern int library_func1 (void);
extern int library_func2 (void);
extern int global;

int
main (void)
{
  int res = -1;

  res += library_func1 ();
  res += library_func2 ();

  switch (res)
    {
    case 0:
      if (global)
	printf ("ifunc working correctly\n");
      else
	{
	  printf ("wrong value returned by library_func2\n");
	  res = -1;
	}
      break;

    case 1:
      if (global)
	printf ("wrong value returned by library_func2\n");
      else
	{
	  printf ("ifunc working correctly\n");
	  res = 0;
	}
      break;

    case 4:
      printf ("non-ifunc testcase\n");
      break;

    default:
      printf ("ifunc function not evaluated at run-time, res = %x\n", res);
      break;
    }
  return res;
}
