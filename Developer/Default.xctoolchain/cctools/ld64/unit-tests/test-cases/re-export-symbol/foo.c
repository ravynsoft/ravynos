
#if USE_MY
  extern int mybar();
#else
  extern int bar();
#endif

int foo(void)
{
#if USE_MY
  return mybar() + 1;
#else
  return bar() + 1;
#endif
}

#if USE_MY
  void* p = &mybar;
#else
  void* p = &bar;
#endif
