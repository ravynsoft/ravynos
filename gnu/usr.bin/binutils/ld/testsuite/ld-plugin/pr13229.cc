void call_something (int);
inline void optimize_me_out (void)
{ 
  call_something(0);
}
__attribute__ ((visibility("hidden")))
void optimize_me_out2 (int param)
{ 
  if ((void *)optimize_me_out != (void *)call_something)
    call_something(0);
}
void test2 (void)
{ 
  optimize_me_out();
}
