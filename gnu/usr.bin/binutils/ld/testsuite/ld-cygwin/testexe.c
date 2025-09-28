int global_a = 2;

void 
exewrite (void)
{
  global_a = 1;
}

extern void dllwrite (void);

int _stdcall
testexe_main (void* p1, void *p2, char* p3, int p4)
{
  dllwrite ();
  /* We can't print or assert in a minimal app like this,
     so use the return status to indicate if global_a
     ended up with the correct expected value.  */
  return 1 - global_a;
}

/* We have to import something, anything at all, from 
   kernel32, in order to have the thread and process
   base thunk routines loaded when we start running!.  */
extern __attribute((dllimport)) void _stdcall Sleep (unsigned int duration);

int _stdcall
testexe_dummy (unsigned int foobar)
{
  Sleep (foobar);
}

