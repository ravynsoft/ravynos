extern __attribute__ ((visibility("hidden"))) int fooblah;

static void
do_nothing (int param)
{ 
  if (param)
   fooblah = 1;
}

void
bar ()
{ 
  do_nothing (0);
}
